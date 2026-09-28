#!/usr/bin/env bash
# logger.sh

OUTPUT="mem_log.csv"
echo "time_sec,mem_available_mb" > "$OUTPUT"

START_TIME=$(date +%s%N)

while true; do
    CURR_TIME=$(date +%s%N)
    ELAPSED=$(awk "BEGIN {print ($CURR_TIME - $START_TIME) / 1000000000}")
    
    # Считываем MemAvailable в Килобайтах и переводим в МБ
    MEM_AVAIL_KB=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
    MEM_AVAIL_MB=$(awk "BEGIN {print $MEM_AVAIL_KB / 1024}")

    echo "$ELAPSED,$MEM_AVAIL_MB" >> "$OUTPUT"
    sleep 0.1
done