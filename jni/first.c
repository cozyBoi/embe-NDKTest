#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>
#include "android/log.h"

#define LOG_TAG "MyTag"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

#include MAX_BUTTON 9

int first() {
    unsigned char push_sw_buff[MAX_BUTTON];
    
    dev = open("/dev/fpga_push_switch", O_RDWR);
    if (dev<0) {
        printf("Device Open Error\n");
        close(dev);
        return -1;
    }
    
    buff_size = sizeof(push_sw_buff);
    
    read(dev, &push_sw_buff, buff_size);
    
    int i = 0;
    for(i = 0; i < 9; i++){
        if(push_sw_buff[i] == 1){
            push_sw_buff[i] = 0;
            LOGV("butt : %d\n", i);
            return i;
        }
    }
    
    return 9;
}
