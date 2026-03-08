CC = arm-openwrt-linux-gcc
CFLAGS = -Wall -Wextra -O2 -g -pthread -I /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/include/
LDFLAGS = -pthread -L /home/zhiwan/t113/Tina-Linux/out/t113-zhiwan_v1/staging_dir/target/usr/lib/ -lasound

TARGET = audio_system
SOURCES = main.c ring_buffer.c audio_record.c audio_playback.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

push: $(TARGET)
	adb push $(TARGET) /root

all-push: all push

.PHONY: all clean push all-push









