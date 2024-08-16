#include "LEDmatrix.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "function.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"
#define I2C_DEVICE_ADDRESS 0x70

static char Logical_frame[8];
static char Physical_frame[8];
typedef struct{
    char c;
    char col;
    unsigned char row[8];
} digit;

static digit number[];
static digit* getnum(char c);
static unsigned char Shift_Operation(unsigned char c, int n);
static int getLogicalFrame(char current, int column);
static void getPhysicalFrame();
static void I2Cwrite();
static int defrow[]={
    0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E
};

void Matrix_start(void){
    runCommand("config-pin P9_18 i2c");
    runCommand("config-pin P9_17 i2c");
    int filedec = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    writeI2cReg(filedec,0x21,0x00);
    writeI2cReg(filedec,0x81,0x00);
}

void int_display(int num){
    if(num<0){
        num = 0;
    }
    else if(num>99){
        num = 99;
    }
    char buff[10];
    snprintf(buff,10,"%2d",num);
    Convert_logical(buff);
}

void dou_display(double num){
    if(num<0){
        num = 0;
    }
    else if(num>99){
        num = 99;
    }
    char buff[10];
    snprintf(buff,10,"%f",num);
    Convert_logical(buff);
}

void char_display(char c){

    Convert_logical(&c);
}

static digit number[] = {
    //{'0', 4, {0x20, 0x50, 0x50, 0x50,  0x50, 0x50, 0x20, 0x00}},
    {'0', 4, {0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00}},
    {'1', 4, {0x20, 0x30, 0x20, 0x20,  0x20, 0x20, 0x70, 0x00}},
    {'2', 4, {0x20, 0x50, 0x40, 0x20,  0x20, 0x10, 0x70, 0x00}},
    {'3', 4, {0x30, 0x40, 0x40, 0x70,  0x40, 0x40, 0x30, 0x00}},
    {'4', 4, {0x40, 0x60, 0x50, 0x50,  0x70, 0x40, 0x40, 0x00}},
    {'5', 4, {0x70, 0x10, 0x10, 0x70,  0x40, 0x50, 0x20, 0x00}},
    {'6', 4, {0x20, 0x10, 0x10, 0x30,  0x50, 0x50, 0x20, 0x00}},
    {'7', 4, {0x70, 0x40, 0x40, 0x20,  0x20, 0x20, 0x20, 0x00}},
    {'8', 4, {0x20, 0x50, 0x50, 0x20,  0x50, 0x50, 0x20, 0x00}},
    {'9', 4, {0x20, 0x50, 0x50, 0x20,  0x40, 0x40, 0x20, 0x00}},
    {'M', 4, {0x50, 0x70, 0x50, 0x50,  0x50, 0x50, 0x50, 0x00}},
    {'.', 1, {0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x40}},
    {'X', 1, {0x00, 0x00, 0x00, 0x00,  0x50, 0x50, 0x50, 0x50}},
    {' ', 4, {0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00}}
};

static digit* getnum(char c){
    for(unsigned i=0;number[i].c !=0;i++){
        if(number[i].c == c){
            return &number[i];
        }
    }
    return NULL;
}

void Convert_logical(char* input){
    memset(Logical_frame,0,sizeof(char)*8);
    int column = 0;
    char* num_string = input;
    while(column<8){
        char current = number[11].c;
        if((num_string !=NULL)==true){
            current = *num_string;
            num_string++;
        }
        column = getLogicalFrame(current, column);
    }
    getPhysicalFrame();
}

static int getLogicalFrame(char current, int column){
    digit* info = getnum(current);
    unsigned char* row_num = info->row;
    int col_num = info->col;
    for (int i=0; i<8; i++){
            unsigned char rows = Shift_Operation(row_num[i],(8-col_num-column));
            Logical_frame[i] = Logical_frame[i] | rows;
    }
    if(col_num!=1){
        column += col_num;
    }
    return column;
}


// static void I2Cwrite1(){
//     int filedec1 = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
//     for (int i = 0; i<8;i++){
//         writeI2cReg(filedec1,defrow[i],Physical_frame[i]);
//     }   
// }

static void getPhysicalFrame(){
    for(int i = 0; i<8; i++){
        Physical_frame[i] = ((Logical_frame[i]>>1)|(Logical_frame[i]<<7));
    }
    I2Cwrite();
}

static unsigned char Shift_Operation(unsigned char c, int bit){
    if (bit>=0){
        return c >> bit;
    }
    else{
        return c << (-1*bit);
    }
}

void print_frame(void){
    for (int i=0; i<8; i++){
        printf("%d\n",Logical_frame[i]);
    }
}
void morse_display(bool *data, int num){
    memset(Logical_frame,0,sizeof(char)*8);
    int cols;
    if(num>8){
        cols = 8;
    }
    else{
        cols = num;
    }
    for(int i=0; i<cols;i++){
        if(data[i]){
            for (int row = 4; row<8; row++){
                Logical_frame[row] |= (1<<(7-i));
            }
        }
    }
    getPhysicalFrame();
}

static void I2Cwrite(){
    runCommand("config-pin P9_18 i2c");
    runCommand("config-pin P9_17 i2c");
    int filedec = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    writeI2cReg(filedec,0x21,0x00);
    writeI2cReg(filedec,0x81,0x00);
    for (int i = 0; i<8;i++){
        writeI2cReg(filedec,defrow[i],Physical_frame[i]);
    }    
}




