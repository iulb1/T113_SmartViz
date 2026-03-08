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
    
    // 关键优化：使用V4L2后端，通常延迟更低
    // cv::CAP_V4L2: V4L2后端标识符
    // V4L2是Linux下最直接的视频接口，绕过中间层
    if (!cap.open(0, cv::CAP_V4L2)) {
        fprintf(stderr, "通过v4l2框架打开失败\n");
    } else {
        printf("V4L2直开测试\n");
    }
    
    if (!cap.isOpened()) {
        fprintf(stderr, "摄像头打开错误\n");
        return -1;
    }

    // 关键优化1: 设置缓冲区大小为1，减少延迟
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    printf("缓冲区大小设为1\n");
     
    int frameCount = 0;
    double totalLatency = 0;
    
    while (true) {
        double grabStart = get_time_ms();
        if (!cap.grab()) {
            fprintf(stderr, "grab frame错误\n");
            break;
        }
        double grabEnd = get_time_ms();
        
        if (!cap.retrieve(frame)) {
            fprintf(stderr, "retrieve 错误\n");
            break;
        }
        
        double retrieveEnd = get_time_ms();
        
        if (frame.empty()) {
            fprintf(stderr, "grabbed 错误\n");
            break;
        }
        
        double displayStart = get_time_ms();
        cv::imshow("Live", frame);
        double displayEnd = get_time_ms();
        
        frameCount++;
        double grabLatency = time_diff_ms(grabStart, grabEnd);
        double retrieveLatency = time_diff_ms(grabEnd, retrieveEnd);
        double displayLatency = time_diff_ms(displayStart, displayEnd);
        double totalFrameLatency = time_diff_ms(grabStart, displayEnd);
        
        totalLatency += totalFrameLatency;
        
        if (frameCount % 100 == 0) {
            double avgLatency = totalLatency / frameCount;
            printf("Frame %d - Grab: %.3fms, Retrieve: %.3fms, Display: %.3fms, Total: %.3fms, Avg: %.3fms\n",
                   frameCount, grabLatency, retrieveLatency, displayLatency, totalFrameLatency, avgLatency);
        }
        
        if (cv::waitKey(1) >= 0)
            break;
    }
    return 0;
}
