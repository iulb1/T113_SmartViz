#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BATCH_SIZE 100          // 批量刷盘阈值：累计100条日志触发刷盘
#define LOG_BUF_SIZE (512)  // 日志缓冲区大小（1MB），用宏定义增强可读性
#define LOG_FILE_PATH "app.log" // 日志文件路径（可根据需求修改）

// 全局变量：需注意多线程场景下的线程安全（后续会补充防护）
char log_buf[LOG_BUF_SIZE] = {0};  // 日志缓冲区，初始化为0避免垃圾数据
int log_count = 0;                  // 已缓存日志条数计数器
int log_fd = -1;                    // 日志文件FD，初始化为-1（标记未初始化）

/**
 * @brief 初始化日志文件（必须在调用write_log前执行）
 * @return 0：初始化成功，-1：初始化失败
 */
int init_log() {
    // 打开日志文件：O_WRONLY（只写）| O_CREAT（不存在则创建）| O_APPEND（追加写入，避免覆盖历史日志）
    // 权限0644：所有者读写，其他用户只读（符合日志文件安全规范）
    log_fd = open(LOG_FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("init_log: open log file failed");  // 打印错误原因（如权限不足、路径不存在）
        return -1;
    }
    printf("日志文件初始化成功，路径：%s\n", LOG_FILE_PATH);
    return 0;
}

/**
 * @brief 写入日志（业务线程可调用）
 * @param log 待写入的日志内容（字符串，无需包含末尾换行符）
 * @note 1. 需先调用init_log初始化日志文件，否则会触发错误
 *       2. 内部实现批量刷盘，减少fsync阻塞次数，提升性能
 *       3. 已补充缓冲区溢出防护，避免内存越界
 */
void write_log(const char *log) {
    // 1. 检查日志文件是否已初始化（避免使用未打开的FD）
    if (log_fd == -1) {
        fprintf(stderr, "write_log error: log file not initialized! Call init_log first.\n");
        return;
    }

    // 2. 计算当前需写入的日志长度（包含末尾换行符"\n"）
    size_t log_len = strlen(log);
    size_t total_needed = strlen(log_buf) + log_len + 1;  // +1 对应"\n"的长度 sizeof(log_buf)
 
    // 3. 若单条日志超过缓冲区大小，直接写入（避免死循环）
    if (log_len + 1 >= LOG_BUF_SIZE) {
        printf("write_log: single log too large (exceed buffer), write directly\n");
        char temp_buf[LOG_BUF_SIZE] = {0};
        snprintf(temp_buf, sizeof(temp_buf), "%s\n", log);  // 截断超长日志，避免越界
        write(log_fd, temp_buf, strlen(temp_buf));
        fsync(log_fd);
        return;
    }
	
    // 4. 缓冲区溢出防护：若剩余空间不足，先触发一次刷盘释放空间
    if (total_needed >= LOG_BUF_SIZE) {
//        printf("write_log: buffer will overflow, trigger flush in advance\n");
        // 写入当前缓冲区所有数据
        ssize_t write_ret = write(log_fd, log_buf, strlen(log_buf));
        if (write_ret == -1) {
            printf("write_log: flush buffer failed!!!\n");
        } else {
            fsync(log_fd);  // 强制刷盘，确保数据持久化
            memset(log_buf, 0, sizeof(log_buf));  // 重置缓冲区
            log_count = 0;                         // 重置计数器
        }
       
    }

    // 5. 将日志写入缓冲区（拼接换行符，让每条日志独占一行）
    snprintf(log_buf + strlen(log_buf),  // 从缓冲区当前末尾开始写入
             LOG_BUF_SIZE - strlen(log_buf),  // 限制写入长度，避免越界
             "%s\n", log);  // 拼接日志内容和换行符

    // 6. 累计日志条数，达到阈值时批量刷盘
    log_count++;
    if (log_count >= BATCH_SIZE) {
        ssize_t write_ret = write(log_fd, log_buf, strlen(log_buf));
    //    printf("write_ret = %ld\n",write_ret);
        if (write_ret == -1) {
            printf("write_log: batch write failed!!!!\n");
        } else {
            fsync(log_fd);  // 批量刷盘：平衡性能与数据安全性
      //      printf("write_log: batch flush success, log count: %d\n", log_count);
            memset(log_buf, 0, sizeof(log_buf));  // 重置缓冲区
            log_count = 0;                         // 重置计数器
        }
    }
}

/**
 * @brief 销毁日志资源（程序退出前调用，确保缓冲区剩余日志刷盘）
 */
void destroy_log() {
    // 刷盘缓冲区中剩余的日志（避免程序退出时丢失未达阈值的日志）
    if (log_fd != -1 && strlen(log_buf) > 0) {
        write(log_fd, log_buf, strlen(log_buf));
        fsync(log_fd);
        printf("destroy_log: flush remaining logs (count: %d)\n", log_count);
        memset(log_buf, 0, sizeof(log_buf));
    }
    // 关闭日志文件FD，释放资源
    if (log_fd != -1) {
        close(log_fd);
        log_fd = -1;
        printf("destroy_log: log file closed\n");
    }
}

// ------------------------------ 测试代码 ------------------------------
int main() {
    // 1. 初始化日志（必须第一步执行）
    if (init_log() != 0) {
        fprintf(stderr, "init log failed, exit\n");
        return EXIT_FAILURE;
    }

    // 2. 模拟业务线程写入日志（150条，触发1次批量刷盘+1次剩余刷盘）
    for (int i = 1; i <= 25000; i++) {
        char log_content[256] = {0};
        snprintf(log_content, sizeof(log_content), "business log: id=%d, msg=normal operation", i);
        write_log(log_content);
    }

    // 3. 程序退出前销毁日志资源（确保剩余50条日志刷盘）
    destroy_log();

    return EXIT_SUCCESS;
}
