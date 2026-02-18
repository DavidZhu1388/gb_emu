#pragma once

#include <common.h>

// https://gbdev.io/pandocs/Timer_and_Divider_Registers.html
typedef struct {
    u16 div; //divider register, incremented at 16384Hz, upper 8 bits are readable as DIV register
    u8 tima;
    u8 tma;
    u8 tac;
} timer_context;

void timer_init();
void timer_tick();

void timer_write(u16 address, u8 value);
u8 timer_read(u16 address);

timer_context *timer_get_context();
