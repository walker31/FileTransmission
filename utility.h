/*
* Copyright 2012	Team#6, F2-101, Neusoft
*
* Linpop follows the IP Messenger communication protocol (Draft-9)
* you can obtain a copy of Linpop at
*
*	http://code.google.com/p/linpop
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the Apache 2.0 License.
*/

#ifndef UTILITY_H
#define UTILITY_H

#include <unistd.h>

extern ssize_t readn(int fd, void *buf, size_t count);
extern ssize_t readMaohao(int fd, void *buf, ssize_t count, char ch);
extern ssize_t writen(int fd, const void *buf, size_t count);
extern ssize_t readline(int fd, void *buf, int size);
extern void delMaohao(char* dest, int size);
extern void addMaohao(char* dest, int size);
extern int selectOperation(int min, int max, int def, char *prom);
extern int getFileName(char* dest, const char* fullpath, int size);
extern int getParentPath(char *dest, int size);
extern void fuckTheString(char *dest, int flag);

#endif //UTILITY_H
