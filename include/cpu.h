#pragma once

#include <common.h>
#include <instructions.h>
#include <bus.h>
#include <emu.h>
#include <stack.h>

typedef struct {
    // CPU registers
    u8 A, F; // Accumulator & Flags
    u8 B, C;
    u8 D, E;
    u8 H, L;
    u16 SP; // Stack Pointer
    u16 PC; // Program Counter
} cpu_registers;

typedef struct {
    cpu_registers regs;
    
    u16 fetched_data; // current fetched instruction data
    u16 mem_dest;
    bool dest_is_mem;
    u8 cur_opcode;
    instruction *cur_inst;

    bool stepping; 
    bool halted;

    bool int_master_enabled;
    u8 ie_register;
} cpu_context;

cpu_registers *cpu_get_regs();

void cpu_init();
bool cpu_step();

typedef void (*IN_PROC)(cpu_context *);

IN_PROC inst_get_processor(in_type type);

#define CPU_FLAG_Z BIT(ctx->regs.F, 7)
#define CPU_FLAG_C BIT(ctx->regs.F, 4)


u16 cpu_read_reg(reg_type reg);
void cpu_set_reg(reg_type rt, u16 val);

u8 cpu_get_ie_register();
void cpu_set_ie_register(u8 n);

void cpu_set_flags(cpu_context *ctx, char z, char n, char h, char c);

u8 cpu_read_reg8(reg_type rt);
void cpu_set_reg8(reg_type rt, u8 val);
