#ifndef CHAT_ROOM_SRV
#define CHAT_ROOM_SRV
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include<sys/epoll.h>
#include <unordered_map>
#include "Common/cJSON.h"
#include "Persistence/MySQL.h"
#include "./Common/CGImysql/sql_connection_pool.h"
#include "./Common/threadpool/thread_pool.h"
#include "./Common/List.h"
#include "Common/requests/request.h"
const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位
 
 /*
 * socket消息结构
 * type可能的取值:
 *  L:   Login
 *  S:   Sign in
 *  C:   Chat
 *  F:   File
 */



int setnonblocking(int fd);



void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);



void modfd(int epollfd, int fd, int ev, int TRIGMode);


class chat_srv {
public:
    chat_srv();
    ~chat_srv();

    void init(int port , string host, string user, string passWord, string databaseName,
              int sql_num, int thread_num);

    void thread_pool();
    void sql_pool();
    void eventListen();
    void eventLoop();

    connection_pool *m_connPool;
private:
    int m_port;//端口
    int m_epollfd;//epoll对象

    //数据库相关

    string m_user;         //登陆数据库用户名
    string m_passWord;     //登陆数据库密码
    string m_databaseName; //使用数据库名
    int m_sql_num;//数据库连接池数量
    string m_host;
    //线程池相关
    threadpool<request> *m_pool;
    int m_thread_num;

    //epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;//监听套接字

    //工具类
    // Utils utils;

    //服务器类
    // Service service;

    //状态保持类
    // Persistence persistence;

    //用户状态哈希表
    unordered_map<int, int> clients_sock;
};

int printf(int a, int b);
#endif