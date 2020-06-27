#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE "/dev/stopwatch"

extern int first(int x,int y);
static int dev;
int first(int x, int y)
{
    if(x == 0){
        oopen();
    }
    else if(x == 1){
        wwrite();
    }
    else if(x == 2){
        cclose();
    }
	return x + y;
}

void oopen(){
    int dev = open(DEVICE, O_WRONLY);
}

void wwrite(){
    char data = 0;
    write(dev, &data, sizeof(data));
}

void cclose(){
    close(dev);
}
