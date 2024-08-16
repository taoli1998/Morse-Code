#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "../as4-linux/sharedDataStruct.h"
#include "../as4-linux/function.h"

// Reference for shared RAM:
// https://markayoder.github.io/PRUCookbook/05blocks/blocks.html#_controlling_the_pwm_frequency

// GPIO Configuration
// ----------------------------------------------------------
volatile register uint32_t __R30;   // output GPIO register
volatile register uint32_t __R31;   // input GPIO register

// Use pru0_rpu_r30_5 as an output (bit 5 = 0b0010 0000)
#define LED_MASK (1 << 5)

// Use pru0_pru_r31_3 as a button (bit 3 = 0b0000 1000)
#define BUTTON_MASK (1 << 3)


// Shared Memory Configuration
// -----------------------------------------------------------
#define THIS_PRU_DRAM       0x00000         // Address of DRAM
#define OFFSET              0x200           // Skip 0x100 for Stack, 0x100 for Heap (from makefile)
#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)

// This works for both PRU0 and PRU1 as both map their own memory to 0x0000000
volatile sharedMemStruct_t *pSharedMemStruct = (volatile void *)THIS_PRU_DRAM_USABLE;

#define PROCESSOR_SPEED_MHZ 200000000
#define BUFFER_WINDOW_TIME_S 4
// Delay per sample is:
//  # instructions per second * seconds/window / samples / window
#define DELAY_PER_SAMPLE (PROCESSOR_SPEED_MHZ * BUFFER_WINDOW_TIME_S / NUM_SAMPLES)

#define DELAY_250_MS 50000000
#define DELAY_100_MS 20000000
static void reset(void);
static void delayIn100Ms(int msDelay);

void main(void){
    reset();   
    while(1){
        if(pSharedMemStruct->isLedOn){
            int i = 0;
            int num_dot = pSharedMemStruct->num_dot_time;
            for(i=0; i< num_dot;i++){
                pSharedMemStruct->curr_dot = i;
                if(pSharedMemStruct->data[i]){
                    __R30 |= LED_MASK;
                }
                else {
                    __R30 &= ~LED_MASK;
                } 
                if((__R31 & BUTTON_MASK) != 0){
                    delayIn100Ms(10);
                }
                else{
                    delayIn100Ms(3);
                }                                              
            }
            reset();         
        }
    }
}

static void delayIn100Ms(int msDelay){
    int i = 0;
    for (i = 0; i < msDelay; i++) {
        __delay_cycles(DELAY_100_MS);
    }
}

static void reset(void){
    __R30 &= ~LED_MASK;
    pSharedMemStruct->isLedOn = 0; 
}
