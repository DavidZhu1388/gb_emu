#include <ppu.h>
#include <lcd.h>
#include <cpu.h>
#include <interrupts.h>
#include <string.h>
#include <cart.h>

void pipeline_fifo_reset();
void pipeline_process();

void increment_ly() {
    lcd_get_context()->ly++;

    if (lcd_get_context()->ly == lcd_get_context()->ly_compare) {
        LCDS_LYC_SET(true);

        if (LCDS_STAT_INT(SS_LYC)) {
            cpu_request_interrupt(IT_LCD_STAT);
        }
    } else {
        LCDS_LYC_SET(false);
    }
}

void ppu_mode_oam() {
    // During this mode, the PPU reads from OAM to find sprites that should be rendered on the current line.
    // The PPU can read up to 10 sprites during this mode. If more than 10 sprites are on the line, only the first 10 will be rendered.
    // This mode lasts for 80 cycles.
    if (ppu_get_context()->line_ticks >= 80) {
        LCDS_MODE_SET(MODE_XFER);

        ppu_get_context()->pfc.cur_fetch_state = FS_TILE;
        ppu_get_context()->pfc.line_x = 0;
        ppu_get_context()->pfc.fetch_x = 0;
        ppu_get_context()->pfc.pushed_x = 0;
        ppu_get_context()->pfc.fifo_x = 0;
    }
}

void ppu_mode_xfer() {
    pipeline_process();

    // During this mode, the PPU reads from VRAM to render the current line. The PPU also reads from OAM to render sprites on top of the background.
    if (ppu_get_context()->pfc.pushed_x >= XRES) {
        pipeline_fifo_reset();

        LCDS_MODE_SET(MODE_HBLANK);

        if (LCDS_STAT_INT(SS_HBLANK)) {
            cpu_request_interrupt(IT_LCD_STAT);
        }
    }
}

static u32 target_frame_time = 1000 / 60; // 60 FPS
static long prev_frame_time = 0;
static long start_timer = 0;
static long frame_count = 0;

void ppu_mode_hblank() {
    // During this mode, the PPU is idle and the CPU can access OAM and VRAM. This is the only time the CPU can access VRAM.
    // This mode lasts for 204 cycles, but can be shorter if the line rendering finishes early.
    if (ppu_get_context()->line_ticks >= TICKS_PER_LINE) {
        increment_ly();

        if (lcd_get_context()->ly >= YRES) {
            LCDS_MODE_SET(MODE_VBLANK);

            cpu_request_interrupt(IT_VBLANK);

            if (LCDS_STAT_INT(SS_VBLANK)) {
                cpu_request_interrupt(IT_LCD_STAT);
            }
            ppu_get_context()->current_frame++;

            //calc FPS..
            u32 end = get_ticks();
            u32 frame_time = end - prev_frame_time;

            if (frame_time < target_frame_time) {
                delay((target_frame_time - frame_time));
            }

            if (end - start_timer >= 1000) {
                u32 fps = frame_count;
                start_timer = end;
                frame_count = 0;

                printf("FPS: %d\n", fps);
            } 

            frame_count++;
            prev_frame_time = get_ticks();

        } else {
            LCDS_MODE_SET(MODE_OAM);
        }

        ppu_get_context()->line_ticks = 0;
    }
}

void ppu_mode_vblank() {
    // During this mode, the PPU is idle and the CPU can access OAM and VRAM. This is the only time the CPU can access VRAM.
    // This mode lasts for 4560 cycles (10 lines).
    if (ppu_get_context()->line_ticks >= TICKS_PER_LINE) {
        increment_ly();

        if (lcd_get_context()->ly >= LINES_PER_FRAME) {
            lcd_get_context()->ly = 0;
            LCDS_MODE_SET(MODE_OAM);
        }

        ppu_get_context()->line_ticks = 0;
    }
}
