/**
 * 示例16：线程局部存储（TLS）
 * 演示每个线程拥有独立的数据副本
 * 通过封装函数展示TLS可以在多个函数间共享，而局部变量无法做到
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_THREADS 2

// 使用__thread关键字声明线程局部变量
__thread int g_counter = 0;
__thread char g_thread_name[32] = "";

// 封装函数：增加计数器（使用TLS，无需传参）
void increment_counter(void) {
    g_counter++;  // 直接访问TLS变量，无需参数传递
}

// 封装函数：执行工作（展示TLS的跨函数访问能力）
void do_work(int work_id) {
    // 注意：这里没有传递任何计数器参数
    // 但可以访问和修改TLS数据
    increment_counter();
    
    printf("[%s] 执行工作%d -> 计数器=%d\n", g_thread_name, work_id, g_counter);
}

// 工作线程
void *worker_thread(void *arg) {
    int thread_id = *(int *)arg;
    
    // 初始化TLS数据
    g_counter = 0;
    snprintf(g_thread_name, sizeof(g_thread_name), "线程%d", thread_id);
    
    printf("[%s] 启动，TLS数据已初始化\n", g_thread_name);
    
    // 执行工作（调用封装函数，展示TLS的跨函数访问）
    for (int i = 0; i < 3; i++) {
        do_work(i + 1);  // 调用函数，函数内部可以访问TLS
        sleep(1);
    }
    
    // 最终统计
    printf("[%s] 完成，最终计数器=%d\n", g_thread_name, g_counter);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    printf("========================================\n");
    printf("  线程局部存储（TLS）演示\n");
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
    
    printf("\n========================================\n");
    printf("  测试完成！每个线程都有独立的数据副本\n");
    printf("========================================\n");
    return 0;
}
