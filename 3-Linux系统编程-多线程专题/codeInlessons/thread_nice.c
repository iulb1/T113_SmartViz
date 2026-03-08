/**
 * 示例2f：线程nice值演示
 * 演示如何使用nice值调整线程优先级（SCHED_OTHER调度策略）
 * 
 * 编译：gcc -pthread -o 02f_thread_nice 02f_thread_nice.c
 * 运行：./02f_thread_nice
 * 
 * 注意：提高nice值（降低优先级）普通用户即可，降低nice值（提高优先级）需要root权限
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <errno.h>
#include <time.h>

// 线程函数：默认nice值线程
void* default_nice_thread(void *params)
{
    int thread_id = *(int *)params;
    long int i = 0;
    int local_count = 0;
    volatile int dummy = 0;  // 防止编译器优化
    struct timespec start, end;
    double elapsed;
    
    // 获取当前nice值（nice(0)返回当前值，不改变它）
    int nice_val = nice(0);
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("线程%d (nice值=%d, 默认优先级) 开始执行\n", thread_id, nice_val);
    
    // 执行大量循环，观察nice值效果
    // 增加循环次数，让效果更明显
    for(i = 0; i < 500000000; ++i){
        dummy += i;  // 防止编译器优化
        local_count++;
        if(i % 10000000 == 0){
            printf("线程%d (nice值=%d): 执行中... 循环次数=%ld\n", thread_id, nice_val, i);
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("线程%d (nice值=%d) 完成，累计计数=%d，耗时=%.2f毫秒\n", thread_id, nice_val, local_count, elapsed);
    return NULL;
}

// 线程函数：高nice值线程（低优先级）
void* high_nice_thread(void *params)
{
    int thread_id = *(int *)params;
    long int i = 0;
    int local_count = 0;
    volatile int dummy = 0;  // 防止编译器优化
    struct timespec start, end;
    double elapsed;
    
    // 设置nice值为10（降低优先级，普通用户即可）
    int old_nice = nice(10);  // nice(10)将nice值增加10
    int new_nice = nice(0);   // 获取新的nice值
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("线程%d (nice值=%d->%d, 低优先级) 开始执行\n", thread_id, old_nice, new_nice);
    
    // 执行大量循环，观察nice值效果
    // 增加循环次数，让效果更明显
    for(i = 0; i < 500000000; ++i){
        dummy += i;  // 防止编译器优化
        local_count++;
        if(i % 10000000 == 0){
            printf("线程%d (nice值=%d): 执行中... 循环次数=%ld\n", thread_id, new_nice, i);
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("线程%d (nice值=%d) 完成，累计计数=%d，耗时=%.2f毫秒\n", thread_id, new_nice, local_count, elapsed);
    return NULL;
}

// 线程函数：低nice值线程（高优先级，需要root权限）
void* low_nice_thread(void *params)
{
    int thread_id = *(int *)params;
    long int i = 0;
    int local_count = 0;
    volatile int dummy = 0;  // 防止编译器优化
    struct timespec start, end;
    double elapsed;
    
    // 尝试设置nice值为-10（提高优先级，需要root权限）
    int old_nice = nice(0);
    errno = 0;
    int ret = nice(-10);  // 尝试降低nice值（提高优先级）
    int new_nice = nice(0);
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    if(errno != 0 || ret == -1){
        printf("线程%d (nice值=%d->%d, 无法降低nice值，需要root权限) 开始执行\n", thread_id, old_nice, new_nice);
        new_nice = old_nice;  // 使用原来的nice值
    } else {
        printf("线程%d (nice值=%d->%d, 高优先级) 开始执行\n", thread_id, old_nice, new_nice);
    }
    
    // 执行大量循环，观察nice值效果
    // 增加循环次数，让效果更明显
    for(i = 0; i < 500000000; ++i){
        dummy += i;  // 防止编译器优化
        local_count++;
        if(i % 10000000 == 0){
            printf("线程%d (nice值=%d): 执行中... 循环次数=%ld\n", thread_id, new_nice, i);
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("线程%d (nice值=%d) 完成，累计计数=%d，耗时=%.2f毫秒\n", thread_id, new_nice, local_count, elapsed);
    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;
    int thread_id1 = 1;
    int thread_id2 = 2;
    int thread_id3 = 3;
    int ret;
    int current_nice;
    
    printf("=== 线程nice值演示 ===\n\n");
    printf("说明：\n");
    printf("1. nice值范围：-20到19（数字越小优先级越高）\n");
    printf("2. 默认nice值：0\n");
    printf("3. 提高nice值（降低优先级）普通用户即可\n");
    printf("4. 降低nice值（提高优先级）需要root权限\n");
    printf("5. nice值用于SCHED_OTHER调度策略（默认策略）\n\n");
    
    // 获取当前进程的nice值
    current_nice = nice(0);
    printf("当前进程nice值: %d\n\n", current_nice);
    
    // ========== 先创建所有线程，让它们并发执行 ==========
    printf("--- 创建所有线程，让它们并发执行 ---\n");
    
    // 创建线程1：默认nice值（0）
    ret = pthread_create(&t1, NULL, default_nice_thread, &thread_id1);
    if(ret != 0){
        fprintf(stderr, "创建线程1失败: %s\n", strerror(ret));
        return 1;
    }
    printf("线程1创建成功（默认nice值=0）\n");
    
    // 创建线程2：高nice值（低优先级）
    ret = pthread_create(&t2, NULL, high_nice_thread, &thread_id2);
    if(ret != 0){
        fprintf(stderr, "创建线程2失败: %s\n", strerror(ret));
        pthread_join(t1, NULL);
        return 1;
    }
    printf("线程2创建成功（将设置nice值=10，低优先级）\n");
    
    // 创建线程3：低nice值（高优先级，需要root权限）
    ret = pthread_create(&t3, NULL, low_nice_thread, &thread_id3);
    if(ret != 0){
        fprintf(stderr, "创建线程3失败: %s\n", strerror(ret));
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        return 1;
    }
    printf("线程3创建成功（将尝试设置nice值=-10，高优先级，需要root权限）\n");
    
    printf("\n--- 所有线程已创建，开始并发执行 ---\n");
    printf("观察：低nice值（高优先级）的线程应该先完成\n");
    printf("注意：nice值的影响在CPU密集型任务中更明显\n");
    printf("注意：如果线程3的nice值设置失败（显示0->0），说明需要root权限\n");
    printf("提示：在多核CPU上，nice值的效果可能不够明显，因为线程可能在不同核心上并行执行\n");
    printf("      要看到更明显的效果，可以限制CPU核心数或增加任务负载\n\n");
    
    // 等待所有线程完成
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf("\n=== 所有线程执行完成 ===\n");
    printf("\n说明：\n");
    printf("1. 通过耗时对比可以观察nice值的效果\n");
    printf("2. nice值越小（如-10），优先级越高，应该耗时更短\n");
    printf("3. nice值越大（如10），优先级越低，应该耗时更长\n");
    printf("\nnice值效果说明：\n");
    printf("- nice值越小，优先级越高，获得CPU时间越多\n");
    printf("- nice值的影响在CPU密集型任务中更明显\n");
    printf("- 在I/O密集型任务中，nice值的影响较小\n");
    printf("- 普通用户只能提高nice值（降低优先级）\n");
    printf("- 降低nice值（提高优先级）需要root权限\n");
    printf("\n注意：在多核CPU上，nice值的效果可能不够明显，因为：\n");
    printf("1. 线程可能在不同CPU核心上并行执行\n");
    printf("2. nice值在SCHED_OTHER策略下的影响不如实时优先级（SCHED_FIFO/SCHED_RR）明显\n");
    printf("3. 要看到更明显的效果，可以使用taskset限制到单个CPU核心：\n");
    printf("   taskset -c 0 sudo ./02f_thread_nice\n");
    
    return 0;
}

