#include "../include/WebQuery.h"
#include "../include/GlobalVariable.h"
#include "../deps/cppjieba/Jieba.hpp"
#include "../include/tiny.h"
#include "../include/socketio.h"
#include "../include/testdecode.h"
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace nzj;
using cppjieba::Jieba;


string offset = "/home/nzj/wkspace2/search/lib/indexfile.lib";
string page = "/home/nzj/wkspace2/search/lib/pagefile.lib";
string invert = "/home/nzj/wkspace2/search/lib/invertfile.lib";


void doit(int fd, Cache &cache) {
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	rio_readinitb(&rio, fd);
	rio_readlineb(&rio, buf, MAXLINE);
	printf("Request headers: \n");
	printf("%s", buf);

	sscanf(buf, "%s %s %s", method, uri, version);

	if (strcasecmp(method, "GET")) {
		clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
		return;
	}

	read_requesthdrs(&rio);

	is_static = parse_uri(uri, filename, cgiargs);	

	if (stat(filename, &sbuf) < 0) {
		clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
		return;
	}

	if (is_static) {
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else {
//		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
//			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
//			return;
//		}
		serve_dynamic(fd, filename, cgiargs, cache);
	}
    close(fd);
}



void clienterror(int fd, const char *cause, const char *errnum, const char *shortmsg, const char *longmsg) {
	char buf[MAXLINE], body[MAXBUF];

	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	rio_writen(fd, buf, strlen(buf));
	rio_writen(fd, body, strlen(body));
}


void read_requesthdrs(rio_t *rp) {
	char buf[MAXLINE];

	rio_readlineb(rp, buf, MAXLINE);
	while (strcmp(buf, "\r\n")) {
		printf("%s", buf);
		rio_readlineb(rp, buf, MAXLINE);
//		printf("%s", buf);
	}
	printf("%s", buf);
	return;
}


int parse_uri(char *uri, char *filename, char *cgiargs) {
	char *ptr;
	
    urldecode(uri);

	if (!strstr(uri, "bin")) {
		strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		if (uri[strlen(uri) - 1] == '/')
			strcat(filename, "home.html");
		return 1;
	}

	else {
		ptr = index(uri, '?');
		if (ptr) {
			strcpy(cgiargs, ptr + 1);
			*ptr = '\0';
		}	
		else
			strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}

}


void serve_static(int fd, char *filename, int filesize) {
	int srcfd;
	char srcp[MAXBUF], filetype[MAXLINE], buf[MAXBUF];
	
	get_filetype(filename, filetype);

    sprintf(srcp, "<html>\r\n<body><p>我喜欢!</p>\r\n<p>我喜欢!</p></body></html>");

    int changdu = strlen(srcp);

	
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sConnection: close\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, changdu);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	rio_writen(fd, buf, strlen(buf));

	printf("Response headers:\n");
	printf("%s", buf);

	srcfd = open(filename, O_RDONLY, 0);
//	srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);

    rio_writen(fd, srcp, strlen(srcp));
//	munmap(srcp, filesize);
}


void get_filetype(char *filename, char *filetype) {
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".png"))
                strcpy(filetype, "image/png");
	else if (strstr(filename, ".jpg"))
                strcpy(filetype, "image/jpeg");
	else
		strcpy(filetype, "text/plain");
}


//void serve_dynamic(int fd, char *filename, char *cgiargs) {
//	char buf[MAXLINE], *emptylist[] = {0, NULL };
//	char *environ[] = {0, NULL};
//	sprintf(buf, "HTTP/1.0 200 OK\r\n");
//	rio_writen(fd, buf, strlen(buf));
//	sprintf(buf, "Server: Tiny Web Server\r\n");
//	rio_writen(fd, buf, strlen(buf));

//	if (fork() == 0) {
//		setenv("QUERY_STRING", cgiargs, 1);
//		dup2(fd, STDOUT_FILENO);
//		execve(filename, emptylist, environ);
//	}
//	wait(NULL);
//}


void serve_dynamic(int fd, char *filename, char *cgiargs, Cache &cache) {
    char filetype[MAXLINE], buf[MAXBUF];

    sprintf(filetype, "text/html");  
    char *srcp = (char *)malloc(sizeof(char) * 1000000);
    if (srcp == NULL) {
        cout << "serve_dynamic: malloc!" << endl;
        return;
    }
    string sentence(cgiargs);
    string res;
    res = cache.findSentence(sentence);

    if (res.size() == 0) {  //缓存中不存在
        cout << "缓存中不存在: " << sentence << endl;
        cout << "开始导入词典: " << endl;
        WebQuery web(page, offset, invert);
        //    string sentence(cgiargs);
        vector<string> words;
        Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);
        cout << "开始分词: " << endl;
        jieba.Cut(sentence, words);
		
        cout << "开始查询: " << endl;
        web.queryWord(words, res, cache);
        cache.addSentence(sentence, res);
    }
    else {
        cout << "缓存中存在: " << sentence << endl;
    }
    cout << "长度为: " << res.length() << endl;
    cout << "开始放入数组srcp:" << endl;
    sprintf(srcp, "<html>\r\n<head><meta charset=\"utf-8\">\r\n<title>搜索结果</title>\r\n</head>\r\n<body>%s</body></html>", 
            res.c_str());
    

    int changdu = strlen(srcp);

    cout << "开始发送数据: " << endl;

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, changdu);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    rio_writen(fd, buf, strlen(buf));

    printf("Response headers:\n");
    printf("%s", buf);

    rio_writen(fd, srcp, strlen(srcp));
    free(srcp);
}

















