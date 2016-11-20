/*
 *  chip8.c
 *  chip8emu
 *
 *  Created by Oliver Markovic on 15.07.08.
 *  Copyright 2008 Oliver Markovic. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "chip8.h"
#include "font.h"

chip8_state_t cs;

void chip8_reset_cpu(chip8_cpu_t *cpu)
{
    assert(cpu);
    
    memset(cpu->dreg, 0, 16 * sizeof(u8));
    cpu->ireg = 0;
    memset(cpu->kreg, 0, 16 * sizeof(u8));
    cpu->delay_timer = 0;
    cpu->sound_timer = 0;
    cpu->pc = 0x200;
    cpu->sp = 0;
    memset(cpu->stack, 0, 16 * sizeof(u8));
}



void chip8_reset_state()
{
    memset(cs.mem, 0, 4096 * sizeof(u8));
    memset(cs.vram, 0, 64*32 * sizeof(u8));
    
    cs.draw_font = 0;
    
    // seed random number generator
    srand(17);
    
    chip8_reset_cpu(&cs.cpu);
}


int chip8_load_rom(const char *path)
{
    FILE *file;
    
    assert(path);
    
    if ((file = fopen(path, "rb")) == 0)
        return 0;
    
    fread(&cs.mem[0x200], sizeof(u8), 2048, file);
    
    fclose(file);
    
    return 1;
}


// keys

void chip8_key_event(chip8_keys_t key, u8 status)
{
    cs.cpu.kreg[key] = status;
}


// graphics

void chip8_clear_screen()
{
    memset(cs.vram, 0, 64*32 * sizeof(u8));
}

void chip8_draw_sprite(int sx, int sy, int sn)
{
    int ix, iy, vx, vy;    
    
    cs.cpu.dreg[15] = 0;
    
    for (iy = 0; iy < sn; iy++) {
        u8 s = cs.mem[cs.cpu.ireg + iy];
        
        for (ix = 0; ix < 8; ix++) {
            vx = (sx + ix) % 64;
            vy = (sy + iy) % 32;
    
            u8 oldval = cs.vram[vy * 64 + vx];
            u8 newval = s & (1 << (7-ix)) ? 1 : 0;
            
            // check for collision
            if (oldval && newval)
                cs.cpu.dreg[15] = 1;

            cs.vram[vy * 64 + vx] ^= newval;
        }
    }
}

void chip8_draw_font(int sx, int sy)
{
    int ix, iy, vx, vy, is;
    
    cs.cpu.dreg[15] = 0;
    
    is = 0;
    for (iy = 0; iy < 5; iy++) {
        for (ix = 0; ix < 4; ix++) {
            vx = (sx + ix) % 64;
            vy = (sy + iy) % 32;
            
            cs.vram[vy * 64 + vx] ^= chip8_font4x5[cs.cpu.ireg][is];
            
            is++;
        }
    }
    
}

u8 *chip8_get_vram()
{
    return cs.vram;
}

// instructions....


void chip8_instr_scdown(u16 opcode)
{
    printf("scdown\n");
    cs.cpu.pc += 2;
}

// clear screen
void chip8_instr_cls(u16 opcode)
{
    chip8_clear_screen();
    cs.cpu.pc += 2;
}

// return from subroutine
void chip8_instr_rts(u16 opcode)
{
    cs.cpu.pc = cs.cpu.stack[--cs.cpu.sp];
    cs.cpu.stack[cs.cpu.sp] = 0;
}

void chip8_instr_scright(u16 opcode)
{
    printf("scright\n");
    cs.cpu.pc += 2;
}

void chip8_instr_scleft(u16 opcode)
{
    printf("scleft\n");
    cs.cpu.pc += 2;
}

void chip8_instr_low(u16 opcode)
{
    printf("low\n");
    cs.cpu.pc += 2;
}

void chip8_instr_high(u16 opcode)
{
    printf("high\n");
    cs.cpu.pc += 2;
}

// jump to address
void chip8_instr_jmp(u16 opcode)
{
    cs.cpu.pc = (opcode & 0xFFF);
}

// jump to subroutine
void chip8_instr_jsr(u16 opcode)
{
    // save return address to stack
    cs.cpu.stack[cs.cpu.sp++] = cs.cpu.pc + 2;
    
    cs.cpu.pc = (opcode & 0xFFF);
}

// skip if register equals immediate
void chip8_instr_skeqi(u16 opcode)
{
    u8 r = (opcode & 0xF00) >> 8;
    u8 i = (opcode & 0xFF);
    
    if (cs.cpu.dreg[r] == i)
        cs.cpu.pc += 4;
    else
        cs.cpu.pc += 2;
}

// skip if register not equal immediate
void chip8_instr_sknei(u16 opcode)
{
    u8 r = (opcode & 0xF00) >> 8;
    u8 i = (opcode & 0xFF);
    
    if (cs.cpu.dreg[r] != i)
        cs.cpu.pc += 4;
    else
        cs.cpu.pc += 2;
}

// skip if register equals register
void chip8_instr_skeq(u16 opcode)
{
    u8 rx = (opcode & 0x0F00) >> 8;
    u8 ry = (opcode & 0x00F0) >> 4;
    
    if (cs.cpu.dreg[rx] == cs.cpu.dreg[ry])
        cs.cpu.pc += 4;
    else
        cs.cpu.pc += 2;
}

// move immediate into register
void chip8_instr_movi(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    u8 i = (opcode & 0xFF);
    
    cs.cpu.dreg[r] = i;
    cs.cpu.pc += 2;
}

// add immediate to register
void chip8_instr_addi(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    u8 i = (opcode & 0xFF);

    cs.cpu.dreg[r] += i;
    cs.cpu.pc += 2;
}

// move register to register
void chip8_instr_mov(u16 opcode)
{
    u8 rx = (opcode & 0x0F00) >> 8;
    u8 ry = (opcode & 0x00F0) >> 4;
    
    cs.cpu.dreg[rx] = cs.cpu.dreg[ry];
    cs.cpu.pc += 2;    
}

// or register into register
void chip8_instr_or(u16 opcode)
{
    u8 rx = (opcode & 0x0F00) >> 8;
    u8 ry = (opcode & 0x00F0) >> 4;
    
    cs.cpu.dreg[rx] |= cs.cpu.dreg[ry];
    cs.cpu.pc += 2;
}

// and register into register
void chip8_instr_and(u16 opcode)
{
    u8 rx = (opcode & 0x0F00) >> 8;
    u8 ry = (opcode & 0x00F0) >> 4;
    
    cs.cpu.dreg[rx] &= cs.cpu.dreg[ry];
    cs.cpu.pc += 2;
}

// xor register into register
void chip8_instr_xor(u16 opcode)
{
    u8 rx = (opcode & 0x0F00) >> 8;
    u8 ry = (opcode & 0x00F0) >> 4;
    
    cs.cpu.dreg[rx] ^= cs.cpu.dreg[ry];
    cs.cpu.pc += 2;
}

void chip8_instr_add(u16 opcode)
{
    u8 rx = (opcode & 0x0F00) >> 8;
    u8 ry = (opcode & 0x00F0) >> 4;

    // carry?
    if (cs.cpu.dreg[rx] + cs.cpu.dreg[ry] > 255)
        cs.cpu.dreg[15] = 1;
    else 
        cs.cpu.dreg[15] = 0;
    
    cs.cpu.dreg[rx] = cs.cpu.dreg[rx] + cs.cpu.dreg[ry];
    cs.cpu.pc += 2;
}

void chip8_instr_sub(u16 opcode)
{
    u8 rx = (opcode & 0x0F00) >> 8;
    u8 ry = (opcode & 0x00F0) >> 4;
    
    // carry?
    if (cs.cpu.dreg[rx] > cs.cpu.dreg[ry])
        cs.cpu.dreg[15] = 1;
    else
        cs.cpu.dreg[15] = 0;

    cs.cpu.dreg[rx] = cs.cpu.dreg[rx] - cs.cpu.dreg[ry];
    
    cs.cpu.pc += 2;    
}

// shift register right
void chip8_instr_shr(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    
    cs.cpu.dreg[15] = cs.cpu.dreg[r] & 0x1;
    cs.cpu.dreg[r] >>= 1;
    
    cs.cpu.pc += 2;
}

// subtract register from register
void chip8_instr_rsb(u16 opcode)
{
    u8 rx = (opcode & 0x0F00) >> 8;
    u8 ry = (opcode & 0x00F0) >> 4;
    
    // carry?
    if (cs.cpu.dreg[ry] > cs.cpu.dreg[rx])
        cs.cpu.dreg[15] = 1;
    else
        cs.cpu.dreg[15] = 0;
    
    cs.cpu.dreg[rx] = cs.cpu.dreg[ry] - cs.cpu.dreg[rx];
    cs.cpu.pc += 2;
}

// shift register left
void chip8_instr_shl(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    
    cs.cpu.dreg[15] = cs.cpu.dreg[r] >> 7;
    cs.cpu.dreg[r] <<= 1;
        
    cs.cpu.pc += 2;
}

// skip if register not equal register
void chip8_instr_skne(u16 opcode)
{
    int rx = (opcode & 0x0F00) >> 8;
    int ry = (opcode & 0x00F0) >> 4;
    
    if (cs.cpu.dreg[rx] != cs.cpu.dreg[ry])
        cs.cpu.pc += 4;
    else
        cs.cpu.pc += 2;
}

// load index register with immediate
void chip8_instr_mvi(u16 opcode)
{
    cs.draw_font = 0;
    cs.cpu.ireg = opcode & 0xFFF;
    cs.cpu.pc += 2;
}

void chip8_instr_jmi(u16 opcode)
{
    cs.cpu.pc = cs.cpu.dreg[0] + (opcode & 0xFFF);
}

void chip8_instr_rand(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    u8 rmax = opcode & 0xFF;
    
    cs.cpu.dreg[r] = ((float)rand() / (float)RAND_MAX) * rmax+1;
    cs.cpu.pc += 2;
}

// draw sprite
void chip8_instr_sprite(u16 opcode)
{
    u8 x = cs.cpu.dreg[(opcode & 0x0F00) >> 8];
    u8 y = cs.cpu.dreg[(opcode & 0x00F0) >> 4];
    u8 s = (opcode & 0x000F);
 
    if (cs.draw_font)
        chip8_draw_font(x, y);
    else
        chip8_draw_sprite(x, y, s);
    
    cs.cpu.pc += 2;
}

void chip8_instr_xsprite(u16 opcode)
{
    printf("xsprite\n");
    cs.cpu.pc += 2;
}

// skip if key pressed
void chip8_instr_skpr(u16 opcode)
{
    u8 k = (opcode & 0x0F00) >> 8;

    if (cs.cpu.kreg[cs.cpu.dreg[k]])
        cs.cpu.pc += 4;
    else
        cs.cpu.pc += 2;
}

// skip if key not pressed
void chip8_instr_skup(u16 opcode)
{
    u8 k = (opcode & 0x0F00) >> 8;
    
    if (!cs.cpu.kreg[cs.cpu.dreg[k]])
        cs.cpu.pc += 4;
    else
        cs.cpu.pc += 2;
}

// get delay timer into vr
void chip8_instr_gdelay(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;

    cs.cpu.dreg[r] = cs.cpu.delay_timer;
    cs.cpu.pc += 2;
}

void chip8_instr_key(u16 opcode)
{
    printf("key\n");
    cs.cpu.pc += 2;
}

// set delay timer to vr
void chip8_instr_sdelay(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    
    cs.cpu.delay_timer = cs.cpu.dreg[r];
    cs.cpu.pc += 2;
}

// set sound timer to vr
void chip8_instr_ssound(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    
    cs.cpu.sound_timer = cs.cpu.dreg[r];
    cs.cpu.pc += 2;
}

// add vr to index register
void chip8_instr_adi(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    
    cs.cpu.ireg += cs.cpu.dreg[r];
    cs.cpu.pc += 2;
}

// point index register to font
void chip8_instr_font(u16 opcode)
{
    u8 r = (opcode & 0x0F00) >> 8;
    
    cs.draw_font = 1;
    
    cs.cpu.ireg = cs.cpu.dreg[r];    
    cs.cpu.pc += 2;
}

void chip8_instr_xfont(u16 opcode)
{
    printf("xfont\n");
    cs.cpu.pc += 2;
}

// store binary coded decimal
void chip8_instr_bcd(u16 opcode)
{
    u8 r = cs.cpu.dreg[(opcode & 0x0F00) >> 8];
    u8 d0 = r % 10;
    u8 d1 = r / 10 % 10;
    u8 d2 = r / 100 % 100;
    
    cs.mem[cs.cpu.ireg] = d2;
    cs.mem[cs.cpu.ireg+1] = d1;
    cs.mem[cs.cpu.ireg+2] = d0;
    
    cs.cpu.pc += 2;
}

// store v0..vx into memory
void chip8_instr_str(u16 opcode)
{
    u8 rmax = (opcode & 0x0F00) >> 8;
    
    for (int i = 0; i < rmax; i++)
        cs.mem[cs.cpu.ireg+i] = cs.cpu.dreg[i];
    
    cs.cpu.pc += 2;
}

// load v0..vx from memory
void chip8_instr_ldr(u16 opcode)
{
    u8 rmax = (opcode & 0x0F00) >> 8;
    
    for (int i = 0; i < rmax; i++)
        cs.cpu.dreg[i] = cs.mem[cs.cpu.ireg+i];
    
    cs.cpu.pc += 2;
}

void chip8_instr_unknown(u16 opcode)
{
    printf("unknown %x\n", opcode);
    cs.cpu.pc += 2;
}

chip8_instruction_t istr_table[] = {
 {0x00C0, "scdown %d", P_3, &chip8_instr_scdown},
 {0x00E0, "cls", P_NONE, &chip8_instr_cls},
 {0x00EE, "rts", P_NONE, &chip8_instr_rts},
 {0x00FB, "scright", P_NONE, &chip8_instr_scright},
 {0x00FC, "scleft", P_NONE, &chip8_instr_scleft},
 {0x00FE, "low", P_NONE, &chip8_instr_low},
 {0x00FF, "high", P_NONE, &chip8_instr_high},
 {0x1000, "jmp %x", P_123, &chip8_instr_jmp},
 {0x2000, "jsr %x", P_123, &chip8_instr_jsr},
 {0x3000, "skeq v%x, %d", P_1_23, &chip8_instr_skeqi},
 {0x4000, "skne v%x, %d", P_1_23, &chip8_instr_sknei},
 {0x5000, "skeq v%x, v%x", P_1_2, &chip8_instr_skeq},
 {0x6000, "mov v%x, %d", P_1_23, &chip8_instr_movi},
 {0x7000, "add v%x, %d", P_1_23, &chip8_instr_addi},
 {0x8000, "mov v%x, v%x", P_1_2, &chip8_instr_mov},
 {0x8001, "or v%d, v%x", P_1_2, &chip8_instr_or},
 {0x8002, "and v%x, v%x", P_1_2, &chip8_instr_and},
 {0x8003, "xor v%x, v%x", P_1_2, &chip8_instr_xor},
 {0x8004, "add v%x, v%x", P_1_2, &chip8_instr_add},
 {0x8005, "sub v%x, v%x", P_1_2, &chip8_instr_sub},
 {0x8006, "shr v%x", P_1, &chip8_instr_shr},
 {0x8007, "rsb v%x, v%x", P_1_2, &chip8_instr_rsb},
 {0x800e, "shl v%x", P_1, &chip8_instr_shl},
 {0x9000, "skne v%x, v%x", P_1_2, &chip8_instr_skne},
 {0xa000, "mvi %x", P_123, &chip8_instr_mvi},
 {0xb000, "jmi %x", P_123, &chip8_instr_jmi},
 {0xc000, "rand v%x, %d", P_1_23, &chip8_instr_rand},
 {0xd000, "sprite %d, %d, %d", P_1_2_3, &chip8_instr_sprite},
 {0xd000, "xsprite %d, %d", P_1_2, &chip8_instr_xsprite},
 {0xe09e, "skpr %d", P_1, &chip8_instr_skpr},
 {0xe0a1, "skup %d", P_1, &chip8_instr_skup},
 {0xf007, "gdelay v%x", P_1, &chip8_instr_gdelay},
 {0xf00a, "key v%x", P_1, &chip8_instr_key},
 {0xf015, "sdelay v%x", P_1, &chip8_instr_sdelay},
 {0xf018, "ssound v%x", P_1, &chip8_instr_ssound},
 {0xf01e, "adi v%x", P_1, &chip8_instr_adi},
 {0xf029, "font v%x", P_1, &chip8_instr_font},
 {0xf030, "xfont v%x", P_1, &chip8_instr_xfont},
 {0xf033, "bcd v%x", P_1, &chip8_instr_bcd},
 {0xf055, "str v%x", P_1, &chip8_instr_str},
 {0xf065, "ldr v%x", P_1, &chip8_instr_ldr},
 {0xffff, "unknown", P_NONE, &chip8_instr_unknown}
};

int chip8_decode_instruction(u16 opcode)
{
    // b0b1b2b3
    int b0 = (opcode & 0xF000) >> 12;
    int b2 = (opcode & 0x00F0) >> 4;
    int b3 = (opcode & 0x000F);
    
    switch (b0) {
        case 0x0:
            switch (b2) {
                case 0xC: return I_SCDOWN;
                case 0xE:
                    switch (b3) {
                        case 0x0: return I_CLS;
                        case 0xE: return I_RTS;
                        default: return I_UNKNOWN;
                    }
                case 0xF:
                    switch (b3) {
                        case 0xB: return I_SCRIGHT;
                        case 0xC: return I_SCLEFT;
                        case 0xE: return I_LOW;
                        case 0xF: return I_HIGH;
                        default: return I_UNKNOWN;
                    }
                default: return I_UNKNOWN;
            }
        case 0x1: return I_JMP;
        case 0x2: return I_JSR;
        case 0x3: return I_SKEQI;
        case 0x4: return I_SKNEI;
        case 0x5: return I_SKEQ;
        case 0x6: return I_MOVI;
        case 0x7: return I_ADDI;
        case 0x8:
            switch (b3) {
                case 0x0: return I_MOV;
                case 0x1: return I_OR;
                case 0x2: return I_AND;
                case 0x3: return I_XOR;
                case 0x4: return I_ADD;
                case 0x5: return I_SUB;
                case 0x6: return I_SHR;
                case 0x7: return I_RSB;
                case 0xE: return I_SHL;
                default: return I_UNKNOWN;
            }
        case 0x9: return I_SKNE;
        case 0xA: return I_MVI;
        case 0xB: return I_JMI;
        case 0xC: return I_RAND;
        case 0xD:
            switch (b3) {
                case 0x0: return I_XSPRITE;
                default: return I_SPRITE;
            }
        case 0xE:
            if (b2 == 0x9 && b3 == 0xE) return I_SKPR;
            else if (b2 == 0xA && b3 == 0x1) return I_SKUP;
            else return I_UNKNOWN;
            case 0xF:
            switch (opcode & 0xFF) {
                case 0x07: return I_GDELAY;
                case 0x0A: return I_KEY;
                case 0x15: return I_SDELAY;
                case 0x18: return I_SSOUND;
                case 0x1E: return I_ADI;
                case 0x29: return I_FONT;
                case 0x30: return I_XFONT;
                case 0x33: return I_BCD;
                case 0x55: return I_STR;
                case 0x65: return I_LDR;
                default: return I_UNKNOWN;
            }
            default: return I_UNKNOWN;
    }
}

void chip8_disassemble_instruction(u16 opcode)
{
    int icode, b1, b2, b3;
    char *s;
    
    icode = chip8_decode_instruction(opcode);
    
    b1 = (opcode & 0x0F00) >> 8;
    b2 = (opcode & 0x00F0) >> 4;
    b3 = (opcode & 0x000F);
    
    s = istr_table[icode].mnemonic;
    
    switch (istr_table[icode].format) {
        case P_NONE: printf(s); break;
        case P_1: printf(s, b1); break;
        case P_2: printf(s, b2); break;
        case P_3: printf(s, b3); break;
        case P_1_2: printf(s, b1, b2); break;
        case P_1_2_3: printf(s, b1, b2, b3); break;
        case P_1_23: printf(s, b1, opcode & 0xFF); break;
        case P_123: printf(s, opcode & 0xFFF); break;
    }
    
    printf("\n");
}

void chip8_execute_step()
{
    u16 opcode, icode;

    // decrement timers if necessary
    if (cs.cpu.delay_timer > 0) cs.cpu.delay_timer--;
    if (cs.cpu.sound_timer > 0) cs.cpu.sound_timer--;
    
    // fetch instruction
    opcode = (cs.mem[cs.cpu.pc] << 8) | cs.mem[cs.cpu.pc+1];
    
    // decode instruction
    icode = chip8_decode_instruction(opcode);
    
    chip8_disassemble_instruction(opcode);
    
    // execute instruction
    void (*func)(int) = istr_table[icode].func;
    
    if (func)
        func(opcode);
}

