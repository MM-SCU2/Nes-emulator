#include "stdint.h"


#define MAX_MEM 1024*64
typedef uint8_t  Byte;
typedef uint16_t  Word;

// ============
//  Structures
// ============

typedef struct NesMem Mem;
struct NesMem {
    Byte memory [MAX_MEM];
};

typedef struct NesCPU CPU;
struct NesCPU {

    Byte A;     // Accumulator register 
    Byte X;
    Byte Y;     // Index registers
    Word PC;    // Program counter
    Byte S;     // Stack pointer
    Byte P;     // Status register

    // Flags

    Byte C:1;   // Carry
    Byte Z:1;   // Zero
    Byte I:1;   // Interrup disable
    Byte D:1;   // Decimal 
    Byte N:1;   // Negative
    Byte V:1;   // Overflow

};
