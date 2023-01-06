#include "cpu.h"
#include "bus.h"
#include <stdio.h>
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

void flag_C(CPU* cpu, Byte bool){cpu->C = bool;}
void flag_Z(CPU* cpu, Byte bool) {cpu->Z = (bool == 0);}
void flag_N(CPU* cpu, Byte bool) {cpu->N = (bool >> 7) == 1;}
void flag_V(CPU* cpu, Byte bool) {cpu->V = bool;}

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

// =================
//  MEMORY ACCESSES
// =================

Byte zp(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    return fetch_zp(mem, zp_addr);
}

Byte zp_xy(CPU* cpu, Mem* mem, Byte offset) {
    Byte zp_addr = fetchByte(cpu, mem);
    return fetch_zp(mem, zp_addr + (Word)offset);
}

Byte abosolute(CPU* cpu, Mem* mem) {
    Word abs_addr = fetchWord(cpu, mem);
    return read_byte(mem, abs_addr);
}

Byte abosolute_xy(CPU* cpu, Mem* mem, Byte offset) {
    Word abs_addr = fetchWord(cpu, mem);
    return read_byte(mem, abs_addr + (Word)offset);
}

Word indirect(CPU* cpu, Mem* mem, Word indirect_addr) {
    Word addr = read_word(mem, indirect_addr);
    return addr; 
}

Word indirect_xy(CPU* cpu, Mem* mem, Word indirect_addr, Byte offset) {
    Word addr = read_word(mem, indirect_addr);
    return read_byte(mem, addr + (Word)offset); 
}

// ==============
//  ADC FUNCTION
// ==============

// the cast to word allows to easily check for carry bit
// and avoid data loose 

// Overflow flag is setted when the sign bit of the result
// differs from the accumulation value sign. Applying logic table
// we see that if we do the XOR between A and R sign bits then apply AND against
// the negation of A with M we obtain if we have to set V or not

void adc(CPU* cpu, Byte M) {
    Word result = (Word)cpu->A + (Word)M + (Word)cpu->C;
    Word AM = ((Word)cpu->A ^ (Word)M);
    Word AR = (Word)cpu->A ^ (Word)result;
    Byte V  = ((AR & ~AM) & 0x0080) >> 7;
    flag_V(cpu, V);
    flag_C(cpu, result > 255);
    flag_Z(cpu, (result & 0x00FF));
    flag_N(cpu, (result & 0x00FF));
    cpu->A = (result & 0x00FF);
}

void adc_imm(CPU* cpu, Mem* mem) {
    Byte data = fetchByte(cpu, mem);
    adc(cpu, data);
}

void adc_zp(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    Byte data = fetch_zp(mem, zp_addr);
    adc(cpu, data);
}

void adc_zp_x(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    Byte data = fetch_zp(mem, zp_addr + cpu->X);
    adc(cpu, data);
}

void adc_absolute(CPU* cpu, Mem* mem) {
    Word abs_addr = fetchWord(cpu, mem);
    Byte data = read_byte(mem, abs_addr);
    adc(cpu, data);
}

void adc_absolute_x(CPU* cpu, Mem* mem) {
    Word abs_addr = fetchWord(cpu, mem);
    Byte data = read_byte(mem, abs_addr + cpu->X);
    adc(cpu, data);
}

void adc_absolute_y(CPU* cpu, Mem* mem) {
    Word abs_addr = fetchWord(cpu, mem);
    Byte data = read_byte(mem, abs_addr + cpu->Y);
    adc(cpu, data);
}

void adc_indirect_x(CPU* cpu, Mem* mem) {
    Word indirect_addr = fetchWord(cpu, mem);
    Word addr =  indirect(cpu, mem, indirect_addr + cpu->X);
    Byte data = read_byte(mem, addr);
    adc(cpu, data);
}

