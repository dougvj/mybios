#!/bin/bash
set +e
TERMINAL=alacritty

qemu-system-i386  \
  -bios ./bios.bin \
  -m 32 \
  -cpu pentium \
  -vga std \
  -machine pc \
  -monitor telnet:127.0.0.1:55555,server \
  -S \
  -gdb tcp::9000 \
  -accel tcg \
  -singlestep \
  -no-reboot \
  -no-shutdown \
  -D qemu.log \
  -d int,cpu_reset,pcall,guest_errors,unimp \
  -serial stdio &

PID=$!
echo $PID > qemu.pid
sleep 1
if [ -f "qemu.pid" ]; then
  echo "QEMU is running"
else
  echo "QEMU is not running"
  exit 1
fi

$TERMINAL -e telnet localhost 55555 &
TELNET_PID=$!
$TERMINAL -e gdb ./bios.elf \
  -ex 'target remote localhost:9000' \
  -ex 'tui enable' \
  -ex 'layout asm'
GDB_PID=$!

tail -f qemu.log &
LOG_PID=$!

wait $PID
kill $GDB_PID
kill $TELNET_PID
kill $LOG_PID
echo "QEMU has exited"
