#include "cpu.h"
#include "bus.h"
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

void flag_C(CPU* cpu, Byte flag){cpu->C = flag;}
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
//  ASL FUNCTIONS
// ================

void asl_acc(CPU* cpu, Mem* mem) {
    flag_C(cpu, (cpu->A >> 7));
    flag_N(cpu, cpu->A);
    flag_Z(cpu, cpu->A);
    cpu->A = cpu->A << 1;
}

void asl_zp(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    Byte data = fetch_zp(mem, zp_addr);
    flag_C(cpu, (data >> 7));
    flag_N(cpu, data);
    flag_Z(cpu, data);
    write_byte(mem, zp_addr, data << 1);
}

void asl_zpx(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    Byte data = fetch_zp(mem, zp_addr + cpu->X);
    flag_C(cpu, (data >> 7));
    flag_N(cpu, data);
    flag_Z(cpu, data);
    write_byte(mem, zp_addr + cpu->X, data << 1);
}

void asl_abs(CPU* cpu, Mem* mem) {
    Word abs_addr = fetchWord(cpu, mem);
    Byte data = read_word(mem, abs_addr);
    flag_C(cpu, (data >> 7));
    flag_N(cpu, data);
    flag_Z(cpu, data);
    write_byte(mem, abs_addr, data << 1);
}

void asl_absx(CPU* cpu, Mem* mem) {
    Word abs_addr = fetchWord(cpu, mem);
    Byte data = read_word(mem, abs_addr + cpu->X);
    flag_C(cpu, (data >> 7));
    flag_N(cpu, data);
    flag_Z(cpu, data);
    write_byte(mem, abs_addr + cpu->X, data << 1);
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
    Byte zp_addr = fetchByte(cpu, mem);
    return fetch_zp(mem, zp_addr);
}

Byte ldxy_zp(CPU* cpu, Mem* mem, Byte xy_offset) {
    Byte zero_addr = fetchByte(cpu, mem);
    Byte zp_addr  =  zero_addr + xy_offset;
    return fetch_zp(mem, zp_addr);
}

Byte ld_abs(CPU* cpu, Mem* mem) {
    Word addr = fetchWord(cpu, mem);
    return read_byte(mem, addr);
}

Byte ldaxy_abs(CPU* cpu, Mem* mem, Byte xy_offset) {
    Word addr = fetchWord(cpu, mem);
    return read_byte(mem, addr+xy_offset);
}

Byte ldxy_indirect(CPU* cpu, Mem* mem, Byte xy_offset) {
    Word indirect_addr = fetchWord(cpu, mem);
    Word addr =  indirect(cpu, mem, indirect_addr+xy_offset);
    return read_byte(mem, addr);
}

//* ---------------------
//*   JMP instructions
//* ---------------------

Word jmp_abs(CPU* cpu, Mem* mem) {
    return fetchWord(cpu, mem);
}

Word jmp_indirect(CPU* cpu, Mem* mem) {
    Word indirect_addr = fetchWord(cpu, mem);
    return indirect(cpu, mem , indirect_addr);
}

// ===================
//  Transfer functions
// ===================

void tax(CPU* cpu) {
    cpu->X = cpu->A;
    flag_Z(cpu, cpu->X);
    flag_N(cpu, cpu->X);
}

void tay(CPU* cpu) {
    cpu->Y = cpu->A;
    flag_Z(cpu, cpu->Y);
    flag_N(cpu, cpu->Y);
}

void tsx(CPU* cpu) {
    cpu->X = cpu->S;
    flag_Z(cpu, cpu->X);
    flag_N(cpu, cpu->X);
}

void txa(CPU* cpu) {
    cpu->A = cpu->X;
    flag_Z(cpu, cpu->A);
    flag_N(cpu, cpu->A);
}

void txs(CPU* cpu) {
    cpu->S = cpu->X;
}

void tya(CPU* cpu) {
    cpu->A = cpu->Y;
    flag_Z(cpu, cpu->A);
    flag_N(cpu, cpu->A);
}

//* -------------------
//*     MAIN LOOP
//* -------------------

