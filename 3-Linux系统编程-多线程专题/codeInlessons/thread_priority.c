#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>

// 高优先级线程函数
void* high_priority_thread(void *params)
{
    int thread_id = *(int *)params;
    printf("线程%d (高优先级) 开始执行\n", thread_id);
    
    // 执行大量循环，观察优先级效果
    for(int i = 0; i < 1000000; ++i){
        if(i % 100000 == 0){
            printf("线程%d (高优先级): 执行中... 循环次数=%d\n", thread_id, i);
//			usleep(1000);
        }
    }
    
    printf("线程%d (高优先级) 完成\n", thread_id);
    return NULL;
}

// 中优先级线程函数
void* medium_priority_thread(void *params)
{
    int thread_id = *(int *)params;
    printf("线程%d (中优先级) 开始执行\n", thread_id);
    
    for(int i = 0; i < 1000000; ++i){
        if(i % 100000 == 0){
            printf("线程%d (中优先级): 执行中... 循环次数=%d\n", thread_id, i);
//			usleep(1000);
        }
    }
    
    printf("线程%d (中优先级) 完成\n", thread_id);
    return NULL;
}

// 低优先级线程函数
void* low_priority_thread(void *params)
{
    int thread_id = *(int *)params;
    printf("线程%d (低优先级) 开始执行\n", thread_id);
    
    for(int i = 0; i < 1000000; ++i){
        if(i % 100000 == 0){
            printf("线程%d (低优先级): 执行中... 循环次数=%d\n", thread_id, i);
        }
    }
    
    printf("线程%d (低优先级) 完成\n", thread_id);
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
    
    printf("=== 线程优先级演示 ===\n\n");
    
    // ========== 创建高优先级线程（优先级50） ==========
    pthread_attr_init(&attr1);
    pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);  // 必须设置
    pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);  // 设置调度策略
    param1.sched_priority = 50;  // 设置优先级
    pthread_attr_setschedparam(&attr1, &param1);
    
    pthread_create(&t1, &attr1, high_priority_thread, &thread_id1);
    printf("线程1创建成功，优先级=50 (高优先级)\n");
    pthread_attr_destroy(&attr1);
    usleep(10000);
    
    // ========== 创建中优先级线程（优先级30） ==========
    pthread_attr_init(&attr2);
    pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
    param2.sched_priority = 30;
    pthread_attr_setschedparam(&attr2, &param2);
    pthread_create(&t2, &attr2, medium_priority_thread, &thread_id2);
    printf("线程2创建成功，优先级=30 (中优先级)\n");
    pthread_attr_destroy(&attr2);
    usleep(10000);
    
    // ========== 创建低优先级线程（优先级10） ==========
    pthread_attr_init(&attr3);
    pthread_attr_setinheritsched(&attr3, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr3, SCHED_FIFO);
    param3.sched_priority = 10;
    pthread_attr_setschedparam(&attr3, &param3);
    pthread_create(&t3, &attr3, low_priority_thread, &thread_id3);
    printf("线程3创建成功，优先级=10 (低优先级)\n");
    pthread_attr_destroy(&attr3);
    
    printf("\n--- 所有线程已创建，开始执行 ---\n");
    printf("观察：高优先级线程应该先完成，低优先级线程最后完成\n\n");
    
    // 等待所有线程完成
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf("\n=== 所有线程执行完成 ===\n");
    printf("优先级效果说明：\n");
    printf("- 在SCHED_FIFO调度策略下，高优先级线程会优先获得CPU时间\n");
    printf("- 高优先级线程会先完成大部分工作\n");
    printf("- 只有当高优先级线程阻塞或完成时，低优先级线程才能运行\n");
    
    return 0;
}
