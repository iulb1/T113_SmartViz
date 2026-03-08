#include <stdio.h>
#include "calculator.h"

int main()
{
    double a = 10.0;
    double b = 3.0;
    
    printf("=== 计算器示例 ===\n");
    printf("a = %.2f, b = %.2f\n\n", a, b);
    
    printf("加法: %.2f + %.2f = %.2f\n", a, b, add(a, b));
    printf("减法: %.2f - %.2f = %.2f\n", a, b, subtract(a, b));
    printf("乘法: %.2f * %.2f = %.2f\n", a, b, multiply(a, b));
    printf("除法: %.2f / %.2f = %.2f\n", a, b, divide(a, b));
    
    return 0;
}

