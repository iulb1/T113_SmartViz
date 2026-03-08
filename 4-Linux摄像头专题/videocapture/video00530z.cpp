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
    
    if (!cap.open(0, cv::CAP_V4L2)) {
        fprintf(stderr, "ERROR! Unable to open camera with V4L2 backend\n");
        if (!cap.open(0)) {
            fprintf(stderr, "ERROR! Unable to open camera\n");
            return -1;
        }
    } else {
        printf("Camera opened with V4L2 backend\n");
    }
    
    if (!cap.isOpened()) {
        fprintf(stderr, "ERROR! Unable to open camera\n");
        return -1;
    }

    // 关键优化1: 设置缓冲区大小为1，减少延迟（最重要！）
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    
    // 设置帧率为30fps
    cap.set(cv::CAP_PROP_FPS, 30);
    
    // V4L2特定优化：尝试使用MJPEG格式
    // cv::VideoWriter::fourcc(): 生成视频编码器四字符码
    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    // cv::CAP_PROP_FOURCC: 像素格式属性
    cap.set(cv::CAP_PROP_FOURCC, fourcc);
    
    // 输出当前使用的格式和帧率
    int actualFourcc = static_cast<int>(cap.get(cv::CAP_PROP_FOURCC));
    char formatStr[5];
    formatStr[0] = (char)(actualFourcc & 0xFF);
    formatStr[1] = (char)((actualFourcc >> 8) & 0xFF);
    formatStr[2] = (char)((actualFourcc >> 16) & 0xFF);
    formatStr[3] = (char)((actualFourcc >> 24) & 0xFF);
    formatStr[4] = '\0';
    printf("Video format: %s\n", formatStr);
    
    double actualFps = cap.get(cv::CAP_PROP_FPS);
    printf("Frame rate: %.2f fps\n", actualFps);

    printf("Stage 4: MJPEG format optimization\n");
    printf("Optimizations: V4L2, BUFFERSIZE=1, 30fps, MJPEG format\n");
    printf("Expected latency: ~40-45ms\n\n");
    
    int frameCount = 0;
    double totalLatency = 0;
    
    while (true) {
        double grabStart = get_time_ms();
        
        if (!cap.grab()) {
            fprintf(stderr, "ERROR! Unable to grab frame\n");
            break;
        }
        
        double grabEnd = get_time_ms();
        
        // retrieve()解码最新抓取的帧（MJPEG解码）
        if (!cap.retrieve(frame)) {
            fprintf(stderr, "ERROR! Unable to retrieve frame\n");
            break;
        }
        
        double retrieveEnd = get_time_ms();
        
        if (frame.empty()) {
            fprintf(stderr, "ERROR! blank frame grabbed\n");
            break;
        }
        
        cv::imshow("Live", frame);
        double displayEnd = get_time_ms();
        
        frameCount++;
        double grabLatency = time_diff_ms(grabStart, grabEnd);
        double retrieveLatency = time_diff_ms(grabEnd, retrieveEnd);
        double displayLatency = time_diff_ms(retrieveEnd, displayEnd);
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
