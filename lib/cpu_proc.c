#include <cpu.h>
#include <emu.h>
#include <bus.h>

//processes CPU instructions

void cpu_set_flags(cpu_context *ctx, char z, char n, char h, char c) {
    if (z != -1) {
        BIT_SET(ctx->regs.F, 7, z);
    }

    if (n != -1) {
        BIT_SET(ctx->regs.F, 6, n);
    }

    if (h != -1) {
        BIT_SET(ctx->regs.F, 5, h);
    }

    if (c != -1) {
        BIT_SET(ctx->regs.F, 4, c);
    }
}

static void proc_none(cpu_context *ctx) {
    printf("IN_NONE executed at PC: %4.4X\n", ctx->regs.PC - 1);
    exit(-7);
}

static void proc_nop(cpu_context *ctx) {
    // NOP does nothing
}

static void proc_ld(cpu_context *ctx) {
    if (ctx->dest_is_mem) {
        // LD (BC), A  for example
        // write to memory
        if (ctx->cur_inst->reg_2 >= RT_AF) {
            // if 16 bit register, write low byte
            bus_write16(ctx->mem_dest, ctx->fetched_data);
        } else {
            bus_write(ctx->mem_dest, ctx->fetched_data);
        }
        return;
    }

    if (ctx->cur_inst->mode == AM_HL_SPR) {
        // special case LD (SPR), A
        u8 hflag = cpu_read_reg(ctx->cur_inst->reg_2) & 0xF + (ctx->fetched_data & 0xF) >= 0x10;
        u8 cflag = cpu_read_reg(ctx->cur_inst->reg_2) & 0xFF + (ctx->fetched_data & 0xFF) >= 0x100;

        cpu_set_flags(ctx, 0, 0, hflag, cflag);
        cpu_set_reg(ctx->cur_inst->reg_1, (cpu_read_reg(ctx->cur_inst->reg_2) + (char)ctx->fetched_data));
        return;
    }

    cpu_set_reg(ctx->cur_inst->reg_1, ctx->fetched_data);
}

static bool check_cond(cpu_context *ctx) {
    bool z = CPU_FLAG_Z;
    bool c = CPU_FLAG_C;

    switch (ctx->cur_inst->cond) {
        case CT_NONE:
            return true;
        case CT_Z:
            return z;
        case CT_NZ:
            return !z;
        case CT_C:
            return c;
        case CT_NC:
            return !c;
        default:
            return false;
    }
}

static void proc_jp(cpu_context *ctx) {
    // JP instruction processing
    // Implementation would go here

    if (check_cond(ctx)) {
        ctx->regs.PC = ctx->fetched_data;
        emu_cycles(1);
    }
}

static void proc_di(cpu_context *ctx) {
    ctx->int_master_enabled = false;
}

static void proc_xor(cpu_context *ctx) {
    // XOR instruction processing
    ctx->regs.A ^= ctx->fetched_data & 0xFF;

    cpu_set_flags(ctx, ctx->regs.A == 0, 0, 0, 0);
}

static IN_PROC processors[] = {
    [IN_NONE] = proc_none, // IN_NONE
    [IN_NOP] = proc_nop, // IN_NOP
    [IN_LD] = proc_ld,
    [IN_JP] = proc_jp,
    [IN_DI] = proc_di,
    [IN_XOR] = proc_xor,

};

IN_PROC inst_get_processor(in_type type) {
    return processors[type];
}