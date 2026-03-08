/**
 * 示例12a：POSIX消息队列实现
 * 演示使用POSIX消息队列API进行线程间通信
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define QUEUE_NAME "/my_message_queue"
#define MAX_MSG_SIZE 256
#define MAX_MSG_COUNT 11

// 消息结构（POSIX消息队列传递的是字节流，需要自己定义结构）
typedef struct {
    int type;
    char data[MAX_MSG_SIZE - sizeof(int)];  // 预留type的空间
} message_t;

mqd_t g_mq;

// 发送线程
void *sender_thread(void *arg) {
    int sender_id = *(int *)arg;
    
    printf("发送线程 %d 开始\n", sender_id);
    
    for (int i = 0; i < 5; i++) {
        //构建一条消息
        message_t msg;
        msg.type = sender_id;
        snprintf(msg.data, sizeof(msg.data), "消息来自发送线程%d-%d", sender_id, i + 1);
        
        // 使用mq_send发送消息
        // 参数：队列描述符、消息指针、消息长度、优先级
        int ret = mq_send(g_mq, (const char *)&msg, sizeof(message_t), 0);
        if (ret == -1) {
            perror("mq_send失败");
            break;
        }
        
        printf("发送线程 %d: 发送消息 %d\n", sender_id, i + 1);
        usleep(200000);
    }
    
    printf("发送线程 %d 完成\n", sender_id);
    return NULL;
}

// 接收线程
void *receiver_thread(void *arg) {
    printf("接收线程开始\n");
    
    int messages_received = 0;
    while (messages_received < 10) {
        message_t msg;
        unsigned int priority;
        // 使用mq_receive接收消息
        // 参数：队列描述符、接收缓冲区、缓冲区大小、优先级输出
        ssize_t ret = mq_receive(g_mq, (char *)&msg, sizeof(message_t), &priority);
        if (ret == -1) {
            if (errno == EAGAIN) {
                // 非阻塞模式下队列为空
                usleep(100000);
                continue;
            }
            perror("mq_receive失败");
            break;
        }  
        printf("接收线程: 收到类型%d的消息(优先级:%u): %s\n", 
               msg.type, priority, msg.data);
        messages_received++;
        
        usleep(100000);
    }
    
    printf("接收线程完成，共接收 %d 条消息\n", messages_received);
    return NULL;
}

int main() {
    pthread_t sender1, sender2, receiver;
    int sender_id1 = 1, sender_id2 = 2;
    
    // 设置消息队列属性
    struct mq_attr attr;
    attr.mq_flags = 0;              // 0表示阻塞模式，O_NONBLOCK表示非阻塞
    attr.mq_maxmsg = MAX_MSG_COUNT; // 最大消息数量
    attr.mq_msgsize = sizeof(message_t); // 每条消息的最大大小
    attr.mq_curmsgs = 0;            // 当前消息数量（只读）
    
    // 创建或打开消息队列
    // O_CREAT: 如果不存在则创建
    // O_RDWR: 读写模式
    // 0644: 权限位
    g_mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (g_mq == (mqd_t)-1) {
        perror("mq_open失败");
        return 1;
    }
    printf("=== POSIX消息队列测试 ===\n\n");
    // 先创建发送线程，不创建接收线程，发送线程把数据放入队列。
    pthread_create(&sender1, NULL, sender_thread, &sender_id1);
    pthread_create(&sender2, NULL, sender_thread, &sender_id2);
    
    // 等待所有线程完成
    pthread_join(sender1, NULL);
    pthread_join(sender2, NULL);
    sleep(5);
    // 创建接收线程
    pthread_create(&receiver, NULL, receiver_thread, NULL);
    
    usleep(100000);
    
    pthread_join(receiver, NULL);
    
    // 关闭消息队列
    mq_close(g_mq);
    
    // 删除消息队列（可选，如果不再使用）
    // mq_unlink(QUEUE_NAME);
    
    printf("\n测试完成！POSIX消息队列实现了线程间的异步通信\n");
    return 0;
}
