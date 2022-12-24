#include "cpu.h"
#include<unistd.h>

// ============
//  Functions 
// ============

void reset(CPU cpu, Mem mem) {
    // init cpu state
    cpu.A = cpu.X = cpu.Y = cpu.P = 0;
    cpu.S = 0xFF; // if setted on 0x01ff it would overflow
    cpu.PC = 0;
     
    // Init memory
    for(int i=0; i< MAX_MEM; i++) {
        mem.memory[i] = 0; 
    }
}

// ====================
//  Context Functions
// ==================== 

void flag_Z(CPU* cpu, Byte reg) {cpu->Z = (reg == 0);}
void flag_N(CPU* cpu, Byte reg) {cpu->N = (reg >> 7) == 1;}

void debug(CPU cpu) {
    printf("--------------------------------------\n");
    printf("STATE OF CPU \n");
    printf("register A: %d \n", cpu.A);
    printf("register x: %d \n", cpu.X);
    printf("register Y: %d \n", cpu.Y);
    printf("PC: %d \n", cpu.PC);
    printf("register S: %d \n", cpu.S);
    printf("register P: %d \n\n", cpu.P);


    printf("STATE OF FLAGS \n");
    printf("C : %d \n", cpu.C);
    printf("Z : %d \n", cpu.Z);
    printf("V : %d \n", cpu.V);
    printf("I : %d \n", cpu.I);
    printf("D : %d \n", cpu.D);
    printf("N : %d \n", cpu.N);
    printf("--------------------------------------\n");
}

// ==================
//  CLOCK FUNCTIONS
// ==================


// await to execute next instrucction
void clock(double time) {
    //while (time >= 0) {
    sleep(time);
    //    time--;
    //}
}

// ==================
//  MEMORY FUNCTIONS
// ==================

void push(CPU* cpu, Mem* mem, Byte data) {
    Word address = (0x0001 << 8) | cpu->S;
    write_byte(mem, address, data);
    cpu->S--;
}

Byte pop(CPU* cpu, Mem* mem) {
    Word address = (0x0001 << 8) | (cpu->S+1);
    cpu->S++;
    return read_byte(mem, address);
}

void push_pc(CPU* cpu, Mem* mem) {
    Word address = (0x0001 << 8) | cpu->S;
    write_word(mem, address, cpu->PC);
    cpu->S-=2;
}

void pop_pc(CPU* cpu, Mem* mem) {
    Word address = (0x0001 << 8) | (cpu->S+1);
    Byte low = read_word(mem, address);
    cpu->S+=2;
}

Word indirect(CPU* cpu, Mem* mem, Word indirect_addr) {
    Word addr = read_word(mem, indirect_addr);
    return addr; 
}

// ================
//  LD FUNCTIONS
// ================

//* --------------------------------------------------------------------------------------------------------
//* --------------------------------------------------------------------------------------------------------
//* LDA LDX and LDY are very similar, Main differences are in the offset they use to 
//* form the addresses.
//* Functions named with _axy is because are the same for LDA, LDX and LDY but use different registers as offset 
//* if doesn't use registers and is the same for the three then the name doesn't have appendex 
//* --------------------------------------------------------------------------------------------------------
//* --------------------------------------------------------------------------------------------------------

Byte ld_imm(CPU* cpu, Mem* mem) {
    Byte imm = fetchByte(cpu, mem);
    return imm;
}

Byte ld_zp(CPU* cpu, Mem* mem) {
    Byte zero_addr = fetchByte(cpu, mem);
    Byte data = read_byte(mem, zero_addr);
    return data;
}

Byte ldxy_zp(CPU* cpu, Mem* mem, Byte xy_offset) {
    Byte zero_addr = fetchByte(cpu, mem);
    Byte z_addr  =  zero_addr + xy_offset;
    z_addr %= 0xff;
    Byte data = read_byte(mem, z_addr);
    return data;
}

Byte ld_abs(CPU* cpu, Mem* mem) {
    Word addr = fetchWord(cpu, mem);
    Byte data  = read_byte(mem, addr);
    return data;
}

Byte ldaxy_abs(CPU* cpu, Mem* mem, Byte xy_offset) {
    Word addr = fetchWord(cpu, mem);
    Byte data  = read_byte(mem, addr+xy_offset);
    return data;
}

Byte ldxy_indirect(CPU* cpu, Mem* mem, Byte xy_offset) {
    Word indirect_addr = fetchWord(cpu, mem);
    Word addr =  indirect(cpu, mem, indirect_addr+xy_offset);
    Byte data = read_byte(mem, addr);
    return data;
}

//* ---------------------
//*   JMP instructions
//* ---------------------

Word jmp_abs(CPU* cpu, Mem* mem) {
    return fetchWord(cpu, mem);
}

