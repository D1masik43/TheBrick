#!/bin/bash

# Format: YY:MM:DD:HH:MM:SS (e.g., 26:03:22:23:15:00)
CURRENT_TIME=$(date +"%y:%m:%d:%H:%M:%S")
DEVICES=$(ls /dev/ttyACM* 2>/dev/null)

if [ -z "$DEVICES" ]; then
    echo "No ESP32-S3 found on /dev/ttyACM*"
    exit 1
fi

for DEV in $DEVICES; do
    echo "Syncing $CURRENT_TIME to $DEV..."

    stty -F "$DEV" 115200 raw -echo -echoe -echok

    (timeout 1 grep -q "RTC Updated!" "$DEV" && echo "ACK Received from $DEV") &
    LISTENER_PID=$!

    echo -e "$CURRENT_TIME\n" > "$DEV"

    wait $LISTENER_PID
    if [ $? -eq 0 ]; then
        echo "RTC Sync Complete."
    else
        echo "Sync Failed: No 'Pong' from $DEV"
    fi
done
