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

#include "userAction.h"
#include "ipmsg.h"
#include "send_receive.h"
#include "commonFormats.h"

#include <langinfo.h>
#include <locale.h>
#include <pthread.h>

extern void destroyer();

typedef struct ExecOption
{
    char doWhat[5];
    char fullpath[50];
    char sendtoIP[20];
} execOption;

//用户输入
void* interacter(void* voidoption)
{
    char com[20], *fileName;
    int count;

    struct ExecOption *execoption =(struct ExecOption *)voidoption;

    ///显式转换参数
    char* operation;
    char* filepath;
    char* destIP;
    operation = execoption->doWhat;
    filepath = execoption->fullpath;
    destIP = execoption->sendtoIP;

    printf("finised initialized args[]\n");
    printf("argv[] = %s, %s, %s\n", operation, filepath, destIP);
    //等待马哥的窗口响应
    login();
    sleep(1);
    login();

    memset(com, '\0', strlen(com));
    strcpy(com, operation);
    printf("finished filling Command\n");

///////////////////////////////////////////////////////////
    //准备好让接收端多飞一会儿，别和下面的处理函数混淆了
    if( !strcmp(com,"gf") ||
            !strcmp(com, "g"))
    {
        sleep(3);
        login();
        sleep(2);
        printf("\n***now sending GetFile signal***\n");
    }
///////////////////////////////////////////////////////////

    if (!strcmp(com, "sendfile") ||
             !strcmp(com, "sf") ||
             !strcmp(com, "s") )
    {
        selectFiles(filepath, destIP);
        ///显式调用selectFiles(目标文件, 目标IP);
    }
    else if (!strcmp(com, "getfile") ||
             !strcmp(com, "gf") ||
             !strcmp(com,"g") )
    {
        //刷新一下
        login();
        pthread_mutex_lock(&usrMutex);
        count = listUsers(NULL, &userList, 2, 1);
        pthread_mutex_unlock(&usrMutex);

        sleep(1);
        //设置默认路径，保存在项目根目录下，不允许自定义
        recvFiles(filepath);
        ///显式调用recvFiles(保存路径);
    }
    else if (!strcmp(com, "cancelSend") ||
             !strcmp(com, "cs"))
    {
        cancelSending();
    }

}


//接收udp包
void* receiver(void *option)
{
    command *peercom;
    struct sockaddr_in peer;
    int mSock = *(int*)option, len;
    char buf[COMLEN];

    while(1)
    {
        if (recvfrom(mSock, buf, sizeof(buf), 0, (struct sockaddr*)&peer, &len)<0)
            continue;
        peercom = (command*)malloc(sizeof(command));
        bzero(peercom, sizeof(command));
        deMsg(buf, sizeof(buf), peercom);
        memcpy(&peercom->peer, &peer, sizeof(peercom->peer));
        /*上锁*/
        sem_wait(&waitNonFull);
        pthread_mutex_lock(&msgMutex);

        mList.comTail->next = peercom;
        mList.comTail = peercom;

        /*解锁*/
        sem_post(&waitNonEmpty);
        pthread_mutex_unlock(&msgMutex);
    }
}

