# Qt Designer 字体大小不一致问题解决方案

## 问题描述
在 Qt Designer 中设计的界面，在开发板上运行时字体大小不一致，通常表现为：
- 开发板上的字体比设计时更大或更小
- 不同分辨率/DPI的设备显示效果不同

## 问题原因

1. **DPI差异**：开发电脑和开发板的屏幕DPI不同
2. **字体缩放**：Qt的高DPI自动缩放机制
3. **字体缺失**：开发板上缺少设计时使用的字体
4. **字体策略**：未统一设置字体策略

## 解决方案

### 方案1：禁用高DPI缩放（最简单，但可能影响其他应用）

在 `main.cpp` 中，在创建 QApplication 之前设置：

```cpp
#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    // 禁用高DPI缩放
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    
    QApplication app(argc, argv);
    // ... 其他代码
}
```

### 方案2：设置固定的DPI（推荐）

```cpp
#include <QApplication>
#include <QScreen>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置固定的逻辑DPI（96是标准DPI）
    // 这样无论物理DPI是多少，都会按96DPI计算
    QApplication::setAttribute(Qt::AA_Use96Dpi);
    
    // 或者手动设置DPI
    // app.setAttribute(Qt::AA_EnableHighDpiScaling, false);
    
    // ... 其他代码
}
```

### 方案3：在代码中统一设置字体（最可靠）

```cpp
#include <QApplication>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 方法1：设置应用程序默认字体
    QFont font("Arial", 10);  // 字体名称和大小（像素）
    app.setFont(font);
    
    // 方法2：使用点大小（pt）而不是像素（px）
    // 点大小会根据DPI自动调整
    QFont font;
    font.setFamily("Arial");
    font.setPointSize(10);  // 使用点大小
    app.setFont(font);
    
    // 方法3：从系统获取标准字体并调整
    QFont font = QApplication::font();
    font.setPointSize(10);
    app.setFont(font);
    
    // ... 其他代码
}
```

### 方案4：使用环境变量（适用于嵌入式系统）

在启动脚本中设置：

```bash
# 禁用高DPI缩放
export QT_AUTO_SCREEN_SCALE_FACTOR=0

# 或者设置固定的缩放因子
export QT_SCALE_FACTOR=1.0

# 设置DPI
export QT_DEVICE_PIXEL_RATIO=1
```

### 方案5：在UI文件中使用相对大小

在 Qt Designer 中：
1. 不要使用固定的像素大小
2. 使用样式表设置字体大小（使用 `em` 或 `pt` 单位）
3. 使用布局管理器，让控件自适应

### 方案6：运行时检测并调整（最灵活）

```cpp
#include <QApplication>
#include <QScreen>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 获取主屏幕的DPI
    QScreen *screen = QApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    
    // 计算缩放因子（相对于96 DPI）
    qreal scaleFactor = dpi / 96.0;
    
    // 如果DPI差异太大，可以限制缩放范围
    if (scaleFactor > 2.0) scaleFactor = 2.0;
    if (scaleFactor < 0.5) scaleFactor = 0.5;
    
    // 根据缩放因子调整字体
    QFont font = QApplication::font();
    int baseSize = 10;  // 基础字体大小（96 DPI下的值）
    font.setPointSize(baseSize * scaleFactor);
    app.setFont(font);
    
    // ... 其他代码
}
```

## 推荐方案（嵌入式开发板）

对于嵌入式开发板，推荐使用**方案2 + 方案3**的组合：

1. 在 `main.cpp` 中设置固定DPI
2. 统一设置应用程序字体
3. 在启动脚本中设置环境变量（可选）

这样可以确保：
- 设计时的效果和运行时的效果一致
- 不依赖系统的DPI设置
- 字体大小可控

## 注意事项

1. **字体文件**：确保开发板上安装了设计时使用的字体，或者使用系统默认字体
2. **测试**：在不同分辨率的设备上测试
3. **样式表**：如果使用QSS样式表，注意字体大小的单位（px vs pt vs em）
4. **布局**：使用布局管理器而不是固定大小，让界面自适应

## 调试方法

在代码中添加调试信息：

```cpp
QScreen *screen = QApplication::primaryScreen();
qDebug() << "Physical DPI:" << screen->physicalDotsPerInch();
qDebug() << "Logical DPI:" << screen->logicalDotsPerInch();
qDebug() << "Device Pixel Ratio:" << screen->devicePixelRatio();
qDebug() << "Font:" << QApplication::font();
```

这样可以了解开发板的实际DPI设置，便于调整。





