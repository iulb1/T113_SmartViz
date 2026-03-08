/**
 * 使用socketpair实现全双工通信
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

int socket_pair[2];  // socket对，两端都可以读写

void *thread_a(void *arg) {
    char buf[256];
    
    // 向socket写入数据
    strcpy(buf, "消息来自线程A");
    write(socket_pair[0], buf, strlen(buf));
    printf("[线程A] 发送: %s\n", buf);
    
    // 从socket读取数据
    memset(buf, 0, sizeof(buf));
    read(socket_pair[0], buf, sizeof(buf));
    printf("[线程A] 接收: %s\n", buf);
    
    close(socket_pair[0]);
    return NULL;
}

void *thread_b(void *arg) {
    char buf[256];
    
    // 从socket读取数据
    memset(buf, 0, sizeof(buf));
    read(socket_pair[1], buf, sizeof(buf));
    printf("[线程B] 接收: %s\n", buf);
    
    // 向socket写入数据
    strcpy(buf, "消息来自线程B");
    write(socket_pair[1], buf, strlen(buf));
    printf("[线程B] 发送: %s\n", buf);
    
    close(socket_pair[1]);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    // 创建socket对（全双工）
    socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair);
    
    pthread_create(&t1, NULL, thread_a, NULL);
    pthread_create(&t2, NULL, thread_b, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    return 0;
}
