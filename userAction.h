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

#ifndef USER_ACTION_H
#define USER_ACTION_H

#include "commonFormats.h"

//其实最后没用到,请参考其他三人的定义
typedef struct user
{
    struct sockaddr_in peer;
    char   name[NAMELEN];
    char   host[NAMELEN];
    char   nickname[NAMELEN];
    int    inUse;
    int    exit;
    struct user *next;
} user;

extern user userList;

extern int insertUser(user *uList, user *target);
extern void destroyUsers(user *uList);
extern int listUsers(user **pusers, user *uList, int size, int flag);
extern int unListUsers(user **pusers, int num);
extern int delUser(user *uList, command *peercom);

#endif //USER_ACTION_H
