#!/bin/sh

export TZ=CST-8

if [ -f /etc/profile ] && ! grep -q "TZ=CST-8" /etc/profile 2>/dev/null; then
    echo 'export TZ=CST-8' >> /etc/profile && \
    echo "Timezone added to /etc/profile"
fi

wait_for_network() {
    local max_attempts=30
    local attempt=0
    
    echo "Waiting for network to be ready..."
    
    while [ $attempt -lt $max_attempts ]; do
        if ip link show wlan0 | grep -q "state UP" 2>/dev/null; then
            if ip addr show wlan0 | grep -q "inet " 2>/dev/null; then
                if ping -c 1 -W 2 8.8.8.8 >/dev/null 2>&1 || \
                   ping -c 1 -W 2 114.114.114.114 >/dev/null 2>&1; then
                    echo "Network is ready"
                    return 0
                fi
            fi
        fi
        
        attempt=$((attempt + 1))
        sleep 1
    done
    
    echo "Warning: Network may not be ready, but proceeding with time sync..."
    return 1
}

wait_for_network

echo "Synchronizing time with time.nist.gov..."
/root/ntpclient -s -h time.nist.gov

if [ $? -eq 0 ]; then
    echo "Time synchronization successful"
    date
else
    echo "Time synchronization failed"
    echo "Trying alternative time server..."
    /root/ntpclient -s -h pool.ntp.org 2>/dev/null || \
    /root/ntpclient -s -h cn.pool.ntp.org 2>/dev/null || \
    /root/ntpclient -s -h time.windows.com 2>/dev/null
fi

