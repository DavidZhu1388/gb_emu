#include <cpu.h>
#include <bus.h>
#include <emu.h>

cpu_context ctx = {0};

void cpu_init() {
    ctx.regs.PC = 0x0100; // start of program
    ctx.regs.A = 0x01; // default value
}

static void fetch_instruction() {
    ctx.cur_opcode = bus_read(ctx.regs.PC++);
    ctx.cur_inst = instruction_by_opcode(ctx.cur_opcode);

    
}

static void fetch_data() {
    ctx.mem_dest = 0;
    ctx.dest_is_mem = false;

    if (ctx.cur_inst == NULL) {
        return;
    }

    switch (ctx.cur_inst->mode) {
        case AM_IMP: return;

        case AM_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg_1);
            return;
        
        case AM_R_D8:
            ctx.fetched_data = bus_read(ctx.regs.PC);
            emu_cycles(1);
            ctx.regs.PC++;
            return;

        case AM_D16: {
            u16 lo = bus_read(ctx.regs.PC);
            emu_cycles(1);

            u16 hi = bus_read(ctx.regs.PC + 1);
            emu_cycles(1);

            ctx.fetched_data = (hi << 8) | lo;

            ctx.regs.PC += 2;
            return;
        }
            

        default:
            printf("Unknown addressing mode %d (%02X)\n", ctx.cur_inst->mode, ctx.cur_opcode);
            exit(-7);
            return;
    }

}

static void execute() {
    IN_PROC proc = inst_get_processor(ctx.cur_inst->type);

    if (!proc) {
        NO_IMPL;
    }

    proc(&ctx);
    
}

bool cpu_step() {

    if (!ctx.halted) {
        u16 pc = ctx.regs.PC;

        fetch_instruction();
        fetch_data();

        printf("%4.4X: %-7s (%2.2X, %2.2X, %2.2X) A: %02X B: %02X C: %02X\n", 
            pc, inst_name(ctx.cur_inst->type), 
            ctx.cur_opcode, bus_read(pc+2), bus_read(pc+1),
            ctx.regs.A,  ctx.regs.B, ctx.regs.C);

        if (ctx.cur_inst == NULL) {
            printf("Invalid opcode: %2.2X at PC: %4.4X\n", ctx.cur_opcode, ctx.regs.PC - 1);
            exit(-6);
        }

        execute();
    }

    return true;
}
