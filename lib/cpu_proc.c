#include <cpu.h>

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

static void goto_addr(cpu_context *ctx, u16 addr, bool pushpc){
    if (check_cond(ctx)) {
        if (pushpc) {
            emu_cycles(2);
            stack_push16(ctx->regs.PC);
        }

        ctx->regs.PC = addr;
        emu_cycles(1);
    }
}

static void proc_jp(cpu_context *ctx) {
    // JP instruction processing
    goto_addr(ctx, ctx->fetched_data, false);
}

static void proc_jr(cpu_context *ctx) {
    char rel = (char)(ctx->fetched_data & 0xFF);
    u16 addr = ctx->regs.PC + rel;
    goto_addr(ctx, addr, false);
}

static void proc_call(cpu_context *ctx) {
    goto_addr(ctx, ctx->fetched_data, true);
}

static void proc_rst(cpu_context *ctx) {
    goto_addr(ctx, ctx->cur_inst->param, true);
}

static void proc_ret(cpu_context *ctx) {
    if (ctx->cur_inst->cond != CT_NONE) {
        emu_cycles(1);
    }

    if (check_cond(ctx)) {
        u16 lo = stack_pop();
        emu_cycles(1);
        u16 hi = stack_pop();
        emu_cycles(1);

        u16 n = (hi << 8) | lo;
        ctx->regs.PC = n;

        emu_cycles(1);
    }
}

static void proc_reti(cpu_context *ctx) { // returning from an interrupt
    ctx->int_master_enabled = true;
    proc_ret(ctx);
}

static void proc_pop(cpu_context *ctx) {
    u16 lo = stack_pop();
    emu_cycles(1);
    u16 hi = stack_pop();
    emu_cycles(1);

    u16 n = (hi << 8) | lo;

    cpu_set_reg(ctx->cur_inst->reg_1, n);

    if (ctx->cur_inst->reg_1 == RT_AF) {
        // if 16 bit register
        cpu_set_reg(ctx->cur_inst->reg_1, n & 0xFFF0); // lower nibble of F is always 0
    }

}

static void proc_push(cpu_context *ctx) {
    // PUSH instruction processing
    
    u16 hi = (cpu_read_reg(ctx->cur_inst->reg_1) >> 8) & 0xFF;
    emu_cycles(1);
    stack_push(hi);

    u16 lo = cpu_read_reg(ctx->cur_inst->reg_2) & 0xFF;
    emu_cycles(1);
    stack_push(lo);

    emu_cycles(1); //for the push itself
}

static void proc_di(cpu_context *ctx) {
    ctx->int_master_enabled = false;
}

static void proc_ldh(cpu_context *ctx) {
    // LDH instruction processing
    if (ctx->cur_inst->reg_1 == RT_A) {
        // LDH A, (a8). (a8) = 0xFF00 + a8
        u16 addr = 0xFF00 | (ctx->fetched_data & 0xFF);
        cpu_set_reg(ctx->cur_inst->reg_1, bus_read(addr));
    } else {
        // LDH (a8), A
        u16 addr = 0xFF00 | (ctx->fetched_data & 0xFF);
        bus_write(addr, ctx->regs.A);
    }
    emu_cycles(1);
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
    [IN_LDH] = proc_ldh,
    [IN_JP] = proc_jp,
    [IN_DI] = proc_di,
    [IN_POP] = proc_pop,
    [IN_PUSH] = proc_push,
    [IN_JR] = proc_jr,
    [IN_CALL] = proc_call,
    [IN_RET] = proc_ret,
    [IN_RST] = proc_rst,
    [IN_RETI] = proc_reti,
    [IN_XOR] = proc_xor,

};

IN_PROC inst_get_processor(in_type type) {
    return processors[type];
}