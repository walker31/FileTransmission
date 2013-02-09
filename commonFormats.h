/*
* Copyright 2012	Team#6, F2-101, Neusoft
*
* This file defines common formats for Linpop, a LAN chatting program
* Linpop follows the IP Messenger communication protocol (Draft-9)
* you can obtain a copy of Linpop at
*
*	http://code.google.com/p/linpop
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the Apache 2.0 License.
*/
#ifndef COMMMON_FORMATS_H
#define COMMMON_FORMATS_H

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <semaphore.h>

#define NAMELEN 50
#define MSGLEN  1000
#define COMLEN  1500
#define RECFRG  1448
#define HSIZE   10
#define FILENAME 128
#define MSGLIMIT 100
#define HL_HEADERSIZE	4
#define HL_FILESIZE	9
#define HL_FILETYPE	1
#define HL_1416		11
#define CAPACITY     50

#define BUFLEN 1024
#define PORT 9741
#define LOGIN_PORT 54321
#define REGISTER_PORT 54320
#define MSG_PORT 54319

#define UL unsigned long
#define UNIT unsigned int

#define ERR(x) {perror("x"); exit(1);}

#define IMHELP    \
  "---------------------------------------------------------\n"\
  "\tLINPOP File Transmission Support Module \n"\
  "\tI am now Ready to send and receive file!\n"\
  "---------------------------------------------------------\n"

//文件序号:文件名:大小(单位:字节):最后修改时间:文件属性 [: 附加属性=val1[,val2…][:附加信息=…]]:\a文件序号…
typedef struct filenode
{
    int    fileSock;
    unsigned int    fileNo;
    char   fileName[FILENAME];
    char   fileSize[NAMELEN];
    char   mtime[NAMELEN];
    int    fileType;
    char   otherAttrs[2*NAMELEN];
    struct filenode* next;
} filenode;

typedef struct ClientRequest
{
    unsigned int requestType;
    char senderName[NAMELEN];
    char senderPasswd[9];
    char sourceIP[16];
    char destIP[16];
    char nickName[NAMELEN];
    int state;
    char gender[1];
    unsigned int age;
    char branch[NAMELEN];
    char telephone[12];
    char birthdate[12];
    unsigned int profilepic;
    char msg[MSGLEN];
} clientRequest;

typedef struct command
{
    unsigned int version;
    unsigned int packetNo;
    char         senderName[NAMELEN];
    char         senderHost[NAMELEN];
    unsigned int commandNo;
    char         additional[MSGLEN];
    struct sockaddr_in peer;
    filenode    *fileList;
    struct command *next;
} command;

typedef int (*Mysnd)(int, const char*, int, int);

typedef struct gsNode
{
    int fileSock;
    struct sockaddr_in peer;
    unsigned int packetNo;
    int	 transferring;
    int    cancelled;
    char  *targetDir;
    filenode fileList;
    struct gsNode *next;
} gsNode;

typedef struct msgList
{
    command comHead;
    command *comTail;
} msgList;


extern const char allHosts[]; //广播地址
extern int msgSock; //消息
extern int fileSock; //文件
extern struct passwd* pwd;
extern struct utsname sysName;
extern char workDir[FILENAME];
extern int utf8; //系统的LC_CTYPE

extern gsNode sendFileHead, getFHead; //发送和接收文件列表
extern msgList mList; //接受到的消息列表

extern pthread_mutex_t sendFMutex; //发送文件
extern pthread_mutex_t getFMutex;  //接收文件
extern pthread_mutex_t msgMutex;   //消息队列
extern pthread_mutex_t usrMutex;   //消息队列
extern sem_t waitNonEmpty, waitNonFull; //消息列表信号量

extern int deMsg(char *msg, int size, command* com);
extern int initMsg(char* msg, command* com, size_t msgLen);
extern filenode* getFilelist(const char* comFiles);
extern void initCommand(command *com, unsigned int flag);
extern void deCommand(command *com);
extern void initGsNode(gsNode *gs);
extern void deGsNode(gsNode *gs);

#endif  //COMMON_FORMATS_H
