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
extern online_t *OnlineList;
epoll_event events[MAX_CLIENT];
extern connection_pool* m_connPool;