void execute(CPU* cpu, Mem* mem) {
    while (1) {
        Byte ins = fetchByte(cpu, mem);

        switch (ins) {
            case ASL_ACC: {
                asl_acc(cpu, mem);
                clock(0.2);
                printf("shifted value is acc: %d \n", cpu->A);
            }
            break;
            case ASL_ZP: {
                asl_zp(cpu, mem);
                clock(0.5);
                printf("shifted value is zp: %d \n", mem->memory[0x56]);
            }
            break;
            case ASL_ZPX: {
                cpu->X = 1; // TODO: delete after testing
                asl_zpx(cpu, mem);
                clock(0.6);
                printf("shifted value is zpx: %d \n", mem->memory[0x59]);
            }
            break;
            case ASL_ABS: {
                asl_abs(cpu, mem);
                clock(0.6);
                printf("shifted value is abs: %d \n", mem->memory[0x5656]);
            }
            break;
            case ASL_ABSX: {                
                cpu->X = 1; // TODO: delete after testing
                asl_absx(cpu, mem);
                clock(0.6);
                printf("shifted value is absx: %d \n", mem->memory[0x5758]);
            }
            break;
            case LDA_IMM: {
                cpu->A = ld_imm(cpu, mem);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.2);
            }
            break;
            case LDA_Z_PAG: {
                cpu->A = ld_zp(cpu, mem);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.3);
            }
            break;
            case LDA_Z_PAG_X: {
                cpu->A = ldxy_zp(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
            }
            break;
            case LDA_ABS: {
                cpu->A = ld_abs(cpu, mem);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
            }
            break;
            case LDA_ABS_X: {
                cpu->A = ldaxy_abs(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
            }
            break;
            case LDA_ABS_Y: {
                cpu->A = ldaxy_abs(cpu, mem, cpu->Y);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
            }
            break;
            case LDA_INDIRECT_X: {
                cpu->A = ldxy_indirect(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.6);
            }
            break;
            case LDA_INDIRECT_Y: {
                cpu->A = ldxy_indirect(cpu, mem, cpu->Y);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.6);
            } 
            break;
            case LDX_IMM: {
                cpu->X = ld_imm(cpu, mem);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.2);
            }
            break;
            case LDX_Z_PAG: {
                cpu->X = ld_zp(cpu, mem);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.3);
            }
            break;
            case LDX_PAG_Y: {
                cpu->A = ldxy_zp(cpu, mem, cpu->Y);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.4);
            }
            break;
            case LDX_ABS: {
                cpu->X = ld_abs(cpu, mem);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.4);
            }
            break;
            case LDX_ABS_Y: {
                cpu->X = ldaxy_abs(cpu, mem, cpu->Y);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.4);
            }
            break;
             case LDY_IMM: {
                cpu->Y = ld_imm(cpu, mem);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.2);
            }
            break;
            case LDY_Z_PAG: {
                cpu->Y = ld_zp(cpu, mem);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.3);
            }

            break;
            case LDY_PAG_X: {
                cpu->Y = ldxy_zp(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.4);
            }
            break;
            case LDY_ABS: {
                cpu->Y = ld_abs(cpu, mem);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.4);
            }
            break;
            case LDY_ABS_X: {
                cpu->Y = ldaxy_abs(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.4);
            }
            break;
            case JMP_ABS: {
                cpu->PC = jmp_abs(cpu, mem);
                clock(0.3);
            }
            break;
            case JMP_INDIRECT: {
                cpu->PC = jmp_indirect(cpu, mem);
                clock(0.5);
            }
            case TAX: {
                tax(cpu);
                clock(0.2);
            }
            break;
            case TAY: {
                tay(cpu);
                clock(0.2);
            }
            break;
            case TSX: {
                tsx(cpu);
                clock(0.2);
            }
            break;
            case TXA: {
                txa(cpu);
                clock(0.2);
            }
            break;
            case TXS: {
                txs(cpu);
                clock(0.2);
            }   
            break;
            case TYA: {
                tya(cpu);
                clock(0.2);
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

    reset(cpu, memory);

    // asl acc
    memory.memory[0] = 0x0A;
    
    // asl zp 
    memory.memory[1] = 0x06;
    memory.memory[2] = 0x56;
    memory.memory[0x56] = 1;

    // asl zpx 
    
    memory.memory[3] = 0x16;
    memory.memory[4] = 0x58;
    memory.memory[0x59] = 2;

    // asl abs 
    memory.memory[5] = 0x0E;
    memory.memory[6] = 0x56;
    memory.memory[7] = 0x56;
    memory.memory[0x5656] = 3;

    // asl absx
    memory.memory[8] = 0x1E;
    memory.memory[9] = 0x57;
    memory.memory[10] = 0x57;
    memory.memory[0x5758] = 8;

    execute(&cpu, &memory);

    return 0;
}