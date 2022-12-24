#include "bus.h"

Byte read_byte(Mem* mem, Word address) {
    return mem->memory[address];
}

Word read_word(Mem* mem, Word address) {
    Byte low = mem->memory[address];
    Byte high = mem->memory[address+1];
    return (high<<8) | low;
}

void write_byte(Mem *mem, Word address, Byte data) {
    mem->memory[address] = data;
}

void write_word(Mem *mem, Word address, Word data) {
    //split data 
    Byte high = (data & 0x1100) >> 8;
    Byte low  = data & 0x0011;
    mem->memory[address] = low;
    mem->memory[address+1] = high;
}


Byte fetchByte(CPU* cpu, Mem* mem) {
    // assert 0 <=  cpu.PC < Max_Mem
    Byte data = (Byte) mem->memory[cpu->PC];
    cpu->PC++;
    return data;
}

Word fetchWord(CPU* cpu, Mem* mem) {
    Byte low_data =  mem->memory[cpu->PC];
    Byte high_data = mem->memory[cpu->PC+1] ;
    cpu->PC+=2;;

    // 6502 is little endian 
    // less significant bytes are in the less significant addresses
    return (high_data << 8) | low_data;
}
