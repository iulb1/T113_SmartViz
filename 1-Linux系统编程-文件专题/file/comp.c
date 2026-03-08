
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

#define DATA_COUNT 100000  // 数据量
#define MAX_NAME 32
#define MAX_PHONE 16
#define PHONE_LEN 11       // 手机号固定长度
#define CSV_LINE_MAX 128   // 单条CSV记录最大长度

typedef struct {
    int id;
    char name[MAX_NAME];
    int age;
    char phone[MAX_PHONE];
} User;

// 生成测试数据
void generate_user(int id, User *user) {
    user->id = id;
    snprintf(user->name, MAX_NAME, "User_%d", id);
    user->age = 20 + (id % 30);
    snprintf(user->phone, MAX_PHONE, "138%08d", id % 100000000);
}

// 手机号合法性校验
int is_valid_phone(const char *phone) {
    if (strlen(phone) != PHONE_LEN || strncmp(phone, "138", 3) != 0) return 0;
    for (int i = 0; i < PHONE_LEN; i++) {
        if (!isdigit(phone[i])) return 0;
    }
    return 1;
}

// -------------------------- 统一使用系统调用（open/read/write） --------------------------

// 二进制写入（系统调用）
double binary_write(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) { perror("binary open write"); return -1; }

    User user;
    clock_t start = clock();
    for (int i = 1; i <= DATA_COUNT; i++) {
        generate_user(i, &user);
        write(fd, &user, sizeof(User));  // 直接写结构体
    }
    close(fd);
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

// 二进制读取（系统调用）
double binary_read(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) { perror("binary open read"); return -1; }

    struct stat st;
    fstat(fd, &st);
    if (st.st_size != DATA_COUNT * sizeof(User)) {
        fprintf(stderr, "binary size mismatch\n");
        close(fd);
        return -1;
    }

    User user;
    long long total_age = 0;
    clock_t start = clock();
    
    for (int i = 0; i < DATA_COUNT; i++) {
        read(fd, &user, sizeof(User));  // 直接读结构体
        total_age += user.age;
    }
    
    close(fd);
    printf("二进制验证：平均年龄=%.2f\n", (double)total_age / DATA_COUNT);
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

// CSV写入（系统调用，与二进制IO方式一致）
double csv_write(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) { perror("csv open write"); return -1; }

    User user;
    char line[CSV_LINE_MAX];
    clock_t start = clock();
    
    for (int i = 1; i <= DATA_COUNT; i++) {
        generate_user(i, &user);
        // 先格式化字符串（CSV核心开销：结构体→字符串）
        int len = snprintf(line, CSV_LINE_MAX, "%d,%s,%d,%s\n",
                          user.id, user.name, user.age, user.phone);
        write(fd, line, len);  // 同样用系统调用write
    }
    
    close(fd);
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

// 读取完整的一行（直到换行符或EOF）
static ssize_t read_line(int fd, char *buf, size_t max_len) {
    ssize_t n;
    size_t total = 0;
    char c;
    
    while (total < max_len - 1) {  // 留一个位置给终止符
        n = read(fd, &c, 1);
        if (n <= 0) break;  // EOF或错误
        
        buf[total++] = c;
        if (c == '\n') break;  // 遇到换行符结束
    }
    
    buf[total] = '\0';  // 添加字符串终止符
    return total > 0 ? total : n;  // 返回读取的长度或错误码
}

// CSV读取（系统调用，修复了行解析问题）
double csv_read(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) { perror("csv open read"); return -1; }

    char line[CSV_LINE_MAX];
    User user;
    long long total_age = 0;
    int count = 0;
    ssize_t n;
    clock_t start = clock();
    
    // 逐行读取（确保每次处理完整的一行）
    while ((n = read_line(fd, line, CSV_LINE_MAX)) > 0 && count < DATA_COUNT) {
        // 解析CSV（字符串分割+格式转换）
        if (sscanf(line, "%d,%[^,],%d,%s", &user.id, user.name, &user.age, user.phone) != 4) {
            fprintf(stderr, "csv parse error at line %d: %s\n", count + 1, line);
            close(fd);
            return -1;
        }
        
        // 数据校验
        if (user.id != count + 1 || !is_valid_phone(user.phone)) {
            fprintf(stderr, "csv data invalid at line %d\n", count + 1);
            close(fd);
            return -1;
        }
        
        total_age += user.age;
        count++;
    }
    
    close(fd);
    if (count != DATA_COUNT) {
        fprintf(stderr, "csv count mismatch: %d vs %d\n", count, DATA_COUNT);
        return -1;
    }
    printf("CSV验证：平均年龄=%.2f\n", (double)total_age / DATA_COUNT);
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

int main() {
    const char *bin_path = "test_bin.dat";
    const char *csv_path = "test_csv.csv";
    double bin_write_t, bin_read_t, csv_write_t, csv_read_t;

    // 热身操作
    binary_write(bin_path);
    binary_read(bin_path);
    csv_write(csv_path);
    csv_read(csv_path);

    // 正式测试
    printf("=== 统一系统调用IO对比（%d条数据）===\n", DATA_COUNT);
    
    // 写入测试
    bin_write_t = binary_write(bin_path);
    csv_write_t = csv_write(csv_path);
    printf("\n写入耗时：\n");
    printf("二进制：%.4f秒\n", bin_write_t);
    printf("CSV：%.4f秒\n", csv_write_t);
    printf("二进制快：%.2f倍\n", csv_write_t / bin_write_t);
    
    // 读取测试
    bin_read_t = binary_read(bin_path);
    csv_read_t = csv_read(csv_path);
    printf("\n读取耗时：\n");
    printf("二进制：%.4f秒\n", bin_read_t);
    printf("CSV：%.4f秒\n", csv_read_t);
    printf("二进制快：%.2f倍\n", csv_read_t / bin_read_t);
    
    // 总耗时
    printf("\n总耗时（写+读）：\n");
    printf("二进制：%.4f秒\n", bin_write_t + bin_read_t);
    printf("CSV：%.4f秒\n", csv_write_t + csv_read_t);
    printf("二进制快：%.2f倍\n", (csv_write_t + csv_read_t) / (bin_write_t + bin_read_t));

    // 清理
   // remove(bin_path);
   // remove(csv_path);
    return 0;
}
    
