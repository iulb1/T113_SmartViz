#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main() {
    // 打印当前进程ID
    printf("当前进程ID: %d\n", getpid());
    
    // 查看默认打开的FD
    printf("\n=== 默认FD列表 ===\n");
    system("ls -l /proc/self/fd/");  // /proc/self 指向当前进程
    
    // 测试fd=1 (标准输出)
    printf("\n=== 测试fd=1 (标准输出) ===\n");
    const char *msg1 = "这句话通过fd=1写入(标准输出)\n";
    ssize_t bytes_written = write(1, msg1, strlen(msg1));
    printf("fd=1写入了 %zd 字节\n", bytes_written);
    
    // 测试fd=2 (标准错误)
    printf("\n=== 测试fd=2 (标准错误) ===\n");
    const char *msg2 = "这句话通过fd=2写入(标准错误)\n";
    bytes_written = write(2, msg2, strlen(msg2));
    printf("fd=2写入了 %zd 字节\n", bytes_written);
    
    // 测试fd=0 (标准输入)
    printf("\n=== 测试fd=0 (标准输入) ===\n");
    printf("请输入一些文字(按回车结束): ");
    char buffer[100];
    ssize_t bytes_read = read(0, buffer, sizeof(buffer)-1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';  // 添加字符串结束符
        printf("fd=0读取了 %zd 字节: %s", bytes_read, buffer);
    } else {
        printf("fd=0读取失败或无数据\n");
    }
    
    // 手动打开一个新文件，验证FD从3开始分配
    int fd = open("test.txt", O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("open failed");
        return 1;
    }
    printf("\n=== 手动打开文件后FD列表 ===\n");
    system("ls -l /proc/self/fd/");
    printf("新打开的文件使用fd=%d\n", fd);
    
    // 关闭手动打开的FD
    close(fd);
    printf("\n=== 关闭手动打开的FD后 ===\n");
    system("ls -l /proc/self/fd/");
    
    return 0;
}
