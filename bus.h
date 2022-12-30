#ifndef BUS_H
#define BUS_H
#include "devices.h"

// =================
//  function headers
// ==================

Byte read_byte(Mem* mem, Word address);
Word read_word(Mem* mem, Word address);
void write_byte(Mem* mem, Word address, Byte data);
void write_word(Mem* mem, Word address, Word data);
Byte fetchByte(CPU* cpu, Mem* mem);
Word fetchWord(CPU* cpu, Mem* mem);
Byte fetch_zp(Mem* mem, Word address);

#endif /* BUS_H */