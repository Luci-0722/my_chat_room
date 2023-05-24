#include "request.h"
#include <pthread.h>
request::request(pthread_t fd)
{
    client_fd = fd;
}

request::~request()
{
}