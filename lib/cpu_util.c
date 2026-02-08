#include <cpu.h>

extern cpu_context ctx;

u16 reverse(u16 n) {
    return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
}

u16 cpu_read_reg(reg_type rt) {
    switch(rt) {
        case RT_A: return ctx.regs.A;
        case RT_F: return ctx.regs.F;
        case RT_B: return ctx.regs.B;
        case RT_C: return ctx.regs.C;
        case RT_D: return ctx.regs.D;
        case RT_E: return ctx.regs.E;
        case RT_H: return ctx.regs.H;
        case RT_L: return ctx.regs.L;

        case RT_AF: return reverse(*((u16 *)&ctx.regs.A)); // need to reverse due to little endian
        case RT_BC: return reverse(*((u16 *)&ctx.regs.B));
        case RT_DE: return reverse(*((u16 *)&ctx.regs.D));
        case RT_HL: return reverse(*((u16 *)&ctx.regs.H));

        case RT_PC: return ctx.regs.PC;
        case RT_SP: return ctx.regs.SP;
        default: return 0;
    }
}

void cpu_set_reg(reg_type rt, u16 val) {
    switch(rt) {
        case RT_A: ctx.regs.A = val & 0xFF; break;
        case RT_F: ctx.regs.F = val & 0xFF; break;
        case RT_B: ctx.regs.B = val & 0xFF; break;
        case RT_C: ctx.regs.C = val & 0xFF; break; // note
        case RT_D: ctx.regs.D = val & 0xFF; break;
        case RT_E: ctx.regs.E = val & 0xFF; break;
        case RT_H: ctx.regs.H = val & 0xFF; break;
        case RT_L: ctx.regs.L = val & 0xFF; break;

        case RT_AF: *((u16 *)&ctx.regs.A) = reverse(val); break;
        case RT_BC: *((u16 *)&ctx.regs.B) = reverse(val); break;
        case RT_DE: *((u16 *)&ctx.regs.D) = reverse(val); break;
        case RT_HL: *((u16 *)&ctx.regs.H) = reverse(val); break; // note
        
        case RT_PC: ctx.regs.PC = val; break;
        case RT_SP: ctx.regs.SP = val; break;
        case RT_NONE: break;
    }
}

cpu_registers *cpu_get_regs() {
    return &ctx.regs;
}