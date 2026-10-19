#!/bin/bash
cd /root/practice3_part2
SCRIPT_NAME=$(basename "$0")
if [ "$SCRIPT_NAME" == "template_task.sh" ]; then
echo "я бригадир, сам не работаю"
exit 0
fi
LOG_FILE="report_${SCRIPT_NAME}.log"
PID=$$
echo "$(date '+%Y-%m-%d %H:%M:%S') [$PID] Скрипт запущен" >> "$LOG_FILE"
CUCKOO_PID=$(pgrep -f "cuckoo.sh --pishitesuda=")
PIPE_PATH=$(cat /proc/$CUCKOO_PID/cmdline | tr '\0' '\n' | grep '^--pishitesuda=' | cut -d'=' -f2)
if [ -z "$PIPE_PATH" ]; then exit 1; fi
exec 3<> "$PIPE_PATH"
echo "${SCRIPT_NAME}[${PID}]: how much time do I have?" >&3
read -r N <&3
exec 3>&-
sleep "$N"
echo "$(date '+%Y-%m-%d %H:%M:%S') [$PID] Скрипт завершился, работал $N секунд." >> "$LOG_FILE"