Word jmp_indirect(CPU* cpu, Mem* mem) {
    Word indirect_addr = fetchWord(cpu, mem);
    Word addr = indirect(cpu, mem , indirect_addr);
    return addr;
}

//* -------------------
//*     MAIN LOOP
//* -------------------

void execute(CPU* cpu, Mem* mem) {
    while (1) {
        Byte ins = fetchByte(cpu, mem);

        switch (ins) {
            case LDA_IMM: {
                cpu->A = ld_imm(cpu, mem);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.2);
                debug(*cpu);
            }
            break;
            case LDA_Z_PAG: {
                cpu->A = ld_zp(cpu, mem);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.3);
                debug(*cpu);
            }
            break;
            case LDA_Z_PAG_X: {
                cpu->X = 0x5; // Todo: delete after testing

                cpu->A = ldxy_zp(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case LDA_ABS: {
                cpu->A = ld_abs(cpu, mem);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case LDA_ABS_X: {
                cpu->X = 0; // Todo: delete after testing

                cpu->A = ldaxy_abs(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case LDA_ABS_Y: {
                cpu->A = ldaxy_abs(cpu, mem, cpu->Y);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case LDA_INDIRECT_X: {
                cpu->X = 0x1; // Todo: delete after testing

                cpu->A = ldxy_indirect(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.6);
                debug(*cpu);
            }
            break;
            case LDA_INDIRECT_Y: {
                cpu->Y = 0x2; // Todo: delete after testing

                cpu->A = ldxy_indirect(cpu, mem, cpu->Y);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.6);
                debug(*cpu);
            } 
            break;
            case LDX_IMM: {
                cpu->X = ld_imm(cpu, mem);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.2);
                debug(*cpu);
            }
            break;
            case LDX_Z_PAG: {
                cpu->X = ld_zp(cpu, mem);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.3);
                debug(*cpu);
            }
            break;
            case LDX_PAG_Y: {
                cpu->Y = 0x5; // Todo: delete after testing

                cpu->A = ldxy_zp(cpu, mem, cpu->Y);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case LDX_ABS: {
                cpu->X = ld_abs(cpu, mem);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case LDX_ABS_Y: {
                cpu->X = ldaxy_abs(cpu, mem, cpu->Y);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.4);
                debug(*cpu);
            }
            break;
             case LDY_IMM: {
                cpu->Y = ld_imm(cpu, mem);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.2);
                debug(*cpu);
            }
            break;
            case LDY_Z_PAG: {
                cpu->Y = ld_zp(cpu, mem);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.3);
                debug(*cpu);
            }

            break;
            case LDY_PAG_X: {
                cpu->X = 0x5; // Todo: delete after testing

                cpu->Y = ldxy_zp(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case LDY_ABS: {
                cpu->Y = ld_abs(cpu, mem);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case LDY_ABS_X: {
                cpu->Y = ldaxy_abs(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.4);
                debug(*cpu);
            }
            break;
            case JMP_ABS: {
                cpu->PC = jmp_abs(cpu, mem);
                clock(0.3);
                debug(*cpu);
            }
            break;
            case JMP_INDIRECT: {
                cpu->PC = jmp_indirect(cpu, mem);
                clock(0.5);
                debug(*cpu);
            }
            break;
            default: {
                printf("UNKNOWN OPCODE \n");
            }
        }
        if (ins == 0x0) {
            break;
        }
    }
}

int main() {

    CPU cpu;
    Mem memory;

    // N flag test
    reset(cpu, memory);
    memory.memory[0] = 0xA9;
    memory.memory[1] = -2;

    // pag_z test
    memory.memory[2] = 0xA5;
    memory.memory[3] = 0xf5;
    memory.memory[0xf5] = 5;

    // pag z with x offset
    memory.memory[4] = 0xb5;
    memory.memory[5] = 0x55;
    memory.memory[0x5A] = 6;

   // lda abs x
    memory.memory[6] = 0xbd;
    memory.memory[7] = 0x10;
    memory.memory[8] = 0x11;
    memory.memory[0x1110] = -7;

    // lda indirect x
    memory.memory[9] = 0xA1;
    memory.memory[10] = 0x11;
    memory.memory[11] = 0x11;
    memory.memory[0x1112] = 0x11;
    memory.memory[0x1113] = 0x12;
    memory.memory[0x1211] = 8;

    // lda indirect y
    memory.memory[12] = 0xb1;
    memory.memory[13] = 0x44;
    memory.memory[14] = 0x44;
    memory.memory[0x4446] = 0x20;
    memory.memory[0x4447] = 0x20;
    memory.memory[0x2020] = 1;

    execute(&cpu, &memory);

    return 0;
}