#ifndef __TINY_H__
#define __TINY_H__

#include "socketio.h"
#include "Cache.h"

#define MAXLINE 4096
#define MAXBUF 8192
using namespace nzj;

void doit(int fd, Cache &cache);

void read_requesthdrs(rio_t *rp);

int parse_uri(char *uri, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, int filesize);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs, Cache &cache);

void clienterror(int fd, const char *cause, const char *errnum, const char *shortmsg, const char *longmsg);


#endif