//处理接收的udp包
void* processor(void *option)
{
    command *peercom, com;
    int comMode, comOpt, temp;
    int len;
    user *cur;
    filenode *head, *curFile;
    gsNode *preSend, *curSend, *curGet, *preGet;

    initCommand(&com, IPMSG_NOOPERATION);
    while(1)
    {
        sem_wait(&waitNonEmpty);
        pthread_mutex_lock(&msgMutex);

        peercom = mList.comHead.next;
        mList.comHead.next = mList.comHead.next->next;
        if (mList.comHead.next == NULL)
            mList.comTail = &mList.comHead;

        sem_post(&waitNonFull);
        pthread_mutex_unlock(&msgMutex);

        memcpy(&com.peer, &peercom->peer, sizeof(com.peer));

        comMode = GET_MODE(peercom->commandNo);
        comOpt = GET_OPT(peercom->commandNo);

        if (comOpt & IPMSG_SENDCHECKOPT)
        {
            com.packetNo = (unsigned int)time(NULL);
            snprintf(com.additional, MSGLEN, "%d", peercom->packetNo);
            com.commandNo = IPMSG_RECVMSG;
            sendMsg(&com); //发送回应
        }

        switch (comMode)
        {
        case IPMSG_SENDMSG: //发送命令
            if (strlen(peercom->additional)>0)
            {
                printf("\nGet message from: %s(%s)\n", peercom->senderName, peercom->senderHost);
                puts(peercom->additional);
            }

            if (comOpt & IPMSG_FILEATTACHOPT)
            {
                printf("\nFile Transmission Request from: %s(%s).\n",
                       peercom->senderName, peercom->senderHost);
                curGet = (gsNode*)malloc(sizeof(gsNode));
                initGsNode(curGet);
                memcpy(&curGet->peer, &peercom->peer, sizeof(curGet->peer));
                curGet->packetNo = peercom->packetNo;
                curGet->fileList.next = peercom->fileList;
                peercom->fileList = NULL; //

                preGet = &getFHead;
                pthread_mutex_lock(&getFMutex);
                while ((preGet->next!=NULL) &&
                        (preGet->next->packetNo!=curGet->packetNo))
                    preGet = preGet->next;

                if (preGet->next==NULL)
                    preGet->next = curGet;

                pthread_mutex_unlock(&getFMutex);
            }

            break;
        case IPMSG_ANSENTRY: //
            cur = (user*)malloc(sizeof(user));
            memcpy(&cur->peer, &peercom->peer, sizeof(cur->peer));
            strncpy(cur->name, peercom->senderName, NAMELEN);
            strncpy(cur->host, peercom->senderHost, NAMELEN);
            strncpy(cur->nickname, peercom->additional, NAMELEN);
            cur->inUse = 0;
            cur->exit = 0;
            cur->next = NULL;
            pthread_mutex_lock(&usrMutex); //lock
            if (insertUser(&userList, cur)<0)
                free(cur);
            pthread_mutex_unlock(&usrMutex); //unlock
            break;
        case IPMSG_BR_ENTRY:
            com.packetNo = (unsigned int)time(NULL);
            com.commandNo = IPMSG_ANSENTRY;//
            strncpy(com.additional, pwd->pw_name, MSGLEN);
            sendMsg(&com);

            cur = (user*)malloc(sizeof(user));
            memcpy(&cur->peer, &peercom->peer, sizeof(cur->peer));
            strncpy(cur->name, peercom->senderName, NAMELEN);
            strncpy(cur->host, peercom->senderHost, NAMELEN);
            strncpy(cur->nickname, peercom->additional, NAMELEN);
            cur->inUse = 0;
            cur->exit = 0;
            cur->next = NULL;
            pthread_mutex_lock(&usrMutex);
            if (insertUser(&userList, cur)<0)
                free(cur);
            pthread_mutex_unlock(&usrMutex);
            break;
        case IPMSG_RECVMSG:
            //
            break;

        case IPMSG_BR_EXIT:
            pthread_mutex_lock(&usrMutex);
            delUser(&userList, peercom);
            pthread_mutex_unlock(&usrMutex);
            break;
        case IPMSG_NOOPERATION:
            //
            break;
        default:
            printf("\nno handle, %x\n", peercom->commandNo);
            break;
        }
        deCommand(peercom);
        free(peercom);
        peercom = NULL;
    }

}

//数据清理
void destroyer()
{
    gsNode *preSend, *curSend, *preGet, *curGet;
    filenode *curFile;
    user *curUsr, *preUsr;

    preSend = &sendFileHead;
    pthread_mutex_lock(&sendFMutex);
    curSend = sendFileHead.next;
    while (curSend!=NULL)
    {
        if ((curSend->cancelled == 1) && (curSend->transferring==0))
        {
            preSend->next = curSend->next;
            deGsNode(curSend);
            free(curSend);
        }
        else preSend = preSend->next;

        curSend = preSend->next;
    }
    pthread_mutex_unlock(&sendFMutex);


    preGet = &getFHead;
    pthread_mutex_lock(&getFMutex);
    curGet = getFHead.next;
    while (curGet!=NULL)
    {
        if ((curGet->cancelled==1) &&(curGet->transferring==0))
        {
            preGet->next = curGet->next;
            deGsNode(curGet);
            free(curGet);
        }
        else preGet = preGet->next;

        curGet = preGet->next;
    }
    pthread_mutex_unlock(&getFMutex);

    preUsr = &userList;
    pthread_mutex_lock(&usrMutex);
    curUsr = userList.next;
    while (curUsr!=NULL)
    {
        if ((curUsr->exit==1) && (curUsr->inUse==0))
        {
            preUsr->next = curUsr->next;
            free(curUsr);
        }
        else preUsr = preUsr->next;

        curUsr = preUsr->next;
    }
    pthread_mutex_unlock(&usrMutex);
}

void* cleaner(void *option)
{
    gsNode *preSend, *curSend, *preGet, *curGet;
    filenode *curFile;
    user *curUsr, *preUsr;

    while(1)
    {
        sleep(20); //20秒钟清理一次
        destroyer();
    }
}

