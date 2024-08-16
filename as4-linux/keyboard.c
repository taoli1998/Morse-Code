#include "keyboard.h"
#include "pru.h"
#include "LEDmatrix.h"
#include "function.h"
#include "morsecode.h"
#include "sharedDataStruct.h"
#include "pru.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h> 
#include <ctype.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define space 7
#define letter 3
static bool stop = 0;
volatile sharedMemStruct_t *pSharedPru0; 
static pthread_t pruThreadId;
static char morse_code[max_dot];

static void* typing(void* args);
static void trim_whitespace(char buff[], int length, uint32_t *pl);
static void get_morse_code(char buff[], bool data[]);
static void print_and_display(char buff[], int numCh);

void keyboard_init(volatile void* pPruBase){
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&pruThreadId, &attr, typing,NULL);
}

void keyboard_stop(void){
    stop = 1;  
    pSharedPru0->isLedOn = 0;
    
    pthread_join(pruThreadId,NULL);
}

static void* typing(void* args){
    while(stop==0){
        printf("> ");
        char *buff = NULL;
        size_t sizeAllocated = 0;
        size_t numCh = getline(&buff, &sizeAllocated, stdin);
        int length = numCh - 1;
        trim_whitespace(buff, length, &numCh);
        if(numCh==0){
            free(buff);
            break;
        }       
        get_morse_code(buff, (bool*)pSharedPru0->data);
        print_and_display(buff,numCh);
        free(buff);
        buff = NULL; 
    }
    set_shut();
    return NULL;    
}

static void print_and_display(char buff[], int numCh){
    int32_t prev_dot = -1;      
    pSharedPru0->curr_dot = -1;
    pSharedPru0->isLedOn = 1;
    pSharedPru0->isBottonPress = 1; 
    printf("Flashing out %d characters: '%s'\n",numCh,buff);
    for (int i=0; i<pSharedPru0->num_dot_time;i++){
        morse_code[i] = (pSharedPru0->data[i]) ? 'X':'_';
        printf("%c",morse_code[i]);
    }               
    printf("\n"); 
    while(pSharedPru0->isLedOn){     
        if(prev_dot != pSharedPru0->curr_dot){\
            char dot = (pSharedPru0->data[pSharedPru0->curr_dot])? 'X':'_';                
            printf("%c",dot);
            fflush(stdout);
            prev_dot = pSharedPru0->curr_dot;
        }
        int dot_time = pSharedPru0->num_dot_time-prev_dot;
        morse_display((bool*)&pSharedPru0->data[pSharedPru0->curr_dot],dot_time);
    }
    printf("\n");
    int_display(0);
}

static void trim_whitespace(char buff[], int length ,uint32_t *pl){
    for (int i = length; i>-1; i--){
        if(isspace(buff[i])){
            buff[i] = '\0';
            length--;
            (*pl)--;            
        }
        else{
            break;
        }
    }
}

static void get_morse_code(char buff[], bool data[]){
    uint32_t index = 0;
    for (int i = 0; buff[i]!=0; i++){
        unsigned short MorseCode = MorseCode_getFlashCode(buff[i]);
        if(index>0){
            int num_space = 0;
            if (MorseCode){
                num_space = letter;             
            }
            else{
                num_space = space;
            }
            for (int j = 0; j<num_space; j++){
                data[index] = 0;
                index++;
            }
        }
        while(MorseCode){
            data[index] = (MorseCode & 0x8000) !=0;
            MorseCode <<= 1;
            index++;
        }
    }
    pSharedPru0->num_dot_time = index;
}