#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// 静态初始化的互斥锁（全局变量）
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// 共享资源
int g_counter = 0;

// 线程函数：多个线程同时访问共享资源
void* thread_func(void *arg)
{
    int thread_id = *(int *)arg;
    int i;
    int local_sum = 0;
    
    printf("线程%d开始工作\n", thread_id);
    // 每个线程执行10次累加操作（减少次数，每次操作都打印）
    for(i = 0; i < 10; ++i){
        // 尝试加锁前打印等待信息
        printf("线程%d: 等待获得锁...\n", thread_id);
        
        // 如果锁被占用，会在这里阻塞等待
        pthread_mutex_lock(&g_mutex);  
        
        // 获得锁后立即打印
        printf("线程%d: ✓ 获得锁，进入临界区\n", thread_id);
        
        // 临界区：访问共享资源
        g_counter++;
        local_sum = g_counter;  // 记录当前值
        printf("线程%d: 在临界区内操作，counter=%d\n", thread_id, g_counter);
        
        // 在临界区内停留一段时间，释放CPU时间片，让另一个线程有机会等待，让我们看到其他线程等待相关的调试信息
        sleep(1);  // 睡眠1秒，让锁持有时间更长
        
        // 释放锁前打印
        printf("线程%d: 准备释放锁，退出临界区\n", thread_id);
        
        // 解锁
        pthread_mutex_unlock(&g_mutex);
        
        printf("线程%d: 已释放锁\n", thread_id);
        
        // 模拟其他工作（不在临界区内）
        usleep(500000);  // 500毫秒，让另一个线程有机会先获得锁
    }
    printf("线程%d完成，最后访问的counter值=%d\n", thread_id, local_sum);
    return NULL;
}
int main()
{
    pthread_t t1, t2;
    int thread_id1 = 1;
    int thread_id2 = 2;
    int ret;
    
    printf("=== 静态初始化互斥锁示例 ===\n\n");
    printf("说明：\n");
    printf("1. 使用PTHREAD_MUTEX_INITIALIZER静态初始化互斥锁\n");
    printf("2. 两个线程同时访问共享计数器g_counter\n");
    printf("3. 使用互斥锁保护临界区，确保线程安全\n");
    printf("4. 静态初始化的锁不需要手动销毁\n\n");
    
    // 创建两个线程
    ret = pthread_create(&t1, NULL, thread_func, &thread_id1);
    if(ret != 0){
        fprintf(stderr, "创建线程1失败: %s\n", strerror(ret));
        return 1;
    }
    
    ret = pthread_create(&t2, NULL, thread_func, &thread_id2);
    if(ret != 0){
        fprintf(stderr, "创建线程2失败: %s\n", strerror(ret));
        pthread_join(t1, NULL);
        return 1;
    }
    
    printf("所有线程已创建，开始并发执行...\n\n");
    
    // 等待所有线程完成
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("\n=== 所有线程执行完成 ===\n");
    printf("最终counter值: %d (应该是20)\n", g_counter);
    printf("如果没有互斥锁保护，counter值会小于20\n");
    printf("\n观察要点：\n");
    printf("- 当一个线程获得锁时，另一个线程会在'等待获得锁'处阻塞\n");
    printf("- 只有当持有锁的线程释放锁后，等待的线程才能获得锁\n");
    printf("- 这展示了互斥锁的排队机制：同一时刻只有一个线程能进入临界区\n");
    printf("\n静态初始化互斥锁的特点：\n");
    printf("- 代码简洁，一行完成初始化\n");
    printf("- 线程安全，由编译器保证\n");
    printf("- 适合全局或静态变量\n");
    printf("- 不需要手动销毁（程序退出时自动清理）\n");
    
    // 注意：静态初始化的锁可以调用destroy，但不是必须的
    // pthread_mutex_destroy(&g_mutex);
    
    return 0;
}
