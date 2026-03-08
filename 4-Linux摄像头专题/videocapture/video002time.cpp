// OpenCV C++ API（使用命名空间cv::）
#include <opencv2/opencv.hpp>

// Linux C标准库（输入输出、时间等）
#include <stdio.h>
#include <sys/time.h>

// Linux C接口：获取当前时间（毫秒）
static double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

// Linux C接口：计算时间差（毫秒）
static double time_diff_ms(double start, double end) {
    return end - start;
}

int main() {
    cv::Mat frame;
    cv::VideoCapture cap;
    cap.open(0);  // 使用默认后端
    
    if (!cap.isOpened()) {
        fprintf(stderr, "打开摄像头失败\n");
        return -1;
    }

    printf("原始场景，加入运行时间\n");
    
    // 延迟测量（使用Linux C接口gettimeofday）
    int frameCount = 0;
    double totalLatency = 0;
    
    while (true) {
        // Linux C接口：获取read开始时间
        double readStart = get_time_ms();
        
        // 使用默认的read()方法（包含缓冲区累积问题）
        // cv::VideoCapture::read(): OpenCV C++ API，读取一帧图像
        cap.read(frame);
        
        // Linux C接口：获取read结束时间
        double readEnd = get_time_ms();
        
        if (frame.empty()) {
            fprintf(stderr, "grabbed 错误\n");
            break;
        }
        
        // Linux C接口：获取display开始时间
        double displayStart = get_time_ms();
        
        // cv::imshow(): OpenCV C++ API，在窗口中显示图像
        cv::imshow("Live", frame);
        
        // Linux C接口：获取display结束时间
        double displayEnd = get_time_ms();
        
        // 计算各阶段延迟（使用Linux C接口计算时间差）
        frameCount++;
        double readLatency = time_diff_ms(readStart, readEnd);
        double displayLatency = time_diff_ms(displayStart, displayEnd);
        double totalFrameLatency = time_diff_ms(readStart, displayEnd);
        
        totalLatency += totalFrameLatency;
        
        if (frameCount % 50 == 0) {
            double avgLatency = totalLatency / frameCount;
            printf("Frame %d - Read: %.3fms, Display: %.3fms, Total: %.3fms, Avg: %.3fms\n",
                   frameCount, readLatency, displayLatency, totalFrameLatency, avgLatency);
        }
        
        // cv::waitKey(): OpenCV C++ API，等待按键
        if (cv::waitKey(5) >= 0)
            break;
    }
    return 0;
}
