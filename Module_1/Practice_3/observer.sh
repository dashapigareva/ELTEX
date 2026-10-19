#!/bin/bash
cd /root/practice3_part2
while read -r script; do
[ -z "$script" ] && continue
RUNNING=0
for pid_dir in /proc/[0-9]*; do
if grep -a -q "$script" "$pid_dir/cmdline" 2>/dev/null; then
RUNNING=1
break
fi
done
if [ $RUNNING -eq 0 ]; then
nohup ./"$script" >/dev/null 2>&1 &
echo "$(date '+%Y-%m-%d %H:%M:%S') Перезапущен $script" >> observer.log
fi
done < observer.conf
