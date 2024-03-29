#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

// 读缓冲区回调
void read_cb(struct bufferevent *bev, void *arg)
{
    char buf[1024] = {0};   
    bufferevent_read(bev, buf, sizeof(buf));
    char* p = "我已经收到了你发送的数据!";
    printf("client say: %s\n", p);

    // 发数据给客户端
    bufferevent_write(bev, p, strlen(p)+1);
    printf("====== send buf: %s\n", p);
}

// 写缓冲区回调
void write_cb(struct bufferevent *bev, void *arg)
{
    printf("我是写缓冲区的回调函数...\n"); 
}

// 事件
void event_cb(struct bufferevent *bev, short events, void *arg)
{
    if (events & BEV_EVENT_EOF)
    {
        printf("connection closed\n");  
    }
    else if(events & BEV_EVENT_ERROR)   
    {
        printf("some other error\n");
    }
    
    bufferevent_free(bev);    
    printf("buffevent 资源已经被释放...\n"); 
}



void cb_listener(
        struct evconnlistener *listener, 
        evutil_socket_t fd, // 这个文件描述符是libevent这个framework内部传入的一个用于通信的文件描述符
        struct sockaddr *addr, 
        int len, void *ptr)
{
   printf("connect new client\n");

   struct event_base* base = (struct event_base*)ptr;
   // 通信操作
   // 添加新事件
   struct bufferevent *bev; //为了使用带缓存区的事件，需要再定义一个缓存区
   //将缓存区和base合二为一
   bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

   // 给bufferevent缓冲区设置回调, 缓存区有数据即调用read_cb，
   bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
   bufferevent_enable(bev, EV_READ); // 默认读回调是关闭的，因此需要enable
}


int main(int argc, const char* argv[])
{

    // init server 
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(9876);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    struct event_base* base;
    base = event_base_new();
    // 创建套接字
    // 绑定
    // 接收连接请求
    struct evconnlistener* listener;

    
    listener = evconnlistener_new_bind(base, cb_listener, base, 
                                  LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                                  36, (struct sockaddr*)&serv, sizeof(serv));

    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}
