# Kernel Memory Monitor

## Features
- Tracks process memory usage
- Soft limit warning
- Hard limit process kill

## Build
make

## Run
sudo insmod monitor.ko
sudo mknod /dev/container_monitor c <major> 0
sudo ./engine

## Test
./workload1
./workload2