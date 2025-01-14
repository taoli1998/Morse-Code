#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>

// WARNING:
// Fields in the struct must be aligned to match ARM's alignment
//    bool/char, uint8_t:   byte aligned
//    int/long,  uint32_t:  word (4 byte) aligned
//    double,    uint64_t:  dword (8 byte) aligned
// Add padding fields (char _p1) to pad out to alignment.

// My Shared Memory Structure
// ----------------------------------------------------------------

#define max_dot 2048

typedef struct {
    bool isLedOn;
    bool isBottonPress;
    uint8_t padding0;
    uint8_t padding1;
    uint32_t num_dot_time;
    uint32_t curr_dot;
    bool data[max_dot];
} sharedMemStruct_t;

#endif
