#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 共享资源
int data_ready = 0;
int shared_data = 0;

// 互斥锁和条件变量
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 生产者线程：准备数据并通知消费者
void* producer_thread(void *arg) {
    for (int i = 1; i <= 5; i++) {
        sleep(1);  // 模拟数据准备时间
        
        pthread_mutex_lock(&mutex);
        
        // 准备数据
        shared_data = i * 100;
        data_ready = 1;
        printf("生产者: 数据已准备好，data = %d\n", shared_data);
        
        // 通知等待的消费者线程
        pthread_cond_signal(&cond);
        
        pthread_mutex_unlock(&mutex);
    }
    
    printf("生产者: 完成所有数据准备\n");
    return NULL;
}

// 消费者线程：等待数据准备好后处理
void* consumer_thread(void *arg) {
    for (int i = 1; i <= 5; i++) {
        pthread_mutex_lock(&mutex);
        
        // 等待数据准备好（使用while循环防止虚假唤醒）
        while (!data_ready) {
            printf("消费者: 等待数据准备...\n");
            pthread_cond_wait(&cond, &mutex);//当条件不满足的时候，这个API会释放锁，当条件满足/收到通知之后又会自己获取锁，不需要程序员管理
        }
        
        // 数据已准备好，处理数据
        printf("消费者: 收到数据，data = %d，开始处理\n", shared_data);
        data_ready = 0;  // 重置标志
        
        pthread_mutex_unlock(&mutex);
        
        // 模拟数据处理时间
        usleep(500000);
    }
    
    printf("消费者: 完成所有数据处理\n");
    return NULL;
}

int main() {
    pthread_t producer, consumer;
    
    printf("=== 条件变量基本使用示例 ===\n\n");
    
    // 创建生产者线程
    if (pthread_create(&producer, NULL, producer_thread, NULL) != 0) {
        perror("创建生产者线程失败");
        return 1;
    }
    
    // 创建消费者线程
    if (pthread_create(&consumer, NULL, consumer_thread, NULL) != 0) {
        perror("创建消费者线程失败");
        return 1;
    }
    
    // 等待线程完成
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    // 注意：静态初始化的条件变量通常不需要销毁
    // 如果需要，可以调用：
    // pthread_cond_destroy(&cond);
    // pthread_mutex_destroy(&mutex);
    
    printf("\n示例完成！\n");
    return 0;
}
