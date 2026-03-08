#include <stdio.h>
#include <stdlib.h>

int main() {
    // 1. 打开/创建文件："w+"表示读写模式
    // - 若文件不存在：创建新文件
    // - 若文件已存在：清空原有内容（覆盖写入）
    FILE *fp = fopen("test_libc.txt", "w+");
    // 流操作失败时返回NULL，必须检查（如路径权限不足、磁盘满）
    if (fp == NULL) {
        perror("fopen failed");  // 打印系统错误原因（如"fopen failed: Permission denied"）
        exit(EXIT_FAILURE);      // 退出程序，标记为失败状态
    }

    // 2. 格式化写入数据：fprintf支持变量嵌入，比POSIX的write更灵活
    const char *name = "Linux";  // 字符串变量
    int version = 5;             // 整数变量
    // 写入内容："Hello Linux File! Version: 5\n"（包含换行符）
    int write_count = fprintf(fp, "Hello %s File! Version: %d\n", name, version);
    // 写入失败时返回负数（如磁盘满、文件被意外删除）
    if (write_count < 0) {
        perror("fprintf failed");
        fclose(fp);  // 出错时必须关闭流，避免缓冲区数据泄漏
        exit(EXIT_FAILURE);
    }
    // write_count为实际写入的字符数（包含换行符，此处共28个字符）
    printf("实际写入字符数：%d\n", write_count);

    // 3. 移动文件流指针到开头：准备读取刚写入的内容
    // SEEK_SET：以文件开头为基准，偏移量0（即定位到文件起始位置）
    int seek_ret = fseek(fp, 0, SEEK_SET);
    // fseek失败返回非0（如文件已被其他进程删除、设备文件无法定位）
    if (seek_ret != 0) {
        perror("fseek failed");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    // 4. 读取文件内容：fgets按行读取，自动处理换行符
    char buf[1024] = {0};  // 初始化缓冲区为0，避免读取到垃圾数据
    // fgets参数：缓冲区地址、缓冲区大小（1024）、目标流
    // 特性：最多读取1023个字符（留1个位置存字符串结束符'\0'），遇到换行符停止
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        // 区分"读取失败"和"正常到达文件末尾"
        if (feof(fp)) {
            printf("已到文件末尾\n");
        } else {
            perror("fgets failed");
        }
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    // buf中已包含fprintf写入的换行符，printf无需额外加'\n'
    printf("读取到的内容：%s", buf);

    // 5. 关闭文件流：自动刷新缓冲区（将未写入磁盘的数据强制刷盘）
    // 必须调用fclose，否则可能导致缓冲区数据丢失（尤其是突然退出程序时）
    fclose(fp);

    return EXIT_SUCCESS;
}
