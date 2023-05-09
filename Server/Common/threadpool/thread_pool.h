#ifndef THREADPOOL_H
#define THREADPOOL_H
#define LISTEN_NUM 12 //连接请求队列长度
#define MSG_LEN 1024
#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class threadpool
{
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    threadpool(connection_pool *connPool, int thread_number = 8, int max_request = 10000);
    ~threadpool();
    bool append(T *request);
    bool append_p(T *request);

private:
    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void *worker(void *arg);//为什么要用静态成员函数呢-----class specific
    void run();

private:
    int m_thread_number;        //线程池中的线程数
    int m_max_requests;         //请求队列中允许的最大请求数
    pthread_t *m_threads;       //描述线程池的数组，其大小为m_thread_number
    std::list<T *> m_workqueue; //请求队列
    locker m_queuelocker;       //保护请求队列的互斥锁
    sem m_queuestat;            //是否有任务需要处理
    connection_pool *m_connPool;  //数据库
    int m_actor_model;          //模型切换（这个切换是指Reactor/Proactor）
};

template <typename T>
//线程池构造函数
threadpool<T>::threadpool(connection_pool *connPool, int thread_number, int max_requests) : m_thread_number(thread_number), m_max_requests(max_requests), m_threads(NULL),m_connPool(connPool)
{
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();
    m_threads = new pthread_t[m_thread_number];     //pthread_t是长整型
    if (!m_threads)
        throw std::exception();
    for (int i = 0; i < thread_number; ++i)
    {
        //函数原型中的第三个参数，为函数指针，指向处理线程函数的地址。
        //若线程函数为类成员函数，
        //则this指针会作为默认的参数被传进函数中，从而和线程函数参数(void*)不能匹配，不能通过编译。
        //静态成员函数就没有这个问题，因为里面没有this指针。
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        //主要是将线程属性更改为unjoinable，使得主线程分离,便于资源的释放，详见PS
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
}

template <typename T>
//reactor模式下的请求入队
bool threadpool<T>::append(T *request)
{
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    //读写事件
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
//proactor模式下的请求入队
bool threadpool<T>::append_p(T *request)
{
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

//工作线程:pthread_create时就调用了它
template <typename T>
void *threadpool<T>::worker(void *arg)
{
    //调用时 *arg是this！
    //所以该操作其实是获取threadpool对象地址
    threadpool *pool = (threadpool *)arg;
    //线程池中每一个线程创建时都会调用run()，睡眠在队列中
    pool->run();
    return pool;
}

//线程池中的所有线程都睡眠，等待请求队列中新增任务
template <typename T>
void threadpool<T>::run()
{
    while(1){
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        char buf[MSG_LEN];
        int ret ,recv_len;
        T* request = m_workqueue.front();
        cJSON *root ,*item;
        char choice[3];
        int client_fd = request->client_fd;
        recv_len = 0;
        while(recv_len < MSG_LEN){
            ret = 0;
            if((ret = recv(client_fd , buf + recv_len , MSG_LEN - recv_len , 0)) <= 0){
                int uid = Account_Srv_ChIsOnline(-1 , 0 ,client_fd);
                if(uid != -1){
                    Account_Srv_SendIsOnline(uid ,0);
                    //向在线好友发送下线通知
                }
                perror("recv");
                return;  //挂！
            }
            recv_len += ret;
        }
        root = cJSON_Parse(buf);
        item = cJSON_GetObjectItem(root,"type");
        strcpy(choice ,item -> valuestring);
        cJSON_Delete(root);
//        printf("收到: sockfd = %d\n%s\n",client_fd,buf);

        switch(choice[0]){
            case 'L' :
                //登录
                Account_Srv_Login(client_fd , buf);
                break;
            case 'S' :
                //注册
                Account_Srv_SignIn(client_fd , buf);
                break;
            case 'A' :
                //添加好友
                Friends_Srv_Add(client_fd, buf);
                break;
            case 'G' :
                //获取好友列表
                Friends_Srv_GetList(client_fd ,buf);
                break;
            case 'g' :
                //获取群列表
                Group_Srv_GetList(client_fd ,buf);
                break;
            case 'P' :
                //私聊
                Chat_Srv_Private(client_fd,buf);
                break;
            case 'p':
                //群聊
                Chat_Srv_Group(client_fd ,buf);
                break;
            case 'F' :
                //文件
                Chat_Srv_File(buf);
                break;
            case 'O' :
                Account_Srv_Out(client_fd ,buf);
                break;
            case 'a':
                Friends_Srv_Apply(client_fd ,buf);
                break;
            case 'c':
                Group_Srv_Create(client_fd ,buf);
                break;
            case 'M' :
                Group_Srv_AddMember(client_fd ,buf);
                break;
            case 'm':
                Group_Srv_ShowMember(client_fd ,buf);
                break;
            case 'Q' :
                //踢人 退群 解散群
                Group_Srv_Quit(client_fd ,buf);
                break;
            case 'E' :
                //获取私聊聊天记录
                Chat_Srv_SendPrivateRes(client_fd ,buf);
                break;
        }
    }
}
#endif