void adc_indirect_y(CPU* cpu, Mem* mem) {
    Word indirect_addr = fetchWord(cpu, mem);
    Word addr =  indirect(cpu, mem, indirect_addr + cpu->Y);
    Byte data = read_byte(mem, addr);
    adc(cpu, data);
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

void asl(CPU* cpu, Mem* mem, Word addr, Byte data) {
    flag_C(cpu, (data >> 7));
    flag_N(cpu, data);
    flag_Z(cpu, data);
    write_byte(mem, addr, data << 1);
}


void asl_zp(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    Byte data = fetch_zp(mem, zp_addr);
    asl(cpu, mem, zp_addr, data);
}

void asl_zp_x(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    Byte data = fetch_zp(mem, zp_addr + cpu->X);
    asl(cpu,mem, zp_addr + cpu->X, data);
}

void asl_absolute(CPU* cpu, Mem* mem) {
    Word abs_addr = fetchWord(cpu, mem);
    Byte data = read_byte(mem, abs_addr);
    asl(cpu, mem, abs_addr, data);
}

void asl_absolute_x(CPU* cpu, Mem* mem) {
    Word abs_addr = fetchWord(cpu, mem);
    Byte data = read_byte(mem, abs_addr + cpu->X);
    asl(cpu, mem, abs_addr + cpu->X, data);
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

Byte ld_absolute(CPU* cpu, Mem* mem) {
    Word addr = fetchWord(cpu, mem);
    return read_byte(mem, addr);
}

Byte ldaxy_absolute(CPU* cpu, Mem* mem, Byte xy_offset) {
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

Word jmp_absolute(CPU* cpu, Mem* mem) {
    return fetchWord(cpu, mem);
}

Word jmp_indirect(CPU* cpu, Mem* mem) {
    Word indirect_addr = fetchWord(cpu, mem);
    return indirect(cpu, mem , indirect_addr);
}

// ================
//  Flags setting
// ================

void cli(CPU* cpu) {
    cpu->I = 0;
}

void sei(CPU* cpu) {
    cpu->I = 1;
}

void clc(CPU* cpu) {
    cpu->C = 0;
}

void sec(CPU* cpu) {
    cpu->C = 1;
}

void cld(CPU* cpu) {
    cpu->D = 0;
}

void sed(CPU* cpu) {
    cpu->D = 1;
}

void clv(CPU* cpu) {
    cpu->V = 0;
}


// ============================
//        Return function 
// ============================

void rts(CPU* cpu, Mem* mem) {
    pop_pc(cpu, mem);
}
// ============================
//  Transfer to memory functions
// ============================

// store accumulator

void sta_zp(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    write_byte(mem, zp_addr, cpu->A);
}

void sta_zp_x(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    write_byte(mem, zp_addr + cpu->X, cpu->A);
}

void sta_absolute(CPU* cpu, Mem* mem) {
    Word addr = fetchWord(cpu, mem);
    write_byte(mem, addr, cpu->A);
}

void sta_absolute_x(CPU* cpu, Mem* mem) {
    Word addr = fetchWord(cpu, mem);
    write_byte(mem, addr + cpu->X, cpu->A);
}

void sta_absolute_y(CPU* cpu, Mem* mem) {
    Word addr = fetchWord(cpu, mem);
    write_byte(mem, addr + cpu->Y, cpu->A);
}

void sta_indirect_x(CPU* cpu, Mem* mem) {
    // indirect address
    Word iaddr = fetchWord(cpu, mem);
    //real address to store data
    Word addr  =  read_word(mem, iaddr + cpu->X);
    write_byte(mem, addr, cpu->A);
}

void sta_indirect_y(CPU* cpu, Mem* mem) {
    // indirect address
    Word iaddr = fetchWord(cpu, mem);
    //real address to store data
    Word addr  =  read_word(mem, iaddr + cpu->Y);
    write_byte(mem, addr, cpu->A);
}

// store X

void stx_zp(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    write_byte(mem, zp_addr, cpu->X);
}

void stx_zp_y(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    write_byte(mem, zp_addr + cpu->Y, cpu->X);
}

void stx_absolute(CPU* cpu, Mem* mem) {
    Word addr = fetchWord(cpu, mem);
    write_byte(mem, addr, cpu->X);
}

// store Y

void sty_zp(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    write_byte(mem, zp_addr, cpu->Y);
}

void sty_zp_x(CPU* cpu, Mem* mem) {
    Byte zp_addr = fetchByte(cpu, mem);
    write_byte(mem, zp_addr + cpu->X, cpu->Y);
}

void sty_absolute(CPU* cpu, Mem* mem) {
    Word addr = fetchWord(cpu, mem);
    write_byte(mem, addr, cpu->Y);
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
            case ADC_IMM: {
                adc_imm(cpu, mem);
                clock(0.2);
                debug(*cpu);
            }
            break;
            case ADC_ZP: {
                adc_zp(cpu, mem);
                clock(0.3);
            }
            break;       
            case ADC_ZPX: {
                adc_zp_x(cpu, mem);
                clock(0.4);
            }
            break;         
            case ADC_ABS: {
                adc_absolute(cpu, mem);
                clock(0.4);
            }
            break;
            case ADC_ABSX: {
                adc_absolute_x(cpu, mem);
                clock(0.4);
            }
            break;
            case ADC_ABSY: {
                adc_absolute_y(cpu, mem);
                clock(0.4);
            }
            break;
            case ADC_INDIRECTX: {
                adc_indirect_x(cpu, mem);
                clock(0.4);
            }
            break;
            case ADC_INDIRECTY: {
                adc_indirect_y(cpu, mem);
                clock(0.4);
            }
            break;
            case ASL_ACC: {
                asl_acc(cpu, mem);
                clock(0.2);
            }
            break;
            case ASL_ZP: {
                asl_zp(cpu, mem);
                clock(0.5);
            }
            break;
            case ASL_ZPX: {
                asl_zp_x(cpu, mem);
                clock(0.6);
            }
            break;
            case ASL_ABS: {
                asl_absolute(cpu, mem);
                clock(0.6);
            }
            break;
            case ASL_ABSX: {                
                asl_absolute_x(cpu, mem);
                clock(0.6);
            }
            break;
            case CLC: {
                clc(cpu);
                clock(0.2);
            }
            break;
            case CLD: {
                cld(cpu);
                clock(0.2);
            }
            break;
            case CLI: {
                cli(cpu);
                clock(0.2);
            }
            break;
            case CLV: {
                clv(cpu);
                clock(0.2);
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
                cpu->A = ld_absolute(cpu, mem);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
            }
            break;
            case LDA_ABS_X: {
                cpu->A = ldaxy_absolute(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->A);
                flag_N(cpu, cpu->A);
                clock(0.4);
            }
            break;
            case LDA_ABS_Y: {
                cpu->A = ldaxy_absolute(cpu, mem, cpu->Y);
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
                cpu->X = ld_absolute(cpu, mem);
                flag_Z(cpu, cpu->X);
                flag_N(cpu, cpu->X);
                clock(0.4);
            }
            break;
            case LDX_ABS_Y: {
                cpu->X = ldaxy_absolute(cpu, mem, cpu->Y);
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
                cpu->Y = ld_absolute(cpu, mem);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.4);
            }
            break;
            case LDY_ABS_X: {
                cpu->Y = ldaxy_absolute(cpu, mem, cpu->X);
                flag_Z(cpu, cpu->Y);
                flag_N(cpu, cpu->Y);
                clock(0.4);
            }
            break;
            case JMP_ABS: {
                cpu->PC = jmp_absolute(cpu, mem);
                clock(0.3);
            }
            break;
            case JMP_INDIRECT: {
                cpu->PC = jmp_indirect(cpu, mem);
                clock(0.5);
            }
            case RTS: {
                rts(cpu, mem);
                clock(0.6);
            }
            case STA_ZP: {
                sta_zp(cpu, mem);
                clock(0.3);
            }
            break;
            case SEC: {
                sec(cpu);
                clock(0.2);
            }
            break;
            case SED: {
                sed(cpu);
                clock(0.2);
            }
            break;
            case SEI: {
                sei(cpu);
                clock(0.2);
            }
            case STA_ZPX: {
                sta_zp_x(cpu, mem);
                clock(0.4);
            }
            break;
            case STA_ABS: {
                sta_absolute(cpu, mem);
                clock(0.4);
            }
            break;
            case STA_ABSX: {
                sta_absolute_x(cpu, mem);
                clock(0.5);
            }
            break;
            case STA_ABSY: {
                sta_absolute_y(cpu, mem);
                clock(0.5);
            }
            break;
            case STA_INDIRECT_X: {
                sta_indirect_x(cpu, mem);
                clock(0.6);
            }
            break;
             case STA_INDIRECT_Y: {
                sta_indirect_y(cpu, mem);
                clock(0.6);
            }
            break;
            case STX_ZP: {
                stx_zp(cpu, mem);
                clock(0.3);
            }
            break;
            case STX_ZPY: {
                stx_zp_y(cpu, mem);
                clock(0.4);
            }
            break;
            case STX_ABS: {
                stx_absolute(cpu, mem);
                clock(0.4);
            }
            case STY_ZP: {
                sty_zp(cpu, mem);
                clock(0.3);
            }
            break;
            case STY_ZPX: {
                sty_zp_x(cpu, mem);
                clock(0.4);
            }
            break;
            case STY_ABS: {
                sty_absolute(cpu, mem);
                clock(0.4);
            }
            break;
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
                tya (cpu);
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

    // abc acc
    memory.memory[0] = 0x69;
    memory.memory[1] = 30;
    cpu.A = 240;

    // expected 
    // A = 70

    // N = 0
    // C = 0
    // Z = 1
    // V = 0




    // // asl zp 
    // memory.memory[1] = 0x06;
    // memory.memory[2] = 0x56;
    // memory.memory[0x56] = 1;

    // // asl zpx 
    
    // memory.memory[3] = 0x16;
    // memory.memory[4] = 0x58;
    // memory.memory[0x59] = 2;

    // // asl abs 
    // memory.memory[5] = 0x0E;
    // memory.memory[6] = 0x56;
    // memory.memory[7] = 0x56;
    // memory.memory[0x5656] = 3;

    // // asl absx
    // memory.memory[8] = 0x1E;
    // memory.memory[9] = 0x57;
    // memory.memory[10] = 0x57;
    // memory.memory[0x5758] = 8;

    execute(&cpu, &memory);

    return 0;
}