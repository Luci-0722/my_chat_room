#ifndef REQUEST
#define REQUEST
#include <pthread.h>
class request
{
private:
    /* data */
    
public:
    pthread_t client_fd;
    request(pthread_t fd);
    ~request();
};

#endif