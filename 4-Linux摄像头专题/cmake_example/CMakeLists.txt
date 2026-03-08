# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(Calculator)

# 设置C标准
set(CMAKE_C_STANDARD 99)

# 添加可执行文件
add_executable(calculator
    main.c
    calculator.c
)

# 设置输出目录（可选）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

