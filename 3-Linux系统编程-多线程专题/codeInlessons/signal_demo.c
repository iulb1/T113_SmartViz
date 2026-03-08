/**
 * 示例15：优雅退出机制
 * 演示使用信号和标志位实现线程的优雅退出
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#define NUM_WORKER_THREADS 3

// 全局退出标志
volatile int g_running = 1;

// 信号处理函数
void signal_handler(int sig) {
    printf("\n收到信号 %d，准备退出...\n", sig);
    if(sig == 10){
        printf("\n收到信号 %d，准备退出...\n", sig);
        printf("用户1信号，不退出\n");
        return;
    }
    g_running = 0;
}

// 工作线程
void *worker_thread(void *arg) {
    int thread_id = *(int *)arg;
    int work_count = 0;
    
    printf("工作线程 %d: 启动\n", thread_id);
    
    // 简单循环：检查退出标志并执行工作
    while (g_running) {
        work_count++;
        printf("工作线程 %d: 执行任务 %d\n", thread_id, work_count);
        sleep(1);
    }
   // 实际 应用场景中，这里做一些资源清理动作
    printf("工作线程 %d: 退出（已完成 %d 个任务）\n", thread_id, work_count);
    return NULL;
}

int main() {
    pthread_t threads[NUM_WORKER_THREADS];
    int thread_ids[NUM_WORKER_THREADS];
    
    // 注册信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);
    
    printf("=== 优雅退出机制测试 ===\n");
    printf("按 Ctrl+C 或发送 SIGTERM 信号来触发退出\n");
    printf("创建 %d 个工作线程...\n\n", NUM_WORKER_THREADS);
    
    // 创建工作线程
    for (int i = 0; i < NUM_WORKER_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }
    
    // 主线程等待退出信号
    printf("主线程: 运行中...\n");
    while (g_running) {
        sleep(1);
    }
    
    printf("\n主线程: 收到退出信号，等待所有工作线程退出...\n");
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_WORKER_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("主线程: 所有线程已退出，程序结束\n");
    return 0;
}
