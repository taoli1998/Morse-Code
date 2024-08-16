#include "LEDmatrix.h"
#include "function.h"
#include "morsecode.h"
#include "keyboard.h"
#include "pru.h"
#include "sharedDataStruct.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


static long long RandomNumber(int lower, int upper);

void run(void){
    printf("Morse code starting...\n");
    pru_init();
    volatile void* pPruBase = getPruMmapAddr();  
    Matrix_start();
    keyboard_init(pPruBase);

    check_null_enter();

    printf("Shutting down...\n");
    keyboard_stop();    
    freePruMmapAddr(pPruBase);
    pru_stop();
    printf("done!\n");
}
void terminate(void){

}

static int shutdown = 0;
void check_null_enter(void){
	while(shutdown == 0) {
        sleepForMs(100);
    }
}
void set_shut(void){
    shutdown = 1;
}
long long getTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000
    + nanoSeconds / 1000000;
    return milliSeconds;
}

long long getTimeInNs(void){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec + seconds*1000*1000*1000;
    assert(nanoSeconds > 0);
    static long long lastTimeHack = 0;
    assert(nanoSeconds > lastTimeHack);
    lastTimeHack = nanoSeconds;
    return nanoSeconds;
}

void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

static long long RandomNumber(int lower, int upper) 
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

void sleep_random_time(){
    int lower = 500, upper = 1500;
    srand(time(0));
    long long randtim = RandomNumber(lower, upper);
    sleepForMs(randtim);
}


void runCommand(char* command)
{
    FILE *pipe = popen(command, "r");
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
    if (fgets(buffer, sizeof(buffer), pipe) == NULL)
    break;
    }
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
    perror("Unable to execute command:");
    printf(" command: %s\n", command);
    printf(" exit code: %d\n", exitCode);
 }
}

int readFromFileToScreen(char *fileName)
{
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", fileName);
        exit(-1);
    }
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);
    fclose(pFile);
    int value = atoi(buff);
    return value;
}

int getVoltage0Reading(char* file)
{
    // Open file
    FILE *f = fopen(file, "r");
    if (!f) {
        printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
        printf(" Check /boot/uEnv.txt for correct options.\n");
        exit(-1);
    }
    // Get reading
    int a2dReading = 0;
    int itemsRead = fscanf(f, "%d", &a2dReading);
    if (itemsRead <= 0) {
        printf("ERROR: Unable to read values from voltage input file.\n");
        exit(-1);
    }
    // Close file
    fclose(f);
    return a2dReading;
}

int initI2cBus(char* bus, int address)
{
    int i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFileDesc;
}

void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
    {
        unsigned char buff[2];
        buff[0] = regAddr;
        buff[1] = value;
        int res = write(i2cFileDesc, buff, 2);
        if (res != 2) {
        perror("I2C: Unable to write i2c register.");
        exit(1);
    }
}

unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
{
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr)) {
    perror("I2C: Unable to write to i2c register.");
    exit(1);
    }
    // Now read the value and return it
    char value = 0;
    res = read(i2cFileDesc, &value, sizeof(value));
    if (res != sizeof(value)) {
    perror("I2C: Unable to read from i2c register");
    exit(1);
    }
    return value;
}

void print_all(long long AveI, long long MinI, long long MaxI, double AveV, double MinV, double MaxV, int dip, long long cur, long long last){
    printf("Intervel ms (%.3f, %.3f) avg=%.3f",MinI/1000000.0, MaxI/1000000.0, AveI/1000000.0);
    printf(" Sample V (%5.3f, %5.3f) avg=%5.3f", MinV, MaxV, AveV);
    printf(" #dip: %3d #Samples: %6lld\n", dip, cur - last);
}


