#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <event2/event.h>
#include <event2/bufferevent.h>


void read_cb(struct bufferevent *bev, void *arg)
{
    char buf[1024] = {0}; 
    bufferevent_read(bev, buf, sizeof(buf));
    printf("Server say: %s\n", buf);
}

void write_cb(struct bufferevent *bev, void *arg)
{
    printf("I am Write_cb function....\n");
}

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
    else if(events & BEV_EVENT_CONNECTED)
    {
        printf("成功连接到服务器, O(∩_∩)O哈哈~\n");
        return;
    }
    
    bufferevent_free(bev);
    printf("free bufferevent...\n");
}

void send_cb(evutil_socket_t fd, short what, void *arg)
{
    char buf[1024] = {0}; 
    struct bufferevent* bev = (struct bufferevent*)arg;
    printf("请输入要发送的数据: \n");
    read(fd, buf, sizeof(buf));
    bufferevent_write(bev, buf, strlen(buf)+1);
}


int main(int argc, const char* argv[])
{
    struct event_base* base; // 构建一个事件处理平台
    base = event_base_new(); // 搭建一个新的事件处理器


    struct bufferevent* bev; // 构建一个buffer对象
    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE); // 初始化

    // 连接服务器
    struct sockaddr_in serv; // 构建一个服务器
    memset(&serv, 0, sizeof(serv)); // 分配内存
    serv.sin_family = AF_INET; // 选择协议族
    serv.sin_port = htons(9876); // 选择端口
    evutil_inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr); // 选择IP地址
    bufferevent_socket_connect(bev, (struct sockaddr*)&serv, sizeof(serv)); // 请求链接

    // 设置回调:分别是读事件的毁掉，写事件的回调和触发事件的回调
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL); 
    bufferevent_enable(bev, EV_READ | EV_PERSIST); // 设置好之后还需要使能才能使用

    // 创建一个事件
    struct event* ev = event_new(base, STDIN_FILENO, 
                                 EV_READ | EV_PERSIST, // 创建读事件和阻塞事件
                                 send_cb, bev); // 该事件现在的状态为非未决
    event_add(ev, NULL); // ev状态变成未决
    
    event_base_dispatch(base); // 循环等待事件监听

    event_base_free(base);

    return 0;
}
