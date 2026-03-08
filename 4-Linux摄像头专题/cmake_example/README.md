# CMake 示例工程 - 计算器

这是一个简单的 CMake 示例工程，演示如何使用 CMake 构建 C 语言项目。

## 项目结构

```
cmake_example/
├── CMakeLists.txt    # CMake 配置文件
├── calculator.h       # 计算器头文件
├── calculator.c       # 计算器实现
├── main.c            # 主程序
└── README.md         # 说明文档
```

## 使用步骤

### 1. 创建构建目录
```bash
mkdir build
cd build
```

### 2. 生成 Makefile
```bash
cmake ..
```

### 3. 编译项目
```bash
make
```

### 4. 运行程序
```bash
./calculator
# 或者
./bin/calculator
```

## 预期输出

```
=== 计算器示例 ===
a = 10.00, b = 3.00

加法: 10.00 + 3.00 = 13.00
减法: 10.00 - 3.00 = 7.00
乘法: 10.00 * 3.00 = 30.00
除法: 10.00 / 3.00 = 3.33
```

## CMakeLists.txt 说明

- `cmake_minimum_required(VERSION 3.10)`: 指定 CMake 最低版本
- `project(Calculator)`: 定义项目名称
- `set(CMAKE_C_STANDARD 99)`: 设置 C 标准为 C99
- `add_executable(...)`: 定义可执行文件及其源文件
- `set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ...)`: 设置输出目录

## 清理构建文件

```bash
cd build
rm -rf *
# 或者
cd ..
rm -rf build
```

