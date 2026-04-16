#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "monitor_ioctl.h"

int main() {
    int fd = open("/dev/container_monitor", O_RDWR);
    int data[3];

    printf("Enter PID: ");
    scanf("%d", &data[0]);

    printf("Enter Soft Limit: ");
    scanf("%d", &data[1]);

    printf("Enter Hard Limit: ");
    scanf("%d", &data[2]);

    ioctl(fd, IOCTL_ADD_PROCESS, data);

    printf("Process registered successfully\n");
    close(fd);
    return 0;
}