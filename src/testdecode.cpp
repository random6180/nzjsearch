#include "../include/testdecode.h"


int hex2dec(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    else {
        return -1;
    }
}


char dec2hex(short int c) {
    if (c >= 0 && c <= 9) {
        return c + '0';
    }
    else if (c >= 10 && c <= 15) {
        return c + 'A' - 10;
    }
    else {
        return -1;
    }
}


void urlencode(char url[]) {
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BUFSIZE];

    for (i = 0; i < len; i++) {
        char c = url[i];
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '/' || c == '.') {
            res[res_len++] = c;
        }
        else {
            int j = (short int )c;
            if (j < 0)
                j += 256;
            int  i1, i0;
            i1 = j / 16;
            i0 = j - i1 * 16;
            res[res_len++] = '%';
            res[res_len++] = dec2hex(i1);
            res[res_len++] = dec2hex(i0);
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}


void urldecode(char url[]) {
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BUFSIZE];

    for (i = 0; i < len; i++) {
        char c = url[i];
        if (c != '%') {
            res[res_len++] = c;
        }
        else {
            char c1 = url[++i];
            char c0 = url[++i];
            int num = 0;
            num = hex2dec(c1) * 16 + hex2dec(c0);
            res[res_len++] = num;
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}




