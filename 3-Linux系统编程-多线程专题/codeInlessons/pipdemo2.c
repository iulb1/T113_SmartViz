/**
 * 示例14：使用管道在线程间传递控制命令
 * 演示匿名管道在线程间通信的应用
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_CMD_SIZE 256

// 命令类型
typedef enum {
    CMD_START,
    CMD_STOP,
    CMD_PAUSE,
    CMD_RESUME,
    CMD_EXIT
} command_t;

int g_pipe_fd[2];  // 管道文件描述符

// 发送命令
void send_command(command_t cmd) {
    char buffer[MAX_CMD_SIZE];
    snprintf(buffer, sizeof(buffer), "%d", cmd);
    write(g_pipe_fd[1], buffer, strlen(buffer) + 1);
    printf("发送命令: %d\n", cmd);
}

// 接收命令
command_t receive_command(void) {
    char buffer[MAX_CMD_SIZE];
    ssize_t n = read(g_pipe_fd[0], buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        return (command_t)atoi(buffer);
    }
    return CMD_EXIT;
}

// 工作线程（接收命令并执行）
void *worker_thread(void *arg) {
    printf("工作线程: 启动，等待命令...\n");
    
    while (1) {
        command_t cmd = receive_command();
        
        switch (cmd) {
            case CMD_START:
                printf("工作线程: 执行 START 命令\n");
                break;
            case CMD_STOP:
                printf("工作线程: 执行 STOP 命令\n");
                break;
            case CMD_PAUSE:
                printf("工作线程: 执行 PAUSE 命令\n");
                break;
            case CMD_RESUME:
                printf("工作线程: 执行 RESUME 命令\n");
                break;
            case CMD_EXIT:
                printf("工作线程: 执行 EXIT 命令，退出\n");
                return NULL;
            default:
                printf("工作线程: 未知命令\n");
                break;
        }
        
        usleep(100000);
    }
    
    return NULL;
}

// 控制线程（发送命令）
void *control_thread(void *arg) {
    printf("控制线程: 开始发送命令\n");
    
    sleep(1);
    send_command(CMD_START);
    
    sleep(1);
    send_command(CMD_PAUSE);
    
    sleep(1);
    send_command(CMD_RESUME);
    
    sleep(1);
    send_command(CMD_STOP);
    
    sleep(1);
    send_command(CMD_EXIT);
    
    printf("控制线程: 完成\n");
    return NULL;
}

int main() {
    pthread_t worker, control;
    
    // 创建管道
    if (pipe(g_pipe_fd) < 0) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }
    
    printf("=== 管道命令传递测试 ===\n\n");
    
    // 创建工作线程
    pthread_create(&worker, NULL, worker_thread, NULL);
    
    // 创建控制线程
    pthread_create(&control, NULL, control_thread, NULL);
    
    // 等待线程完成
    pthread_join(control, NULL);
    pthread_join(worker, NULL);
    
    // 关闭管道
    close(g_pipe_fd[0]);
    close(g_pipe_fd[1]);
    
    printf("\n测试完成！管道实现了线程间的命令传递\n");
    return 0;
}
