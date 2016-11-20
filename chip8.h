/*
 *  chip8.h
 *  chip8emu
 *
 *  Created by Oliver Markovic on 15.07.08.
 *  Copyright 2008 Oliver Markovic. All rights reserved.
 *
 */

#ifndef CHIP8_H
#define CHIP8_H

#include "types.h"


typedef struct {
    u8 dreg[16];
    u16 ireg;
    u8 kreg[16];
    u8 delay_timer;
    u8 sound_timer;
    u16 pc;
    u8 sp;
    u16 stack[16];
} chip8_cpu_t;

void chip8_reset_cpu(chip8_cpu_t *cpu);


typedef struct {
    u8 mem[4096];
    u8 vram[64*32];
    u8 draw_font;
    chip8_cpu_t cpu;
} chip8_state_t;

void chip8_reset_state();
int chip8_load_rom(const char *file);
void chip8_execute_step();
u8 *chip8_get_vram();


// keys

typedef enum {
    CHIP8_KEY_0,
    CHIP8_KEY_1,
    CHIP8_KEY_2,
    CHIP8_KEY_3,
    CHIP8_KEY_4,
    CHIP8_KEY_5,
    CHIP8_KEY_6,
    CHIP8_KEY_7,
    CHIP8_KEY_8,
    CHIP8_KEY_9,
    CHIP8_KEY_A,
    CHIP8_KEY_B,
    CHIP8_KEY_C,
    CHIP8_KEY_D,
    CHIP8_KEY_E,
    CHIP8_KEY_F,
} chip8_keys_t;

void chip8_key_event(chip8_keys_t key, u8 status);

// instructions

typedef enum {
    I_SCDOWN = 0,
    I_CLS,
    I_RTS,
    I_SCRIGHT,
    I_SCLEFT,
    I_LOW,
    I_HIGH,
    I_JMP,
    I_JSR,
    I_SKEQI,
    I_SKNEI,
    I_SKEQ,
    I_MOVI,
    I_ADDI,
    I_MOV,
    I_OR,
    I_AND,
    I_XOR,
    I_ADD,
    I_SUB,
    I_SHR,
    I_RSB,
    I_SHL,
    I_SKNE,
    I_MVI,
    I_JMI,
    I_RAND,
    I_SPRITE,
    I_XSPRITE,
    I_SKPR,
    I_SKUP,
    I_GDELAY,
    I_KEY,
    I_SDELAY,
    I_SSOUND,
    I_ADI,
    I_FONT,
    I_XFONT,
    I_BCD,
    I_STR,
    I_LDR,
    I_UNKNOWN
} chip_instr_t;

typedef enum {
    P_NONE,
    P_1, P_2, P_3,
    P_1_2,
    P_1_2_3,
    P_1_23,
    P_123,
} chip8_instr_format_t;


typedef struct {
    u16 code;
    char *mnemonic;
    chip8_instr_format_t format;
    void *func;
} chip8_instruction_t;



#endif // CHIP8_H