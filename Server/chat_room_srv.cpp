#include "chat_room_srv.h"
template<typename T>
chat_srv<T>::chat_srv() {

}


template<typename T>
chat_srv<T>::~chat_srv() {
    close(m_epollfd);
    close(m_listenfd);
    delete m_pool;
}

template<typename T>
void chat_srv<T>::init(int port , string host, string user, string passWord, string databaseName,
            int sql_num, int thread_num){
    m_port = port;
    m_user = user;
    m_passWord = passWord;
    m_databaseName = databaseName;
    m_sql_num = sql_num;
    m_thread_num = thread_num;     
    m_host = host;        
}

template<typename T>
void chat_srv<T>::thread_pool(){
    m_pool = new thread_pool<T>(m_connPool, m_thread_num)
}

template<typename T>
void chat_srv<T>::sql_pool(){
    m_connPool = connection_pool::GetInstance();
    m_connPool->init(host, m_user, m_passWord, m_databaseName, 3306, m_sql_num, m_close_log);
}

template<typename T>
void chat_srv<T>::eventListen(){
    List_Init(OnlineList , online_t);
    int len;
    int optval;
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
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);
    addfd(epoll_fd, listen_fd, false, 0);
    setnonblocking(listen_fd);
}

template<typename T>
void chat_srv<T>::eventLoop(){
    bool stop_server = false;
    while(!stop_server){
        int number = epoll_wait(epoll_fd, events, MAX_CLIENT, -1);
        if(number < 0) break; //错误
        for(int i = 0; i < number; ++i) {
            printf("\nget %d epoll msg\n", number);
            int sockfd = events[i].data.fd;
            if (listen_fd == sockfd) {
                client_fd = accept(listen_fd , (struct sockaddr *)&client_addr , (socklen_t *)&len);
                printf("get new client %d\n", client_fd);
                addfd(epoll_fd, client_fd, false, 1);
                m_pool->append(new request(client_fd));
            }else if(events[i].events & EPOLLIN) {
                printf("client %d send mss\n", sockfd);
                m_pool->append(new request(sockfd));
            }
            printf("have process one epoll_request\n");
        }
        if(client_fd < 0) {
            perror("accept");
            exit(0);
        }
    }
}