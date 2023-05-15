/************************************************************************
	>    File Name: Connect.c
	>       Author: fujie
	>         Mail: fujie.me@qq.com
	> Created Time: 2017年08月09日 星期三 14时19分48秒
 ************************************************************************/
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "./Connect.h"
#include "./Account_Srv.h"
#include "./Friends_Srv.h"
#include "./Chat_Srv.h"
#include "./Group_Srv.h"
#include "../Common/cJSON.h"
#include "../Common/List.h"
#include "../Persistence/Friends_Persist.h"
#include "../Common/threadpool/thread_pool.h"
#include "../Common/requests/request.h"
#define LISTEN_NUM 12 //连接请求队列长度
#define MSG_LEN 1024
const int MAX_CLIENT = 1024;
threadpool<request> *my_thread_pool;
online_t *OnlineList;
epoll_event events[MAX_CLIENT];
extern connection_pool* m_connPool;
int epoll_fd;
int setnonblocking(int fd)
{
    //fcntl可以获取/设置文件描述符性质
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}


void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    //如果对描述符socket注册了EPOLLONESHOT事件，
    //那么操作系统最多触发其上注册的一个可读、可写或者异常事件，且只触发一次。
    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}


void modfd(int epollfd, int fd, int ev, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void Connect(int port){
    my_thread_pool = new threadpool<request>(m_connPool);
    printf("is connecting\n");
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
    int listen_fd = socket(AF_INET , SOCK_STREAM , 0);
    setnonblocking(listen_fd);
    if(listen_fd < 0) {
        perror("socket");
        exit(0);
    }
    optval = 1;
    if(setsockopt(listen_fd , SOL_SOCKET , SO_REUSEADDR , (void *)&optval , sizeof(int)) < 0){
        perror("socksetopt");
        exit(0);
    }
    if(bind(listen_fd, (struct sockaddr *)&serv_addr , len) < 0){
        perror("bind");
        exit(0);
    }
    if(listen(listen_fd , LISTEN_NUM) < 0){
        perror("listen");
        exit(0);
    }
    epoll_fd = epoll_create(5);
    addfd(epoll_fd, listen_fd, false, 0);
    setnonblocking(listen_fd);
    while(1){
        int number = epoll_wait(epoll_fd, events, MAX_CLIENT, -1);
        if(number < 0) break; //错误
        for(int i = 0; i < number; ++i) {
            printf("\nget %d epoll msg\n", number);
            int sockfd = events[i].data.fd;
            if (listen_fd == sockfd) {
                client_fd = accept(listen_fd , (struct sockaddr *)&client_addr , (socklen_t *)&len);
                printf("get new client %d\n", client_fd);
                addfd(epoll_fd, client_fd, false, 1);
                my_thread_pool->append(new request(client_fd));
            }else if(events[i].events & EPOLLIN) {
                printf("client %d send mss\n", sockfd);
                my_thread_pool->append(new request(sockfd));
            }
            printf("have process one epoll_request\n");
        }
        if(client_fd < 0) {
            delete my_thread_pool;
            perror("accept");
            exit(0);
        }
    }
    
}
