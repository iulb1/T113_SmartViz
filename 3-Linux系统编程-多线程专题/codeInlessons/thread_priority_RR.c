#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <time.h>

// 共享计数器（用于观察轮转效果）
volatile int g_thread1_count = 0;
volatile int g_thread2_count = 0;
volatile int g_thread3_count = 0;

// 线程函数：相同优先级的线程1
void* rr_thread1(void *params)
{
    int thread_id = *(int *)params;
    int local_count = 0;
    struct timespec start, end;
    double elapsed;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("线程%d (SCHED_RR, 优先级50) 开始执行\n", thread_id);
    
    // 执行大量循环，观察轮转效果
    for(int i = 0; i < 10000000; ++i){
        local_count++;
        g_thread1_count++;
        if(i % 1000000 == 0){
            printf("线程%d (SCHED_RR): 执行中... 循环次数=%d, 累计计数=%d\n", 
                   thread_id, i, g_thread1_count);
            usleep(1000);  // 短暂休眠，让其他线程有机会运行
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("线程%d (SCHED_RR) 完成，累计计数=%d，耗时=%.2f毫秒\n", 
           thread_id, g_thread1_count, elapsed);
    return NULL;
}

// 线程函数：相同优先级的线程2
void* rr_thread2(void *params)
{
    int thread_id = *(int *)params;
    int local_count = 0;
    struct timespec start, end;
    double elapsed;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("线程%d (SCHED_RR, 优先级50) 开始执行\n", thread_id);
    
    for(int i = 0; i < 10000000; ++i){
        local_count++;
        g_thread2_count++;
        if(i % 1000000 == 0){
            printf("线程%d (SCHED_RR): 执行中... 循环次数=%d, 累计计数=%d\n", 
                   thread_id, i, g_thread2_count);
            usleep(1000);
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("线程%d (SCHED_RR) 完成，累计计数=%d，耗时=%.2f毫秒\n", 
           thread_id, g_thread2_count, elapsed);
    return NULL;
}

// 线程函数：相同优先级的线程3
void* rr_thread3(void *params)
{
    int thread_id = *(int *)params;
    int local_count = 0;
    struct timespec start, end;
    double elapsed;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("线程%d (SCHED_RR, 优先级50) 开始执行\n", thread_id);
    
    for(int i = 0; i < 10000000; ++i){
        local_count++;
        g_thread3_count++;
        if(i % 1000000 == 0){
            printf("线程%d (SCHED_RR): 执行中... 循环次数=%d, 累计计数=%d\n", 
                   thread_id, i, g_thread3_count);
            usleep(1000);
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("线程%d (SCHED_RR) 完成，累计计数=%d，耗时=%.2f毫秒\n", 
           thread_id, g_thread3_count, elapsed);
    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;
    pthread_attr_t attr1, attr2, attr3;
    struct sched_param param1, param2, param3;
    int thread_id1 = 1;
    int thread_id2 = 2;
    int thread_id3 = 3;
    int ret;
    
    printf("=== 轮询调度（SCHED_RR）演示 ===\n\n");
    
    // ========== 创建线程1（SCHED_RR，优先级50） ==========
    pthread_attr_init(&attr1);
    pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);  // 必须设置
    pthread_attr_setschedpolicy(&attr1, SCHED_RR);  // 设置调度策略为RR
    param1.sched_priority = 50;  // 设置优先级
    pthread_attr_setschedparam(&attr1, &param1);
    
    pthread_create(&t1, &attr1, rr_thread1, &thread_id1);
    printf("线程1创建成功，调度策略=SCHED_RR，优先级=50\n");
    pthread_attr_destroy(&attr1);
    usleep(10000);
    
    // ========== 创建线程2（SCHED_RR，优先级50） ==========
    pthread_attr_init(&attr2);
    pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr2, SCHED_RR);
    param2.sched_priority = 50;  // 相同优先级
    pthread_attr_setschedparam(&attr2, &param2);
    pthread_create(&t2, &attr2, rr_thread2, &thread_id2);
    printf("线程2创建成功，调度策略=SCHED_RR，优先级=50\n");
    pthread_attr_destroy(&attr2);
    usleep(10000);
    
    // ========== 创建线程3（SCHED_RR，优先级50） ==========
    pthread_attr_init(&attr3);
    pthread_attr_setinheritsched(&attr3, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr3, SCHED_RR);
    param3.sched_priority = 50;  // 相同优先级
    pthread_attr_setschedparam(&attr3, &param3);
    pthread_create(&t3, &attr3, rr_thread3, &thread_id3);
    printf("线程3创建成功，调度策略=SCHED_RR，优先级=50\n");
    pthread_attr_destroy(&attr3);
    
    printf("\n--- 所有线程已创建，开始执行 ---\n");
    printf("观察：在SCHED_RR策略下，相同优先级的线程会按时间片轮转执行\n");
    printf("      三个线程的输出应该会交替出现，而不是一个线程独占CPU\n\n");
    
    // 等待所有线程完成
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf("\n=== 所有线程执行完成 ===\n");
    printf("线程1累计计数: %d\n", g_thread1_count);
    printf("线程2累计计数: %d\n", g_thread2_count);
    printf("线程3累计计数: %d\n", g_thread3_count);
    printf("\n轮询调度（SCHED_RR）效果说明：\n");
    printf("- 在SCHED_RR调度策略下，相同优先级的线程会按时间片轮转执行\n");
    printf("- 每个线程获得固定时间片（通常100ms），时间片用完后切换到下一个线程\n");
    printf("- 这样可以保证相同优先级的线程都能获得公平的CPU时间\n");
    printf("- 与SCHED_FIFO的区别：\n");
    printf("  * SCHED_FIFO：相同优先级按先进先出，一个线程会一直运行直到完成或阻塞\n");
    printf("  * SCHED_RR：相同优先级按时间片轮转，保证公平性\n");
    
    return 0;
}
