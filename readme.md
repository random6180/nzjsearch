搜索引擎

本项目直接借助浏览器客户端在url中输入用户查询词，并把搜索结果返回，如下所示：
![搜索结果](https://github.com/random6180/nzjsearch/blob/master/%E6%90%9C%E7%B4%A2%E7%BB%93%E6%9E%9C.jpg)

第一部分：搜索

一.创建英文、中文词典， CreateEndict， CreateCnDict文件

1.int CreateEnDict::loadFile(const string &dir, const string &suffx);
调用类GetFilenameFromDir读取文件
regex reg(“[A-Za-z]+”);
auto word_begin = sregex_iterator(line.begin(), line.end(), reg);
auto word_end = sregex_iterator();
word = (*it).str()就得到了一个单词，再大写转小写
放入词典map<string, int> _dictionary, string代表单词, int 代表数量

2.int CreateEnDict::dumpFile(const string &path);
把_dictionary导入文件中，形成词典

3.int CreateCnDict::loadFile(const string &dir, const string &suffx);
创建中文词典
regex reg(“[\u4e00-\u9fa5]”)
相比英文词典步骤，多了一条，就是先读取一行，匹配中文句子，对中文句子进行分词，放入词典。


二.导入词典，生成字典，Corrector

1.int Corrector::loadDictionary(const string &dictPath);
从磁盘加载词典进入内存map<string, int> _dictionary

2.int Corrector::createIndex();
调用Mysplit(string, vector<string>)函数把单词分成每个字，
数据结构map<string, set<int>> _indexMap中string为单个字， set<int>为字所在词的索引

3.shared_ptr<vector<string>> Corrector::findWord(const string &word, int wordNum);
输入一个单词，对单词调用MySplit切分成字
找出所有字的交集词，对这些词与word求最小编辑距离，放入优先级队列priority_queue，
去除前wordNum个作为候选词。


三.缓存管理，Cache，CacheManager

1.每个Cache内部存放unordered_map<string,  shared_ptr<vector<string>>> _wordMap.

2.int CacheManager::initCache(const string &filename, int size)
读取文件，放入vector<Cache> _cacheVec中，里面存放每个单词的纠错词。

3.int CacheManager::updateCache()
将所有的_cacheVec[i]，除i=0，放入_cacheVec[0]中，然后用_cacheVec[0]更新其他_cacheVec[i]。



四.解析xml，RssParse

1.int RssParse::loadFile(const string &dir, const string &suffix);
依次导入每个xml文件，调用parseRss进行解析。

2.int RssParse::parseRss(const string &filename)；
定义数据结构RssItem，里面有title, link, description, content字符串，
vector<shared_ptr<RssItem>> _rssVec为指向RssItem的指针向量，
此函数解析xml文件，生成_rssVec结构

3.int RssParse::dumpFile(const string &filename, const string &indexFile)；
将_rssVec结构放入网页文件PAGE_LIB，同时生成对应的偏移文件OFFSET_LIB。


五.生成倒排索引，InvertIndex

1.int InvertIndex::loadFile(const string &pageFile, const string &offsetFile);
根据offsetFile偏移文件，读取文档1放入，string str，对str进行分词，去除停用词。
定义结构Docinfo，里面有frequency, length，分别代表单词频率，文档长度。
结构map<string, map<int, Docinfo>> _invertIndex，键为单词，值为单词所在文档id，以及该单词在该文档中的Docinfo属性。

2.int InvertIndex::dumpFile(const string &indexFile)；
根据_invertIndex，求出tf = 单词频率/文档总长度，越大越好，
idf = 文档总数/该单词出现在的文档数，越大越好，计算weight = tf * idf作为权重。
放入倒排索引文件indexFile。


六.查询句子，返回对应文档内容，WebQuery

1.WebQuery::WebQuery(const string &pageFile, const string &offsetFile, const string &indexFile);
将偏移文件和倒排索引文件导入内存，其中偏移文件对应的结构为vector<pair<int, int>> _offset，
倒排索引文件对应的结构为map<string, map<int, double>> _invertIndex。

2.int WebQuery::queryWord(vector<string> pWords, string &res)；
对pWords中的每个单词，根据_invertIndex，查询对应的文档及其权重，并对权重进行累加，
放入map<int, double> resMap，再由resMap生成vector<pair<double, int>> resVec，并案权重排序。
选取前20个文档，根据_offset，查询文档id的起始位置即长度，生成string res作为html内容返回。


第二部分：http服务器

一.创建http服务器，tcpserver

1.Tcpserver::Tcpserver(int _port, int pthnum, int epnum);
定义端口号，线程数， epoll事件数。



2.void Tcpserver::Serve()；
开启服务。首先调用函数open_listenfd，创建监听套接字，
接着创建线程池，ThreadPool pool(pthread_num, doit)，并运行线程池，
再创建epoll事件，Epoll ep(event_num)，
把监听文件描述符添加进来，ep.epollAdd(listen_fd, EPOLLIN | EPOLLET)，
然后启动循环，在循环中调用ep.epollWait()，若有事件，则判断事件里的文件描述符是什么，
若是listen_fd，则调用accept函数，接收连接描述符connfd，并放入事件，
若是其他文件描述符connfd，说明可以read了，则把该文件描述符加入线程池。


二.线程池设计，threadpool

1.ThreadPool::ThreadPool(int threadnum, function<void(int)> handle);
构造函数，参数为线程数及线程对应的回调函数。
为线程变量pthread_t分配空间。

2.void ThreadPool::addTask(int fd)；
用线程互斥所，往线程池的向量vector<int> taskqueue中添加文件描述符。

3.void ThreadPool::start()；
线程池启动函数，调用pthread_create()函数，其中的回调函数是pthfunc，回调函数参数是
线程池this指针，即线程函数可以访问线程池。

4.void *ThreadPool::pthfunc(void *arg)；
上锁，访问taskqueue向量，若为空，则等待条件变量，否则取出文件描述符，
执行线程池的回调函数handler。


三.epoll事件设计，epoll

1.Epoll::Epoll(int maxevents)；
设置结构体struct epoll_event[]数组大小为maxevents，并分配空间，
创建epoll_fd描述符，epoll_create(1024)。

2.int Epoll::epollAdd(int fd, int event)；
调用epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev)往epoll_fd中添加事件。

3.void Epoll::epollWait()；
调用nfds = epoll_wait(epoll_fd, events, max_events, -1)等待事件到达，
并放入struct epoll_event *events结构体中。


四.处理函数，tiny

1.void doit(int fd)；
调用rio_readlineb(&rio, buf, MAXLINE)，读取一行，解析请求行得method, uri, version，
调用parse_uri解析uri并解码，得到文件名和参数，通过判断uri中有无bin判断是否是静态文件，
若无bin，返回固定html，若有bin，则获取搜索词，调用搜索函数。
2.void serve_dynamic(int fd, char *filename, char *cgiargs)；
调用WebQuery web(page, offset, invert)，初始化网页查询类，
对cgiargs进行分词，得到vector<string> words，
调用web.queryWord(words, res)，得到文档内容res，并发送给客户端。


五.改进，缓存设计

线程池threadpool存放缓存类vector<Cache> _cacheVec，
其中Cache类封装了两个map，
一个是unordered_map<string, shared_ptr<vector<string>>> _wordMap，用于单词纠错，
另一个是unordered_map<string, string> _sentenceMap，用于缓存已查询词汇。
_cacheVec向量长度为线程数+1，其中_cacheVec[0]为线程共享区，用于同步和更新缓存。
