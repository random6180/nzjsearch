#include"../include/WebQuery.h"
#include"../include/Mylog.h"
#include "../include/Corrector.h"
#include "../include/GlobalVariable.h"
#include<json/json.h>
#include<fstream>
#include<sstream>
#include<algorithm>
#include <iostream>
using std::ifstream;
using std::istringstream;
using std::cout;
using std::endl;

namespace nzj
{
WebQuery::WebQuery(const string &pageFile, const string &offsetFile, const string &indexFile)
: _pageFileName(pageFile)
{
	loadOffsetFile(offsetFile);
	loadIndexFile(indexFile);
}
int WebQuery::loadOffsetFile(const string &offsetFile)
{
	ifstream ifs(offsetFile);
	if(!ifs.is_open())
	{
		Mylog::getInstance()->_root.debug("WebQuery: load offset file error!");
		return -1;
	}
	Mylog::getInstance()->_root.debug("WebQuery: load offset file ...");
	int docid, begin, length;
	while(ifs)
	{
		ifs>>docid>>begin>>length;
		_offset.push_back(std::make_pair(begin, length));
	}
	ifs.close();
	return 0;
}
int WebQuery::loadIndexFile(const string &indexFile)
{
	ifstream ifs(indexFile);
	if(!ifs.is_open())
	{
		Mylog::getInstance()->_root.debug("WebQuery: load index file error!");
		return -1;
	}
	Mylog::getInstance()->_root.debug("WebQuery: load index file ...");
	string line;
	string word;
	int docid;
	double weight;
	while(std::getline(ifs, line))
	{
		istringstream iss(line);
		iss>>word;
		map<int, double> tmpMap;
		while(iss)
		{
			iss>>docid>>weight;
			tmpMap.insert(std::make_pair(docid, weight));
		}
		_invertIndex.insert(std::make_pair(word, tmpMap));
	}
	ifs.close();
	return 0;
}
int WebQuery::queryWord(vector<string> pWords, string &res, Cache &cache)
{
	map<int, double> resMap;//用于存储包含关键字的docid和其权重加和
    vector<string> subWords;
    for(size_t i=0; i<pWords.size(); ++i) {
        auto rets = _invertIndex.find(pWords[i]);
        if(rets == _invertIndex.end()) {
            cout << "倒排索引中不存在:" << endl;
            shared_ptr<vector<string>> tmp(new vector<string>);
            tmp = cache.findWord(pWords[i]);
            if (tmp->size() == 0) {
                cout << "纠错词不在缓存中:" << endl;
                Corrector corr;
                corr.loadDictionary(DICT_CN);
                corr.createIndex();
                tmp = corr.findWord(pWords[i],2);
                if (tmp->size() == 0) {
                    subWords.push_back(pWords[i]);
                }
                else {
                    cache.addWord(pWords[i], tmp);
                    subWords.push_back((*tmp)[0]);
                }
            }
            else {
                subWords.push_back((*tmp)[0]);
            }
        }
        else {
            subWords.push_back(pWords[i]);
        }
    }


	for(size_t idx=0; idx<subWords.size(); ++idx)
	{
		auto ret=_invertIndex.find(subWords[idx]);
		if(ret!=_invertIndex.end())//如果在倒排索引中找到关键字，则遍历其map
		{
			map<int, double>::iterator it;
			for(it=ret->second.begin(); it!=ret->second.end(); ++it)
			{
				auto tmpRet=resMap.insert(*it);
				if(!tmpRet.second)//如果插入失败，则把权重加到对应docid上
				{
					tmpRet.first->second = tmpRet.first->second + it->second;
				}
			}
		}
	}
//	Json::Value root;
//	Json::Value arrJson;
    string title, link, summary;

	if(resMap.size()==0)//如果集合是空的，说明没有符合关键词的文章
	{
//		Json::Value elemJson;
		title = "404 Not Found";
		summary = "Sorry, we can't find what you want.";
		link="https://www.baidu.com";
		
        res = res + "<a href=\"" +  link + "\">" + title + "</a><p>" + summary + "</p>\r\n\r\n";

		return -1;//返回-1表示没有找到
	}
	vector<std::pair<double, int>> resVec;
	for(auto it=resMap.begin(); it!=resMap.end(); ++it)
	{
		resVec.push_back(std::make_pair(it->second, it->first));
	}
	std::sort(resVec.begin(), resVec.end(), std::greater<std::pair<double, int>>());//对文章按权重由大到小排序
	//for(size_t idx=0; idx<resVec.size(); ++idx)//for debug
	//{
	//	std::cout<<resVec[idx].first<<" "<<resVec[idx].second<<std::endl;
	//}
	int docid, begin, length;
//    string title, link, summary;
	ifstream ifs(_pageFileName);
	if(!ifs.is_open())
	{
		Mylog::getInstance()->_root.debug("WebQuery: open web page file error!");
		return -1;
	}
    int num = 0;
	for(size_t idx=0; idx<resVec.size() && num < 20; ++idx, ++num)
	{
    //    res = res + "<a href=\"";
		docid=resVec[idx].second;//获取文章id, 起始位置和长度
		begin=_offset[docid-1].first;
		length=_offset[docid-1].second;
		string docStr(length, '\0');
		ifs.seekg(begin);
		ifs.read(&docStr[0], length);//读取文章

//		Json::Value elemJson;
		int titlePos1=docStr.find("<title>")+7;
		int titlePos2=docStr.find("</title>");
		title = docStr.substr(titlePos1, titlePos2-titlePos1);
		int summaryPos1=docStr.find("<description>")+13;
		int summaryPos2=docStr.find("</description>");
		summary = docStr.substr(summaryPos1, summaryPos2-summaryPos1);
		int urlPos1=docStr.find("<link>")+6;
		int urlPos2=docStr.find("</link>");
		link = docStr.substr(urlPos1, urlPos2-urlPos1);
		
        res = res + "<a href=\"" +  link + "\">" + title + "</a><p>" + summary + "</p>\r\n\r\n";


	}
	ifs.close();
//	root["files"]=arrJson;
//	Json::StyledWriter sWriter;
//	res=sWriter.write(root);
	return 0;
}
}
