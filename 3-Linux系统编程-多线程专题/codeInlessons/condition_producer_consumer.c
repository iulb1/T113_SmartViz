/**
 * 示例5：生产者-消费者模型（使用条件变量）
 * 演示如何使用条件变量实现高效的生产者-消费者模式
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define BUFFER_SIZE 10
#define PRODUCER_ITEMS 50
#define CONSUMER_ITEMS 50

// 缓冲区结构
typedef struct {
    int data[BUFFER_SIZE];
    size_t write_pos;
    size_t read_pos;
    size_t count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;   // 缓冲区不满的条件
    pthread_cond_t not_empty;   // 缓冲区不空的条件
} buffer_t;

buffer_t g_buffer;

// 初始化缓冲区
void buffer_init(buffer_t *buf) {
    memset(buf->data, 0, sizeof(buf->data));
    buf->write_pos = 0;
    buf->read_pos = 0;
    buf->count = 0;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->not_full, NULL);
    pthread_cond_init(&buf->not_empty, NULL);
}

// 销毁缓冲区
void buffer_destroy(buffer_t *buf) {
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->not_full);
    pthread_cond_destroy(&buf->not_empty);
}

// 生产者：写入数据
void buffer_put(buffer_t *buf, int item) {
    pthread_mutex_lock(&buf->mutex);
    
    // 等待缓冲区不满
    while (buf->count >= BUFFER_SIZE) {
        printf("生产者: 缓冲区满，等待...\n");
        pthread_cond_wait(&buf->not_full, &buf->mutex);
    }
    
    // 写入数据
    buf->data[buf->write_pos] = item;
    buf->write_pos = (buf->write_pos + 1) % BUFFER_SIZE;
    buf->count++;
    
    printf("生产者: 写入 %d，缓冲区使用: %zu/%d\n", item, buf->count, BUFFER_SIZE);
    
    // 通知消费者
    pthread_cond_signal(&buf->not_empty);
    pthread_mutex_unlock(&buf->mutex);
}

// 消费者：读取数据
int buffer_get(buffer_t *buf) {
    pthread_mutex_lock(&buf->mutex);
    
    // 等待缓冲区不空
    while (buf->count == 0) {
        printf("消费者: 缓冲区空，等待...\n");
        pthread_cond_wait(&buf->not_empty, &buf->mutex);
    }
    
    // 读取数据
    int item = buf->data[buf->read_pos];
    buf->read_pos = (buf->read_pos + 1) % BUFFER_SIZE;
    buf->count--;
    
    printf("消费者: 读取 %d，缓冲区使用: %zu/%d\n", item, buf->count, BUFFER_SIZE);
    
    // 通知生产者
    pthread_cond_signal(&buf->not_full);
    pthread_mutex_unlock(&buf->mutex);
    
    return item;
}

// 生产者线程
void *producer_thread(void *arg) {
    int producer_id = *(int *)arg;
    
    printf("生产者 %d 开始工作\n", producer_id);
    
    for (int i = 0; i < PRODUCER_ITEMS; i++) {
        int item = producer_id * 1000 + i;
        buffer_put(&g_buffer, item);
        usleep(100000);  // 模拟生产时间
    }
    
    printf("生产者 %d 完成\n", producer_id);
    return NULL;
}

// 消费者线程
void *consumer_thread(void *arg) {
    int consumer_id = *(int *)arg;
    int items_consumed = 0;
    
    printf("消费者 %d 开始工作\n", consumer_id);
    
    while (items_consumed < CONSUMER_ITEMS) {
        int item = buffer_get(&g_buffer);
        items_consumed++;
        
        // 处理数据
        printf("消费者 %d 处理数据: %d\n", consumer_id, item);
        
        usleep(150000);  // 模拟消费时间（比生产慢）
    }
    
    printf("消费者 %d 完成，共消费 %d 项\n", consumer_id, items_consumed);
    return NULL;
}

int main() {
    pthread_t producer, consumer;
    int producer_id = 1, consumer_id = 1;
    
    // 初始化缓冲区
    buffer_init(&g_buffer);
    
    printf("=== 生产者-消费者模型测试 ===\n");
    printf("缓冲区大小: %d\n", BUFFER_SIZE);
    printf("生产者将生产 %d 项\n", PRODUCER_ITEMS);
    printf("消费者将消费 %d 项\n\n", CONSUMER_ITEMS);
    
    // 创建生产者线程
    pthread_create(&producer, NULL, producer_thread, &producer_id);
    
    // 创建消费者线程
    pthread_create(&consumer, NULL, consumer_thread, &consumer_id);
    
    // 等待线程完成
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    // 清理
    buffer_destroy(&g_buffer);
    
    printf("\n测试完成！条件变量实现了高效的生产者-消费者同步\n");
    return 0;
}

