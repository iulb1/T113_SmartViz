/**
 * 示例：互斥锁导致读操作串行化问题
 * 演示多个读线程使用互斥锁时必须等待，无法并发执行
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define READ_THREAD_COUNT 5
#define READ_OPERATIONS 3

// 共享数据
int g_data = 100;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// 获取当前时间（毫秒）
long long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

// 读线程：使用互斥锁
void* read_thread_mutex(void *arg) {
    int thread_id = *(int *)arg;
    long long start_time, end_time, lock_wait_start, lock_wait_time;
    
    for (int i = 0; i < READ_OPERATIONS; i++) {
        lock_wait_start = get_time_ms();
        
        // 加锁（互斥锁）
        pthread_mutex_lock(&g_mutex);
        
        // 记录获得锁后的时间（实际执行开始时间）
        start_time = get_time_ms();
        lock_wait_time = start_time - lock_wait_start;
        
        printf("[%lld ms] 读线程 %d: 开始读取（操作 %d），等待锁耗时 %lld ms\n", 
               start_time, thread_id, i + 1, lock_wait_time);
        
        // 模拟读取操作（耗时100ms）
        usleep(100000);
        int value = g_data;
        
        end_time = get_time_ms();
        printf("[%lld ms] 读线程 %d: 读取完成，值 = %d，执行耗时 %lld ms\n", 
               end_time, thread_id, value, end_time - start_time);
        
        // 解锁
        pthread_mutex_unlock(&g_mutex);
        
        // 两次读取之间稍作休息
        usleep(50000);
    }
    
    return NULL;
}

int main() {
    pthread_t threads[READ_THREAD_COUNT];
    int thread_ids[READ_THREAD_COUNT];
    long long total_start, total_end;
    
    printf("=== 互斥锁导致读操作串行化问题演示 ===\n");
    printf("创建 %d 个读线程，每个线程执行 %d 次读取操作\n", 
           READ_THREAD_COUNT, READ_OPERATIONS);
    printf("每次读取操作耗时 100ms\n\n");
    
    total_start = get_time_ms();
    
    // 创建多个读线程
    for (int i = 0; i < READ_THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, read_thread_mutex, &thread_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < READ_THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    total_end = get_time_ms();
    
    printf("\n=== 性能分析 ===\n");
    printf("总耗时: %lld ms\n", total_end - total_start);
    printf("理论最小耗时（如果完全并发）: %d ms\n", 
           READ_OPERATIONS * 100);  // 3次操作 * 100ms
    printf("实际耗时（串行执行）: 约 %lld ms\n", total_end - total_start);
    printf("\n问题：多个读线程必须串行执行，无法并发，性能下降！\n");
    printf("原因：互斥锁同一时刻只允许一个线程访问，即使是只读操作\n");
    
    return 0;
}