//初始化udp和tcp
int initialize()
{
    struct sockaddr_in server;
    char targetHost[NAMELEN];
    const int on=1;

    msgSock = socket(AF_INET, SOCK_DGRAM, 0);  //UDP for Msg

    fileSock = socket(AF_INET, SOCK_STREAM, 0); //TCP for File

    server.sin_family = AF_INET;
    server.sin_port = htons(IPMSG_DEFAULT_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt(msgSock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))<0)
    {
        printf("LPFSupport: Set Socket Option Failed.\n");
        exit(1);
    }

    if (bind(msgSock, (struct sockaddr*)&server, sizeof(server))<0)
    {
        printf("LPFSupport: Udp socket bind error.\n");
        exit(1);
    }

    if (bind(fileSock, (struct sockaddr*)&server, sizeof(server))<0)
    {
        printf("LPFSupport: Tcp socket bind error.\n");
        exit(1);
    }

    if (listen(fileSock, 10)<0)
    {
        printf("LPFSupport:  Tcp listen error.\n");
        exit(1);
    }
    printf(IMHELP);
}

int main (int argc, char* argv[])
{
    execOption execoption;
    bzero(&execoption, sizeof(execOption));
    printf("\nCmdline argc=%d argv[] = %s, %s, %s\n"
           , argc
           , argv[1]
           , argv[2]
           , argv[3]);

    if(argc<2 || argc >4)
    {
        printf("Usage: ./lLPFSupport <sf/gf> [filepath] [destination IP]\n\n");
        return -1;
    }
    if(argc == 4)
    {
        strcpy(execoption.doWhat, argv[1]);
        strcpy(execoption.fullpath, argv[2]);
        strcpy(execoption.sendtoIP, argv[3]);
    }
    if(argc == 2)
    {
        strcpy(execoption.doWhat, argv[1]);
        strcpy(execoption.fullpath, ".");
        strcpy(execoption.sendtoIP, "");
    }
    printf("received argv[] = %s, %s, %s\n"
           , execoption.doWhat
           , execoption.fullpath
           , execoption.sendtoIP);
    /*
    ///可以定义三个参数如下：
    ///argv[1]: sf/gf: 区分发送文件和接收文件
    ///argv[2]: 目标路径
    ///argv[3]: 发送时的目标IP
    void* arg[3];

    if(argc == 4)
    {  // 发送指令sf - 源文件路径 - 目标IP
    //       argv[1] = "sf";
    //      argv[2] = "/home/new/a.c";
    //        argv[3] = "127.0.0.1";
        void* arg[3] = {argv[1], argv[2], argv[3]};

        //printf("arg_entered: %s, %s, %s\n", argv[1], argv[2], argv[3]);
        printf("argv received: %s, %s, %s\n", (char*)arg[0], (char*)arg[1], (char*)arg[2]);
    }
    else if (argc == 3)
    {  // 接收指令gf - 保存路径
        void* arg[3] = {argv[1], argv[2], NULL};
        printf("argv received: %s, %s\n", (char*)arg[0], (char*)arg[1]);
        //printf("arg_entered: %s, %s\n", argv[1], argv[2]);
    }
    else
    {
        printf("[Usage]: ./LPfsupport operation argv[1] argv[2]\n");
        return 1;
    }
        //return 1;

    ///参数数组ended here
    */

    pthread_t procer, recver, iter, fler, cler;
    int *fSock;
    int tmp;
    pthread_attr_t attr;

    uname(&sysName);
    pwd = getpwuid(getuid());
    getcwd(workDir, sizeof(workDir));

    utf8 = 0;
    if (setlocale(LC_CTYPE, ""))
        if (!strcmp(nl_langinfo(CODESET), "UTF-8"))
            utf8 = 1;

    initGsNode(&sendFileHead);
    initCommand(&mList.comHead, IPMSG_NOOPERATION);
    mList.comTail = &mList.comHead;
    userList.next = NULL;
    sem_init(&waitNonEmpty, 0, 0);
    sem_init(&waitNonFull, 0, MSGLIMIT);

    initialize();
    pthread_create(&procer, NULL, &processor, &msgSock);
    pthread_create(&recver, NULL, &receiver, &msgSock);
    //pthread_create(&iter, NULL, interacter, NULL);
    if (-1 == pthread_create(&iter, NULL, &interacter, &execoption))
        perror("pthread error");
    pthread_create(&cler, NULL, &cleaner, NULL);

    login();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    while(1)
    {
        if ((tmp=accept(fileSock, NULL, NULL))<0) //不管是谁发送都可以accept
            printf("Tcp accept error.\n");
        else
        {
            fSock = (int *)malloc(sizeof(int));
            *fSock = tmp;
            pthread_create(&fler, &attr, &sendData, fSock);
            ///需要发送东西时，创建独立线程搞起来发数据
        }
    }

    pthread_join(procer, NULL);
    pthread_join(recver, NULL);
    pthread_join(iter, NULL);
    pthread_join(cler, NULL);
    return 0;
}
