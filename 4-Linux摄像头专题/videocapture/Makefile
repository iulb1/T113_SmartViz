CC = arm-openwrt-linux-g++
CXXFLAGS = -Wall -Wextra -O2 -g -pthread \
           -I /home/zhiwan/cprogrammer/camera/opencv/_install/include/opencv4

LDFLAGS = -pthread -L /home/zhiwan/cprogrammer/camera/opencv/_install/lib/ \
	  -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs \
          -L /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/lib/ -lz -ljpeg -lpng12 \
          -L /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/rootfs/usr/lib/arm-qt/lib/ \
          -lQt5Core \
          -lQt5Gui \
          -lQt5Widgets \
          -lQt5Test

TARGET = videocapture_basic
SOURCES = videocapture_basic.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

push: $(TARGET)
	adb push $(TARGET) /root

all-push: all push

.PHONY: all clean push all-push

