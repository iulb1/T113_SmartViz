#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // 包含inet_addr函数（IP地址转换）
#include <string.h>
#include <openssl/md5.h>  // 依赖OpenSSL库，编译时需链接：-lcrypto

// 与服务端保持一致的参数定义
#define BUF_SIZE 65536     // 数据块大小（64KB）：必须与服务端相同
#define FILENAME_MAX 256   // 文件名最大长度：必须与服务端相同
#define SERVER_IP "127.0.0.1"  // 服务端IP（可根据实际情况修改，如"192.168.1.100"）

/**
 * @brief 计算数据的MD5值（16字节）
 * @param data 待计算MD5的数据源地址
 * @param len  数据源长度（字节数）
 * @param md5  输出参数：存储计算后的MD5值（需提前分配16字节空间）
 * @note 与服务端calc_md5函数实现完全一致，确保校验结果匹配
 */
void calc_md5(const unsigned char *data, int len, unsigned char md5[16]) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, data, len);
    MD5_Final(md5, &ctx);
}

/**
 * @brief 断点续传客户端核心逻辑
 * @param server_ip 服务端IP地址字符串（如"127.0.0.1"）
 * @param server_port 服务端监听端口（如8888）
 * @param local_file_path 本地待传输的文件路径（如"./large_file.mp4"）
 * @return 0：传输成功，-1：传输失败
 */
