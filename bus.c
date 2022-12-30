#include "bus.h"
// =====================================================================================
//                            MEMORY ACCESS FUNCTIONS 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// considerations:
//  * 6502 is little endian.less significant bytes are in the less significant addresses
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// =====================================================================================

Byte fetch_zp(Mem* mem, Word address) {
    Word zp_address = address % 0x00ff;
    return read_byte(mem, zp_address);
}


Byte read_byte(Mem* mem, Word address) {
    if(address >= 0x0000 && address <= 0xFFFF) 
        return mem->memory[address];
    else 
        return -1;
}

Word read_word(Mem* mem, Word address) {
    if(address >= 0x0000 && address <= 0xFFFF){ 
        Byte low = mem->memory[address];
        Byte high = mem->memory[address+1];
        return (high<<8) | low;
    }
    else return -1;
}

void write_byte(Mem *mem, Word address, Byte data) {
    if(address >= 0x0000 && address <= 0xFFFF) 
        mem->memory[address] = data;
}

void write_word(Mem *mem, Word address, Word data) {
    if(address >= 0x0000 && address <= 0xFFFF){
        //split data 
        Byte high = (data & 0x1100) >> 8;
        Byte low  = data & 0x0011;
        mem->memory[address] = low;
        mem->memory[address+1] = high;
    }
}

Byte fetchByte(CPU* cpu, Mem* mem) {
    Byte data = read_byte(mem, cpu->PC);
    cpu->PC++;
    return data;
}

Word fetchWord(CPU* cpu, Mem* mem) {
    Word data = read_word(mem, cpu->PC);
    cpu->PC+=2;;
    return data;
}
