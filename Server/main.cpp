/*************************************************************************
	>    File Name: char_room_srv.c
	>       Author: fujie
	>         Mail: fujie.me@qq.com
	> Created Time: 2017年08月10日 星期四 15时17分19秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include<sys/epoll.h>
#include "Common/cJSON.h"
#include "Service/Connect.h"
#include "Persistence/MySQL.h"
#include "./Common/CGImysql/sql_connection_pool.h"
#include "./chat_room_srv.h"
#include "request.h"
int main(){
    char buf[1024];
    char host[50] ,user[30],pass[50],database[50];
    int fd = open("config.json" ,O_RDONLY);
    if(fd == -1) {
        printf("配置文件打开失败!");
        getchar();
        exit(0);
    }
    read(fd ,buf ,1024);
    cJSON* root = cJSON_Parse(buf);
    cJSON* item = cJSON_GetObjectItem(root ,"host");
    strcpy(host ,item -> valuestring);
    item = cJSON_GetObjectItem(root ,"user");
    strcpy(user ,item -> valuestring);
    item = cJSON_GetObjectItem(root ,"pass");
    strcpy(pass ,item -> valuestring);
    item = cJSON_GetObjectItem(root ,"database");
    strcpy(database ,item -> valuestring);
    item = cJSON_GetObjectItem(root ,"port");
    int port = item -> valueint;
    close(fd);
    cJSON_Delete(root);
    //初始化char_rom服务器
    chat_srv<request> chat_server;
    chat_server.init(port, host, user, pass, database, 5, 8);

    //数据库
    chat_server.sql_pool();

    //线程池
    chat_server.thread_pool();

    //监听
    chat_server.eventListen();

    //运行
    chat_server.eventLoop();
}
