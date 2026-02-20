#include <cpu.h>
#include <bus.h>
#include <emu.h>
#include <interrupts.h>
#include <dbg.h>
#include <timer.h>

cpu_context ctx = {0};

#define CPU_DEBUG 0

void cpu_init() {
    ctx.regs.PC = 0x100;
    ctx.regs.SP = 0xFFFE;
    *((short *)&ctx.regs.A) = 0xB001;
    *((short *)&ctx.regs.B) = 0x1300;
    *((short *)&ctx.regs.D) = 0xD800;
    *((short *)&ctx.regs.H) = 0x4D01;
    ctx.ie_register = 0;
    ctx.int_flags = 0;
    ctx.int_master_enabled = false;
    ctx.enabling_ime = false;

    timer_get_context()->div = 0xABCC;
}

static void fetch_instruction() {
    ctx.cur_opcode = bus_read(ctx.regs.PC++);
    ctx.cur_inst = instruction_by_opcode(ctx.cur_opcode);

    
}

void fetch_data(); // only cpu.c can access this

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
        emu_cycles(1); // fetch takes 1 cycle
        fetch_data();

        char flags[16]; 
        sprintf(flags, "%c%c%c%c", 
            ctx.regs.F & (1 << 7) ? 'Z' : '-',
            ctx.regs.F & (1 << 6) ? 'N' : '-',
            ctx.regs.F & (1 << 5) ? 'H' : '-',
            ctx.regs.F & (1 << 4) ? 'C' : '-'
        );

#if CPU_DEBUG == 1
        char inst[16];
        inst_to_str(&ctx, inst);

        printf("%08lX - %04X: %-12s (%2.2X, %2.2X, %2.2X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X\n", 
            emu_get_context()->ticks,
            pc, inst, ctx.cur_opcode, bus_read(pc+1), bus_read(pc+2), 
            ctx.regs.A, flags, ctx.regs.B, ctx.regs.C, ctx.regs.D, ctx.regs.E, ctx.regs.H, ctx.regs.L);
#endif

        if (ctx.cur_inst == NULL) {
            printf("Invalid opcode: %2.2X at PC: %4.4X\n", ctx.cur_opcode, ctx.regs.PC - 1);
            exit(-6);
        }

        dbg_update();
        dbg_print();

        execute();
    } else {
        // is halted
        emu_cycles(1);

        if (ctx.int_flags) {
            ctx.halted = false;
        }
    }

    if (ctx.int_master_enabled) {
        cpu_handle_interrupts(&ctx);
        ctx.enabling_ime = false;
    }

    if (ctx.enabling_ime) {
        ctx.int_master_enabled = true;
    }

    return true;
}

u8 cpu_get_ie_register() {
    return ctx.ie_register;
}

void cpu_set_ie_register(u8 n) {
    ctx.ie_register = n;
}

void cpu_request_interrupt(interrupt_type t) {
    ctx.int_flags |= t;
}