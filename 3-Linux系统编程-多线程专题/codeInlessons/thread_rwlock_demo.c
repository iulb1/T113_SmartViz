/**
 * 示例：读写锁优化读操作性能
 * 演示使用读写锁时，多个读线程可以并发执行，与互斥锁案例对比
 * 配置：5个读线程，每个读3次，每次100ms（与互斥锁案例完全一致）
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
pthread_rwlock_t g_rwlock;

// 获取当前时间（毫秒）
long long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

// 读线程：使用读写锁
void* read_thread_rwlock(void *arg) {
    int thread_id = *(int *)arg;
    long long start_time, end_time;
    
    for (int i = 0; i < READ_OPERATIONS; i++) {
        start_time = get_time_ms();
        
        // 加读锁（读写锁 - 允许多个线程同时读取）
        pthread_rwlock_rdlock(&g_rwlock);
        printf("[%lld ms] 读线程 %d: 开始读取（操作 %d）\n", 
               start_time, thread_id, i + 1);
        
        // 模拟读取操作（耗时100ms）
        usleep(100000);
        int value = g_data;
        
        end_time = get_time_ms();
        printf("[%lld ms] 读线程 %d: 读取完成，值 = %d，耗时 %lld ms\n", 
               end_time, thread_id, value, end_time - start_time);
        
        // 解锁
        pthread_rwlock_unlock(&g_rwlock);
        
        // 两次读取之间稍作休息
        usleep(50000);
    }
    
    return NULL;
}

int main() {
    pthread_t threads[READ_THREAD_COUNT];
    int thread_ids[READ_THREAD_COUNT];
    long long total_start, total_end;
    
    // 初始化读写锁
    pthread_rwlock_init(&g_rwlock, NULL);
    
    printf("=== 读写锁优化读操作性能演示 ===\n");
    printf("创建 %d 个读线程，每个线程执行 %d 次读取操作\n", 
           READ_THREAD_COUNT, READ_OPERATIONS);
    printf("每次读取操作耗时 100ms\n");
    printf("使用读写锁：多个读线程可以并发执行\n\n");
    
    total_start = get_time_ms();
    
    // 创建多个读线程
    for (int i = 0; i < READ_THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, read_thread_rwlock, &thread_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < READ_THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    total_end = get_time_ms();
    
    // 清理
    pthread_rwlock_destroy(&g_rwlock);
    
    printf("\n=== 性能分析 ===\n");
    printf("总耗时: %lld ms\n", total_end - total_start);
    printf("理论最小耗时（如果完全并发）: %d ms\n", 
           READ_OPERATIONS * 100);  // 3次操作 * 100ms
    printf("实际耗时（并发执行）: 约 %lld ms\n", total_end - total_start);
    printf("\n优势：多个读线程可以并发执行，性能大幅提升！\n");
    printf("原因：读写锁的读锁是共享的，允许多个线程同时读取\n");
    
    printf("\n=== 与互斥锁对比 ===\n");
    printf("互斥锁：5个读线程串行执行，总耗时约 2000+ ms\n");
    printf("读写锁：5个读线程并发执行，总耗时约 %lld ms\n", total_end - total_start);
    printf("性能提升：约 %.1f 倍\n", 
           2000.0 / (total_end - total_start > 0 ? (double)(total_end - total_start) : 1));
    
    return 0;
}
