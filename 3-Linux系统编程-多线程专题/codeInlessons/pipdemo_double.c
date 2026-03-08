/**
 * 使用两个管道实现全双工通信
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

int pipe1[2];  // 管道1：线程A -> 线程B
int pipe2[2];  // 管道2：线程B -> 线程A

void *thread_a(void *arg) {
    char buf[256];
   	printf("线程A启动\n"); 
    // 关闭不需要的端
  //  close(pipe1[0]);  // 关闭管道1的读端（A只写）
  //  close(pipe2[1]);  // 关闭管道2的写端（A只读）

    // 向管道1写入数据（A -> B）
    strcpy(buf, "消息来自线程A");
    write(pipe1[1], buf, strlen(buf));
    printf("[线程A] 发送: %s\n", buf);
    
    // 从管道2读取数据（B -> A）
    memset(buf, 0, sizeof(buf));
    read(pipe2[0], buf, sizeof(buf));
    printf("[线程A] 接收: %s\n", buf);
    
	sleep(1);    
    close(pipe1[1]);
    close(pipe2[0]);
    return NULL;
}

void *thread_b(void *arg) {
    char buf[256];
   	printf("线程B启动\n"); 
    
    // 关闭不需要的端
 //   close(pipe1[1]);  // 关闭管道1的写端（B只读）
 //   close(pipe2[0]);  // 关闭管道2的读端（B只写）
    
    // 从管道1读取数据（A -> B）
    memset(buf, 0, sizeof(buf));
    read(pipe1[0], buf, sizeof(buf));
    printf("[线程B] 接收: %s\n", buf);
    
    // 向管道2写入数据（B -> A）
    strcpy(buf, "消息来自线程B");
    write(pipe2[1], buf, strlen(buf));
    printf("[线程B] 发送: %s\n", buf);
	sleep(2);    
    
    close(pipe1[0]);
    close(pipe2[1]);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    // 创建两个管道
    pipe(pipe1);
    pipe(pipe2);
    
    pthread_create(&t1, NULL, thread_a, NULL);
    pthread_create(&t2, NULL, thread_b, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    return 0;
}
