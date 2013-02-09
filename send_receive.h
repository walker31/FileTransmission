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

#ifndef SEND_RECV_H
#define SEND_RECV_H

#include "commonFormats.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

extern void* sendMsg(command* com);
extern int Chat();
extern int selectFiles();
extern void* sendData(void* option);
extern int sendDir(int fSock, const char* fullpath, int fileSize, int fileType);
extern int traverseDir(int fSock, char* fullpath, Mysnd snd);
extern int listReadToSend(gsNode **list, gsNode *gs, int size);
extern int cancelSending();
extern int recvFiles();
extern void* getData(void* option);
extern int getFile(void* option, gsNode *gList);
extern int parseHeader(filenode *pfn, char * recvs);
extern int getDir(void *option, gsNode *gList);
extern int listGFiles(gsNode **list, gsNode *gs, int size);
extern int login();
extern int logout();


#endif //SEND_RECV_H
