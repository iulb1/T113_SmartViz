/**
 * 示例：消息队列优先级演示
 * 演示不同优先级的消息如何被接收
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

#define QUEUE_NAME "/priority_queue"
#define MAX_MSG_SIZE 128
#define MAX_MSG_COUNT 10

typedef struct {
    int msg_id;
    char content[256];
} priority_message_t;

mqd_t g_mq;

// 发送线程：发送不同优先级的消息
void *sender_thread(void *arg) {
    printf("=== 发送线程开始发送消息 ===\n");
    
    // 按顺序发送不同优先级的消息
    // 注意：发送顺序是 0, 10, 5, 100, 0
    priority_message_t msg;
    
    msg.msg_id = 1;
    strcpy(msg.content, "普通消息1");
    mq_send(g_mq, (const char *)&msg, sizeof(priority_message_t), 0);
    printf("[发送] 消息1 (优先级0): %s\n", msg.content);
    
    msg.msg_id = 2;
    strcpy(msg.content, "重要消息");
    mq_send(g_mq, (const char *)&msg, sizeof(priority_message_t), 10);
    printf("[发送] 消息2 (优先级10): %s\n", msg.content);
    
    msg.msg_id = 3;
    strcpy(msg.content, "中等消息");
    mq_send(g_mq, (const char *)&msg, sizeof(priority_message_t), 5);
    printf("[发送] 消息3 (优先级5): %s\n", msg.content);
    
    msg.msg_id = 4;
    strcpy(msg.content, "紧急消息");
    mq_send(g_mq, (const char *)&msg, sizeof(priority_message_t), 100);
    printf("[发送] 消息4 (优先级100): %s\n", msg.content);
    
    msg.msg_id = 5;
    strcpy(msg.content, "普通消息2");
    mq_send(g_mq, (const char *)&msg, sizeof(priority_message_t), 0);
    printf("[发送] 消息5 (优先级0): %s\n", msg.content);
    
    printf("=== 发送完成 ===\n\n");
    return NULL;
}

// 接收线程：按优先级顺序接收消息
void *receiver_thread(void *arg) {
    printf("=== 接收线程开始接收消息 ===\n");
    printf("注意：接收顺序应该按优先级从高到低，同优先级按FIFO\n\n");
    
    priority_message_t msg;
    unsigned int priority;
    int count = 0;
    
    while (count < 5) {
        ssize_t ret = mq_receive(g_mq, (char *)&msg, sizeof(priority_message_t), &priority);
        if (ret == -1) {
            if (errno == EAGAIN) {
                usleep(100000);
                continue;
            }
            perror("mq_receive失败");
            break;
        }
        
        printf("[接收] 消息%d (优先级%u): %s\n", msg.msg_id, priority, msg.content);
        count++;
        usleep(100000);
    }
    
    printf("\n=== 接收完成 ===\n");
    printf("预期接收顺序：消息4(100) → 消息2(10) → 消息3(5) → 消息1(0) → 消息5(0)\n");
    return NULL;
}

int main() {
    pthread_t sender, receiver;
    
    // 设置消息队列属性
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG_COUNT;
    attr.mq_msgsize = sizeof(priority_message_t);
    attr.mq_curmsgs = 0;
    
    // 创建消息队列
    g_mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (g_mq == (mqd_t)-1) {
        perror("mq_open失败");
        return 1;
    }
    
    printf("=== 消息队列优先级演示 ===\n\n");
    
    // 先创建发送线程，发送所有消息
    pthread_create(&sender, NULL, sender_thread, NULL);
    pthread_join(sender, NULL);
    
    // 等待一下，确保所有消息都已发送
    sleep(1);
    
    // 创建接收线程，按优先级顺序接收
    pthread_create(&receiver, NULL, receiver_thread, NULL);
    pthread_join(receiver, NULL);
    
    // 清理
    mq_close(g_mq);
    mq_unlink(QUEUE_NAME);
    
    printf("\n演示完成！\n");
    return 0;
}
