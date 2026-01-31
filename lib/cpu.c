#include <cpu.h>
#include <bus.h>
#include <emu.h>

cpu_context ctx = {0};

void cpu_init() {
    ctx.regs.PC = 0x0100; // start of program
}

static void fetch_instruction() {
    ctx.cur_opcode = bus_read(ctx.regs.PC++);
    ctx.cur_inst = instruction_by_opcode(ctx.cur_opcode);

    if (ctx.cur_inst == NULL) {
        printf("Invalid opcode: %2.2X at PC: %4.4X\n", ctx.cur_opcode, ctx.regs.PC - 1);
        exit(-6);
    }
}

static void fetch_data() {
    ctx.mem_dest = 0;
    ctx.dest_is_mem = false;

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
            printf("Unknown addressing mode %d\n", ctx.cur_inst->mode);
            exit(-7);
            return;
    }

}

static void execute() {
    printf("Not implemented execution yet...\n");
}

bool cpu_step() {

    if (!ctx.halted) {
        u16 pc = ctx.regs.PC;

        fetch_instruction();
        fetch_data();

        printf("Executing opcode: %2.2X at PC: %4.4X\n", ctx.cur_opcode, pc);

        execute();
    }

    return true;
}
