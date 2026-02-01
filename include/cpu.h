#pragma once

#include <common.h>
#include <instructions.h>

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
} cpu_context;

void cpu_init();
bool cpu_step();

typedef void (*IN_PROC)(cpu_context *);

IN_PROC inst_get_processor(in_type type);

#define CPU_FLAG_Z BIT(ctx->regs.F, 7)
#define CPU_FLAG_C BIT(ctx->regs.F, 4)


u16 cpu_read_reg(reg_type reg);

void cpu_set_flags(cpu_context *ctx, char z, char n, char h, char c);
