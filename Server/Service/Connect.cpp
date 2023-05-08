/************************************************************************
	>    File Name: Connect.c
	>       Author: fujie
	>         Mail: fujie.me@qq.com
	> Created Time: 2017年08月09日 星期三 14时19分48秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "./Connect.h"
#include "./Account_Srv.h"
#include "./Friends_Srv.h"
#include "./Chat_Srv.h"
#include "./Group_Srv.h"
#include "../Common/cJSON.h"
#include "../Common/List.h"
#include "../Persistence/Friends_Persist.h"
#include "../Common/threadpool/thread_pool.h"
#define LISTEN_NUM 12 //连接请求队列长度
#define MSG_LEN 1024
online_t *OnlineList;

void Connect(int port){
    int sock_fd;
    int client_fd;
    int len;
    int optval;
    List_Init(OnlineList , online_t);
    struct sockaddr_in serv_addr , client_addr;
    len = sizeof(struct sockaddr_in);
    memset(&serv_addr , 0 ,len);
    memset(&client_addr , 0 , len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_fd = socket(AF_INET , SOCK_STREAM , 0);
    if(sock_fd < 0) {
        perror("socket");
        exit(0);
    }
    optval = 1;
    if(setsockopt(sock_fd , SOL_SOCKET , SO_REUSEADDR , (void *)&optval , sizeof(int)) < 0){
        perror("socksetopt");
        exit(0);
    }
    if(bind(sock_fd , (struct sockaddr *)&serv_addr , len) < 0){
        perror("bind");
        exit(0);
    }
    if(listen(sock_fd , LISTEN_NUM) < 0){
        perror("listen");
        exit(0);
    }
    while(1){
        client_fd = accept(sock_fd , (struct sockaddr *)&client_addr , (socklen_t *)&len);
        if(client_fd < 0) {
            perror("accept");
            exit(0);
        }
        pthread_t thid;
        pthread_create(&thid , NULL , thread ,(void *)(long)client_fd);
    }
    
}
