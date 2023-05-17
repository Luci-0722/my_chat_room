#ifndef REQUEST
#define REQUEST
class request
{
private:
    /* data */
    
public:
    pthread_t client_fd;
    request(pthread_t fd);
    ~request();
};

request::request(pthread_t fd)
{
    client_fd = fd;
}

request::~request()
{
}
#endif