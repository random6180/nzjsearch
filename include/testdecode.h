#ifndef __TESTDECODE_H__
#define __TESTDECODE_H__

#include <stdio.h>
#include <string.h>

#define BUFSIZE 1024

int hex2dec(char c);

char dec2hex(short int c);

void urlencode(char url[]);

void urldecode(char url[]);


#endif
