#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <event2/event.h>

// 对操作处理的回调函数，参数要固定
void read_cb(evutil_socket_t fd, short what, void *arg)
{
    // 读管道
    char buf[1024] = {0};
    int len = read(fd, buf, sizeof(buf));
    printf("data len = %d, buf = %s\n", len, buf);
    printf("read event: %s", what & EV_READ ? "Yes" : "No");
}


// 读管道
int main(int argc, const char* argv[])
{
    unlink("myfifo"); // 如果myfifo这个文件已经存在，则删除
    //创建有名管道
    mkfifo("myfifo", 0664);

    // open file
    int fd = open("myfifo", O_RDONLY | O_NONBLOCK);
    if(fd == -1)
    {
        perror("open error");
        exit(1);
    }

    // 读管道
    struct event_base* base = NULL;  // 创建一个事件处理框架
    base = event_base_new();  // 初始化

    // 创建事件
    struct event* ev = NULL;
    ev = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, NULL);

    // 添加事件，NULL表示阻塞等待事件
    event_add(ev, NULL);

    // 事件循环，等待事件处理
    event_base_dispatch(base);

    // 释放资源
    event_free(ev);
    event_base_free(base);
    close(fd);
    
    return 0;
}
