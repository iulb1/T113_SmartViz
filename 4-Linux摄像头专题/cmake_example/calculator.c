#include "calculator.h"

// 加法
double add(double a, double b)
{
    return a + b;
}

// 减法
double subtract(double a, double b)
{
    return a - b;
}

// 乘法
double multiply(double a, double b)
{
    return a * b;
}

// 除法
double divide(double a, double b)
{
    if (b == 0.0) {
        return 0.0; // 简单处理除零情况
    }
    return a / b;
}

