#!/bin/bash
PIPE_PATH=""
for arg in "$@"; do
    if [[ $arg == --pishitesuda=* ]]; then
        PIPE_PATH="${arg#*=}"
    fi
done
if [ -z "$PIPE_PATH" ]; then exit 1; fi
rm -f "$PIPE_PATH"
mkfifo "$PIPE_PATH"
cleanup() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') Shutdown!" >> /root/practice3_part2/cuckoo.log
    rm -f "$PIPE_PATH"
    exit 0
}
trap cleanup SIGTERM SIGINT
echo "$(date '+%Y-%m-%d %H:%M:%S') Startup!" >> /root/practice3_part2/cuckoo.log
exec 3<> "$PIPE_PATH"
while true; do
    if read -r line <&3; then
        if [[ $line =~ (.*)\[([0-9]+)\]:\ how\ much\ time\ do\ I\ have\? ]]; then
            name="${BASH_REMATCH[1]}"
            pid="${BASH_REMATCH[2]}"
            n=$((RANDOM % 9 + 2))
            echo "$(date '+%Y-%m-%d %H:%M:%S') ${name}[${pid}] $n" >> /root/practice3_part2/cuckoo.log
            echo "$n" >&3
        fi
    fi
done
