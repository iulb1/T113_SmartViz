/**
 * 示例16b：线程局部存储（TLS）- 使用pthread_key接口
 * 演示每个线程拥有独立的数据副本
 * 通过封装函数展示TLS可以在多个函数间共享，而局部变量无法做到
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_THREADS 2

// 使用pthread_key创建线程键
pthread_key_t g_thread_key;

// 线程特定数据结构
typedef struct {
    int counter;
    char thread_name[32];
} thread_data_t;

// 键的析构函数（线程退出时自动调用）
void destructor(void *data) {
    thread_data_t *td = (thread_data_t *)data;
    printf("[清理] %s的数据被自动清理\n", td->thread_name);
    free(td);
}

// 封装函数：增加计数器（使用TLS，无需传参）
void increment_counter(void) {
    // 通过pthread_getspecific获取当前线程的TLS数据
    thread_data_t *td = (thread_data_t *)pthread_getspecific(g_thread_key);
    if (td) {
        td->counter++;  // 直接访问TLS数据，无需参数传递
    }
}

// 封装函数：执行工作（展示TLS的跨函数访问能力）
void do_work(int work_id) {
    // 注意：这里没有传递任何计数器参数
    // 但可以访问和修改TLS数据
    increment_counter();
    
    // 通过pthread_getspecific获取当前线程的TLS数据
    thread_data_t *td = (thread_data_t *)pthread_getspecific(g_thread_key);
    if (td) {
        printf("[%s] 执行工作%d -> 计数器=%d\n", td->thread_name, work_id, td->counter);
    }
}

// 工作线程
void *worker_thread(void *arg) {
    int thread_id = *(int *)arg;
    
    // 初始化TLS数据：为当前线程分配并设置数据
    thread_data_t *td = malloc(sizeof(thread_data_t));
    td->counter = 0;
    snprintf(td->thread_name, sizeof(td->thread_name), "线程%d", thread_id);
    pthread_setspecific(g_thread_key, td);
    
    printf("[%s] 启动，TLS数据已初始化\n", td->thread_name);
    
    // 执行工作（调用封装函数，展示TLS的跨函数访问）
    for (int i = 0; i < 3; i++) {
        do_work(i + 1);  // 调用函数，函数内部可以访问TLS
        sleep(1);
    }
    
    // 最终统计
    td = (thread_data_t *)pthread_getspecific(g_thread_key);
    if (td) {
        printf("[%s] 完成，最终计数器=%d\n", td->thread_name, td->counter);
    }
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    // 创建线程键
    pthread_key_create(&g_thread_key, destructor);
    
    printf("========================================\n");
    printf("  线程局部存储（TLS）演示 - pthread_key\n");
    printf("========================================\n");
    printf("说明：每个线程拥有独立的TLS数据副本\n");
    printf("     TLS数据可以在函数间共享，无需传参\n");
    printf("========================================\n\n");
    
    // 创建线程
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // 清理线程键
    pthread_key_delete(g_thread_key);
    
    printf("\n========================================\n");
    printf("  测试完成！每个线程都有独立的数据副本\n");
    printf("========================================\n");
    return 0;
}
