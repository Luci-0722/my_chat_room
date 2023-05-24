#include "chat_room_srv.h"
#define MAX_CLIENT 1024
extern online_t *OnlineList;
int printf(int a, int b) {
    return a + b;
}

chat_srv::chat_srv() {

}



chat_srv::~chat_srv() {
    close(m_epollfd);
    close(m_listenfd);
    delete m_pool;
}


void chat_srv::init(int port , string host, string user, string passWord, string databaseName,
            int sql_num, int thread_num){
    m_port = port;
    m_user = user;
    m_passWord = passWord;
    m_databaseName = databaseName;
    m_sql_num = sql_num;
    m_thread_num = thread_num;     
    m_host = host;        
}


void chat_srv::thread_pool(){
    m_pool = new threadpool<request>(m_connPool, m_thread_num);
}


void chat_srv::sql_pool(){
    m_connPool = connection_pool::GetInstance();
    m_connPool->init(m_host, m_user, m_passWord, m_databaseName, m_port, m_sql_num);
}


void chat_srv::eventListen(){
    List_Init(OnlineList , online_t)
    int len;
    int optval;
    struct sockaddr_in serv_addr , client_addr;
    len = sizeof(struct sockaddr_in);
    memset(&serv_addr , 0 ,len);
    memset(&client_addr , 0 , len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(m_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_listenfd = socket(AF_INET , SOCK_STREAM , 0);
    setnonblocking(m_listenfd);
    if(m_listenfd < 0) {
        perror("socket");
        exit(0);
    }
    optval = 1;
    if(setsockopt(m_listenfd , SOL_SOCKET , SO_REUSEADDR , (void *)&optval , sizeof(int)) < 0){
        perror("socksetopt");
        exit(0);
    }
    if(bind(m_listenfd, (struct sockaddr *)&serv_addr , len) < 0){
        perror("bind");
        exit(0);
    }
    if(listen(m_listenfd , LISTEN_NUM) < 0){
        perror("listen");
        exit(0);
    }
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);
    addfd(m_epollfd, m_listenfd, false, 0);
    setnonblocking(m_listenfd);
}


void chat_srv::eventLoop(){
    bool stop_server = false;
    struct sockaddr_in serv_addr , client_addr;
    int len = sizeof(struct sockaddr_in);
    while(!stop_server){
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0) break; //错误
        for(int i = 0; i < number; ++i) {
            printf("\nget %d epoll msg\n", number);
            int sockfd = events[i].data.fd;
            if (m_listenfd == sockfd) {
                int client_fd = accept(m_listenfd , (struct sockaddr *)&client_addr , (socklen_t *)&len);
                printf("get new client %d\n", client_fd);
                addfd(m_epollfd, client_fd, false, 1);
                m_pool->append(new request(client_fd));
            }else if(events[i].events & EPOLLIN) {
                printf("client %d send mss\n", sockfd);
                m_pool->append(new request(sockfd));
            }
            printf("have process one epoll_request\n");
        }
    }
}

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