int resume_transfer_client(const char *server_ip, int server_port, const char *local_file_path) {
    // 1. 检查本地文件是否存在且可读
    struct stat local_file_stat;
    if (stat(local_file_path, &local_file_stat) == -1) {
        perror("resume_transfer_client: local file not exist or unreadable");
        return -1;
    }
    // 打开本地文件（只读模式，需从断点位置读取）
    int local_fd = open(local_file_path, O_RDONLY);
    if (local_fd == -1) {
        perror("resume_transfer_client: open local file failed");
        return -1;
    }

    // 2. 创建TCP套接字，连接服务端
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("resume_transfer_client: create socket failed");
        close(local_fd);
        return -1;
    }
    // 配置服务端地址结构体
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);  // 端口号：主机字节序→网络字节序
    // IP地址：字符串→网络字节序（如"127.0.0.1"→0x7F000001）
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr.s_addr) <= 0) {
        fprintf(stderr, "resume_transfer_client: invalid server IP: %s\n", server_ip);
        close(sock_fd);
        close(local_fd);
        return -1;
    }
    // 连接服务端
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("resume_transfer_client: connect to server failed");
        close(sock_fd);
        close(local_fd);
        return -1;
    }
    printf("成功连接服务端：%s:%d\n", server_ip, server_port);

    // 3. 提取本地文件名（从路径中截取，如"./test.mp4"→"test.mp4"）
    char filename[FILENAME_MAX] = {0};
    const char *last_slash = strrchr(local_file_path, '/');
    if (last_slash != NULL) {
        strncpy(filename, last_slash + 1, FILENAME_MAX - 1);  // 跳过'/'，复制文件名
    } else {
        strncpy(filename, local_file_path, FILENAME_MAX - 1);  // 无路径，直接用原名称
    }
    // 发送文件名给服务端（固定长度FILENAME_MAX，与服务端读取逻辑匹配）
    if (write(sock_fd, filename, FILENAME_MAX) != FILENAME_MAX) {
        perror("resume_transfer_client: send filename failed");
        close(sock_fd);
        close(local_fd);
        return -1;
    }
    printf("已向服务端发送文件名：%s\n", filename);

    // 4. 接收服务端返回的断点位置（已传输的字节数）
    off_t resume_pos = 0;
    if (read(sock_fd, &resume_pos, sizeof(resume_pos)) != sizeof(resume_pos)) {
        perror("resume_transfer_client: read resume_pos failed");
        close(sock_fd);
        close(local_fd);
        return -1;
    }
    // 计算本地文件总大小和剩余待传输字节数
    off_t local_file_total = local_file_stat.st_size;
    off_t remaining_bytes = local_file_total - resume_pos;
    if (remaining_bytes <= 0) {
        printf("本地文件已全部传输完成（服务端文件大小：%lld字节，本地文件大小：%lld字节）\n",
               (long long)resume_pos, (long long)local_file_total);
        close(sock_fd);
        close(local_fd);
        return 0;
    }
    printf("断点位置：%lld字节，本地文件总大小：%lld字节，剩余待传输：%lld字节\n",
           (long long)resume_pos, (long long)local_file_total, (long long)remaining_bytes);

    // 5. 定位本地文件指针到断点位置，准备开始传输
    if (lseek(local_fd, resume_pos, SEEK_SET) == -1) {
        perror("resume_transfer_client: lseek local file to resume_pos failed");
        close(sock_fd);
        close(local_fd);
        return -1;
    }

    // 6. 分块读取本地文件，传输给服务端并校验MD5
    unsigned char buf[BUF_SIZE];          // 数据块缓冲区
    unsigned char md5[16] = {0};          // 本地计算的MD5值
    char server_ack;                      // 服务端的确认信号（'A'=成功，'N'=重传）
    ssize_t read_len;                     // 本地文件读取的字节数
    off_t transferred_bytes = 0;          // 本次传输已完成的字节数（用于进度显示）

    while (1) {
        // 计算本次需读取的最大字节数（避免最后一块不足BUF_SIZE）
        size_t read_max = (remaining_bytes > BUF_SIZE) ? BUF_SIZE : remaining_bytes;
        // 从本地文件读取数据块
        read_len = read(local_fd, buf, read_max);
        if (read_len == -1) {
            perror("resume_transfer_client: read local file block failed");
            break;
        } else if (read_len == 0) {
            // 读取到0表示本地文件已读完（传输完成）
            printf("\n本地文件读取完成，等待服务端确认...\n");
            break;
        }

        // 计算当前数据块的MD5值
        calc_md5(buf, read_len, md5);
        // 先发送数据块，再发送MD5值（与服务端"先读数据再读MD5"的逻辑匹配）
        if (write(sock_fd, buf, read_len) != read_len) {
            perror("resume_transfer_client: send data block failed");
            break;
        }
        if (write(sock_fd, md5, 16) != 16) {
            perror("resume_transfer_client: send MD5 failed");
            break;
        }

        // 等待服务端确认（'A'=成功，'N'=需重传当前块）
        if (read(sock_fd, &server_ack, 1) != 1) {
            perror("resume_transfer_client: read server ack failed");
            break;
        }
        if (server_ack == 'A') {
            // 确认成功：更新剩余字节数和已传输字节数
            remaining_bytes -= read_len;
            transferred_bytes += read_len;
            // 显示传输进度（百分比）
            float progress = (float)(resume_pos + transferred_bytes) / local_file_total * 100;
            printf("\r传输进度：%.2f%%（已传：%lld字节，剩余：%lld字节）",
                   progress, (long long)(resume_pos + transferred_bytes), (long long)remaining_bytes);
            fflush(stdout);  // 强制刷新输出缓冲区，确保进度实时显示
        } else if (server_ack == 'N') {
            // 确认失败：需重传当前块（不更新计数，重新读取当前块）
            printf("\n服务端提示MD5校验失败，重传当前数据块...\n");
            // 回退本地文件指针，重新读取当前块
            lseek(local_fd, -read_len, SEEK_CUR);
        } else {
            // 未知确认信号：异常退出
            fprintf(stderr, "\nresume_transfer_client: unknown server ack: %c\n", server_ack);
            break;
        }
    }

    // 7. 检查传输结果
    if (remaining_bytes == 0) {
        printf("\n文件传输完成！总传输大小：%lld字节\n", (long long)local_file_total);
    } else {
        fprintf(stderr, "\n文件传输中断！已传输：%lld字节，剩余：%lld字节\n",
                (long long)(resume_pos + transferred_bytes), (long long)remaining_bytes);
        close(sock_fd);
        close(local_fd);
        return -1;
    }

    // 8. 释放资源：关闭套接字和本地文件
    close(sock_fd);
    close(local_fd);
    return 0;
}

// 主函数：解析命令行参数并启动客户端
int main(int argc, char *argv[]) {
    // 检查命令行参数：需传入"服务端端口号"和"本地待传输文件路径"
    if (argc != 3) {
        fprintf(stderr, "用法错误！正确格式：%s <服务端端口号> <本地待传输文件路径>\n", argv[0]);
        fprintf(stderr, "示例：%s 8888 ./large_video.mp4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 解析参数：服务端端口号（字符串→整数）
    int server_port = atoi(argv[1]);
    if (server_port <= 0 || server_port > 65535) {
        fprintf(stderr, "无效端口号！端口号范围：1~65535\n");
        exit(EXIT_FAILURE);
    }
    const char *local_file_path = argv[2];  // 本地待传输文件路径

    // 启动断点续传客户端（默认连接本地服务端，可修改SERVER_IP为远程IP）
    int ret = resume_transfer_client(SERVER_IP, server_port, local_file_path);
    if (ret == 0) {
        printf("客户端任务完成，退出\n");
        exit(EXIT_SUCCESS);
    } else {
        fprintf(stderr, "客户端任务失败，退出\n");
        exit(EXIT_FAILURE);
    }
}
