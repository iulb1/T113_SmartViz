#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main() {
    int fd;
    ssize_t bytes_read, bytes_written;
    char buffer[1024];
    const char *message = "Hello, strace!\nThis is a test for file operations.\n";
    char input[10]; // 用于接收用户输入
    
    
    // 创建并写入文件
    printf("\n===== 开始写入文件 =====\n");
    printf("按回车键继续写入操作...\n");
    scanf("%*c");
    
    fd = open("testfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open for writing failed");
        return 1;
    }
    
    bytes_written = write(fd, message, strlen(message));
    if (bytes_written == -1) {
        perror("write failed");
        close(fd);
        return 1;
    }
    printf("Wrote %zd bytes\n", bytes_written);
    printf("按回车键关闭文件");
    scanf("%*c");
    if (close(fd) == -1) {
        perror("close after write failed");
        return 1;
    }
    
    // 读取文件内容
    printf("\n===== 开始读取文件 =====\n");
    printf("按回车键继续读取操作...\n");
    scanf("%*c");
    
    fd = open("testfile.txt", O_RDONLY);
    if (fd == -1) {
        perror("open for reading failed");
        return 1;
    }
    
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("read failed");
        close(fd);
        return 1;
    }
    buffer[bytes_read] = '\0'; // 添加字符串结束符
    printf("Read %zd bytes:\n%s\n", bytes_read, buffer);
    
	printf("按回车键关闭文件");
    scanf("%*c");
    
    if (close(fd) == -1) {
        perror("close after read failed");
        return 1;
    }
    
    printf("\n===== 所有操作完成 =====\n");
    printf("按回车键退出程序...\n");
    scanf("%*c");
    
    return 0;
}
