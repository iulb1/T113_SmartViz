#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>


int main() {
    // 1. 创建并打开文件（只写模式，不存在则创建，存在则截断，权限0644）
    int fd = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {  // 必须检查返回值，排查错误（如权限不足、路径不存在）
        perror("open file failed");  // perror：自动拼接系统错误原因
        exit(EXIT_FAILURE);
    }
    printf("文件打开成功，FD：%d\n", fd);  // 首次打开时，FD默认从3开始（0/1/2为标准IO）

    // 2. 写入数据（字符串末尾的'\0'是C语言字符串结束符，无需写入文件）
    const char *content = "Hello Linux File!";
    // 字符串长度为16（"Hello Linux File!"共16个可见字符）
    ssize_t write_len = write(fd, content, strlen(content));
    if (write_len == -1) {
        perror("write file failed");
        close(fd);  // 出错时必须关闭FD，避免资源泄漏
        exit(EXIT_FAILURE);
    }
    printf("实际写入字节数：%zd\n", write_len);

    // 3. 移动文件指针到开头（当前文件以O_WRONLY模式打开，虽能移动指针但后续读取会失败）
    off_t new_pos = lseek(fd, 0, SEEK_SET);
    if (new_pos == -1) {
        perror("lseek failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 4. 读取数据（注意：此处存在逻辑问题！O_WRONLY模式打开的文件无法读取，read会返回-1）
    char buf[1024] = {0};  // 初始化缓冲区，避免读取到垃圾数据
    ssize_t read_len = read(fd, buf, sizeof(buf));
    printf("read_len:%ld\n",read_len);
    if (read_len == -1) {
        perror("read file failed");  // 此处会触发错误，因文件以只写模式打开
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("读取到的内容：%s\n", buf);

    // 5. 关闭文件（释放FD，避免资源泄漏）
    close(fd);
    return EXIT_SUCCESS;
}
