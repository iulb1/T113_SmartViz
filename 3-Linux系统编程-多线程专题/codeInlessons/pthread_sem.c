/**
 * 示例：信号量控制并发访问数量
 * 演示使用信号量实现简单的线程池
 * 
 * 场景：有10个任务需要执行，但最多只能同时执行3个任务
 * 使用信号量控制并发数量，避免资源过载
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define MAX_CONCURRENT_TASKS 3  // 最大并发任务数
#define TOTAL_TASKS 10           // 总任务数

// 任务结构
typedef struct {
    int task_id;
    int duration;  // 任务执行时间（秒）
} task_t;

// 信号量：控制并发任务数
// 初始值为MAX_CONCURRENT_TASKS，表示最多允许3个任务同时执行
sem_t g_semaphore;

// 任务执行函数
void *task_thread(void *arg) {
    task_t *task = (task_t *)arg;
    
    // 获取信号量（P操作）
    // 如果信号量值 > 0，立即获得许可，信号量值减1
    // 如果信号量值 = 0，阻塞等待，直到有任务完成释放信号量
    printf("任务 %d: 等待执行许可...\n", task->task_id);
    sem_wait(&g_semaphore);
    
    printf("任务 %d: 开始执行（将运行 %d 秒）\n", task->task_id, task->duration);
    
    // 模拟任务执行（实际应用中可能是网络请求、文件处理等）
    sleep(task->duration);
    
    printf("任务 %d: 执行完成\n", task->task_id);
    
    // 释放信号量（V操作）
    // 信号量值加1，唤醒等待的线程
    sem_post(&g_semaphore);
    
    return NULL;
}

int main() {
    pthread_t threads[TOTAL_TASKS];
    task_t tasks[TOTAL_TASKS];
    
    // 初始化信号量：最多允许MAX_CONCURRENT_TASKS个并发任务
    // 参数说明：
    //   &g_semaphore: 信号量对象
    //   0: pshared=0表示线程间共享（如果是进程间共享，设为非0）
    //   MAX_CONCURRENT_TASKS: 初始值，表示有3个可用"许可证"
    sem_init(&g_semaphore, 0, MAX_CONCURRENT_TASKS);
    
    printf("=== 信号量控制并发任务数 ===\n");
    printf("最大并发数: %d\n", MAX_CONCURRENT_TASKS);
    printf("总任务数: %d\n\n", TOTAL_TASKS);
    
    // 创建所有任务线程
    for (int i = 0; i < TOTAL_TASKS; i++) {
        tasks[i].task_id = i + 1;
        tasks[i].duration = 2;  // 每个任务执行2秒
        
        pthread_create(&threads[i], NULL, task_thread, &tasks[i]);
        usleep(100000);  // 稍微错开创建时间，便于观察
    }
    
    printf("\n所有任务已提交，等待完成...\n\n");
    
    // 等待所有线程完成
    for (int i = 0; i < TOTAL_TASKS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // 清理信号量
    sem_destroy(&g_semaphore);
    
    printf("\n所有任务完成！信号量确保了最多 %d 个任务同时执行\n", 
           MAX_CONCURRENT_TASKS);
    return 0;
}
