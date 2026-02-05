#include <ram.h>

typedef struct {
    u8 wram[0x2000]; // 8 KB Work RAM
    u8 hram[0x80];   // 128 bytes High RAM
} ram_context;

static ram_context ctx;

u8 wram_read(u16 address) {
    if (address >= 0xC000 && address < 0xE000) {
        return ctx.wram[address - 0xC000];
    }

    printf("WRAM read invalid address: %4.4X\n", address);
    exit(-1);
}

void wram_write(u16 address, u8 value) {
    if (address >= 0xC000 && address < 0xE000) {
        ctx.wram[address - 0xC000] = value;
        return;
    }

    printf("WRAM write invalid address: %4.4X\n", address);
    exit(-1);
}

u8 hram_read(u16 address) {
    if (address >= 0xFF80 && address <= 0xFFFF) {
        return ctx.hram[address - 0xFF80];
    }

    printf("HRAM read invalid address: %4.4X\n", address);
    exit(-1);
}

void hram_write(u16 address, u8 value) {
    if (address >= 0xFF80 && address <= 0xFFFF) {
        ctx.hram[address - 0xFF80] = value;
        return;
    }

    printf("HRAM write invalid address: %4.4X\n", address);
    exit(-1);
}