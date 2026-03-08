#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <openssl/md5.h>  // 依赖OpenSSL库，编译时需链接：-lcrypto

#define BUF_SIZE 65536     // 数据块大小（64KB）：平衡传输效率与内存占用
#define FILENAME_MAX 256   // 文件名最大长度：避免缓冲区溢出
#define LISTEN_BACKLOG 5   // 监听队列长度：默认值，支持同时5个客户端等待连接

/**
 * @brief 计算数据的MD5值（16字节）
 * @param data 待计算MD5的数据源地址
 * @param len  数据源长度（字节数）
 * @param md5  输出参数：存储计算后的MD5值（需提前分配16字节空间）
 */
void calc_md5(const unsigned char *data, int len, unsigned char md5[16]) {
    MD5_CTX ctx;          // MD5上下文结构体
    MD5_Init(&ctx);       // 初始化MD5上下文
    MD5_Update(&ctx, data, len);  // 输入数据到MD5上下文
    MD5_Final(md5, &ctx); // 完成MD5计算，结果存入md5数组
}

/**
 * @brief 处理单个客户端的断点续传请求
 * @param client_fd 与客户端通信的套接字FD
 * @note 流程：1. 读取文件名 → 2. 定位断点位置 → 3. 分块接收数据并校验 → 4. 完成传输
 */
void handle_resume_transfer(int client_fd) {
    // 1. 读取客户端发送的文件名（固定长度FILENAME_MAX，避免变长读取的不确定性）
    char filename[FILENAME_MAX] = {0};  // 初始化缓冲区，避免垃圾数据
    ssize_t read_ret = read(client_fd, filename, FILENAME_MAX);
    if (read_ret == -1) {
        perror("handle_resume_transfer: read filename failed");
        return;
    } else if (read_ret == 0) {
        fprintf(stderr, "handle_resume_transfer: client closed before send filename\n");
        return;
    }
    // 去除文件名中可能的空字符（防止客户端传入异常数据）
    filename[strcspn(filename, "\0")] = '\0';
    printf("开始接收文件：%s\n", filename);

    // 2. 检查文件是否已存在，确定断点位置（已传输的字节数）
    struct stat file_stat;
    off_t resume_pos = 0;  // 断点位置：默认从0开始（新文件）
    // 打开文件：O_RDWR（读写）| O_CREAT（不存在则创建），权限0644（所有者读写，其他读）
    int file_fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (file_fd == -1) {
        perror("handle_resume_transfer: open file failed");
        // 向客户端发送错误标记（此处简化为发送0，实际可定义错误码）
        write(client_fd, &resume_pos, sizeof(resume_pos));
        return;
    }
    // 若文件已存在，获取当前大小作为断点位置
    if (stat(filename, &file_stat) == 0) {
        resume_pos = file_stat.st_size;
        printf("文件已存在，断点位置：%lld 字节\n", (long long)resume_pos);
    }

    // 3. 向客户端发送断点位置，客户端从该位置开始传输
    if (write(client_fd, &resume_pos, sizeof(resume_pos)) == -1) {
        perror("handle_resume_transfer: send resume_pos failed");
        close(file_fd);
        return;
    }
    // 将文件指针定位到断点位置，准备续写数据
    if (lseek(file_fd, resume_pos, SEEK_SET) == -1) {
        perror("handle_resume_transfer: lseek to resume_pos failed");
        close(file_fd);
        return;
    }

    // 4. 循环接收数据块，校验MD5后写入文件
    unsigned char buf[BUF_SIZE];          // 数据块缓冲区
    unsigned char client_md5[16] = {0};   // 客户端发送的MD5值
    unsigned char server_md5[16] = {0};   // 服务器计算的MD5值
    while (1) {
        // 读取客户端发送的数据块（最大BUF_SIZE字节）
        read_ret = read(client_fd, buf, BUF_SIZE);
        if (read_ret == -1) {
            perror("handle_resume_transfer: read data block failed");
            break;
        } else if (read_ret == 0) {
            // 读取到0表示客户端关闭连接，传输完成
            printf("客户端已断开连接，传输结束\n");
            break;
        }

        // 读取客户端发送的MD5值（固定16字节）
        if (read(client_fd, client_md5, 16) != 16) {
            fprintf(stderr, "handle_resume_transfer: read MD5 failed (invalid length)\n");
            break;
        }

        // 计算接收数据块的MD5值，与客户端发送的MD5比对
        calc_md5(buf, read_ret, server_md5);
        if (memcmp(client_md5, server_md5, 16) == 0) {
            // MD5校验通过，写入数据到文件
            if (write(file_fd, buf, read_ret) != read_ret) {
                perror("handle_resume_transfer: write data to file failed");
                break;
            }
            // 向客户端发送“接收确认”（'A'表示成功）
            char ack = 'A';
            write(client_fd, &ack, 1);
        } else {
            // MD5校验失败，要求客户端重传当前块
            char ack = 'N';
            write(client_fd, &ack, 1);
            printf("数据块MD5校验失败，要求客户端重传\n");
        }
    }

    // 计算最终文件大小（断点位置 + 本次传输的字节数）
    off_t final_size = resume_pos + lseek(file_fd, 0, SEEK_CUR) - resume_pos;
    printf("文件传输完成，最终大小：%lld 字节\n", (long long)final_size);

    // 释放资源：关闭文件FD（必须执行，避免泄漏）
    close(file_fd);
}

int main(int argc, char *argv[]) {
    // 检查命令行参数：需传入端口号（如./server 8888）
    if (argc != 2) {
        fprintf(stderr, "用法错误！正确格式：%s <端口号>\n", argv[0]);
        fprintf(stderr, "示例：%s 8888\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // 将端口号从字符串转为整数（需检查有效性）
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "无效端口号！端口号范围：1~65535\n");
        exit(EXIT_FAILURE);
    }

    // 1. 创建TCP套接字（AF_INET：IPv4，SOCK_STREAM：TCP，0：默认协议）
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("main: create socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. 设置服务器地址结构体
    struct sockaddr_in server_addr = {0};  // 初始化，避免随机值
    server_addr.sin_family = AF_INET;      // IPv4地址族
    server_addr.sin_port = htons(port);    // 端口号：主机字节序→网络字节序（大端）
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 绑定所有网卡的IP（监听所有网络接口）

    // 3. 绑定套接字到指定端口（解决“地址已被使用”问题：设置SO_REUSEADDR）
    int reuse = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("main: setsockopt SO_REUSEADDR failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("main: bind socket failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 4. 将套接字设为监听模式（等待客户端连接）
    if (listen(sock_fd, LISTEN_BACKLOG) == -1) {
        perror("main: listen failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("服务器启动成功，监听端口：%d（等待客户端连接...）\n", port);

    // 5. 循环接收客户端连接（单线程：一次处理一个客户端，适合演示；生产环境需多线程/IO多路复用）
    while (1) {
        struct sockaddr_in client_addr;  // 存储客户端地址信息
        socklen_t client_len = sizeof(client_addr);  // 地址长度（传入传出参数）
        // 接受客户端连接：返回与客户端通信的新FD（原sock_fd继续监听）
        int client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("main: accept client failed");
            continue;  // 接受失败不退出，继续等待下一个连接
        }
        // 处理当前客户端的断点续传请求
        handle_resume_transfer(client_fd);
        // 关闭与客户端的通信FD（传输完成后释放）
        close(client_fd);
        printf("客户端连接已关闭，等待下一个连接...\n\n");
    }

    // 理论上不会执行到这里（while(1)循环），但仍需关闭监听FD（规范）
    close(sock_fd);
    return EXIT_SUCCESS;
}
