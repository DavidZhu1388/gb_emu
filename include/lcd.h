#pragma once 

#include <common.h>

typedef struct {
    //registers
    u8 lcdc;
    u8 lcds; // status
    u8 scroll_y;
    u8 scroll_x;
    u8 ly;
    u8 ly_compare;
    u8 dma;
    u8 bg_palette;
    u8 obj_palette[2];
    u8 win_y;
    u8 win_x;

    //other data
    u32 bg_colors[4];
    u32 sp1_colors[4];
    u32 sp2_colors[4];

} lcd_context;

typedef enum {
    MODE_HBLANK,
    MODE_VBLANK,
    MODE_OAM,
    MODE_XFER
} lcd_mode;

lcd_context *lcd_get_context();

/*
7    LCD & PPU enable: 0 = Off; 1 = On
6    Window tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
5    Window enable: 0 = Off; 1 = On
4    BG & Window tile data area: 0 = 8800–97FF; 1 = 8000–8FFF
3    BG tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
2    OBJ size: 0 = 8×8; 1 = 8×16
1    OBJ enable: 0 = Off; 1 = On
0    BG & Window enable / priority [Different meaning in CGB Mode]: 0 = Off; 1 = On
*/
#define LCDC_BGW_ENABLE (BIT(lcd_get_context()->lcdc, 0))
#define LCDC_OBJ_ENABLE (BIT(lcd_get_context()->lcdc, 1))
#define LCDC_OBJ_HEIGHT (BIT(lcd_get_context()->lcdc, 2) ? 16 : 8)
#define LCDC_BG_MAP_AREA (BIT(lcd_get_context()->lcdc, 3) ? 0x9C00 : 0x9800)
#define LCDC_BGW_DATA_AREA (BIT(lcd_get_context()->lcdc, 4) ? 0x8000 : 0x8800)
#define LCDC_WIN_ENABLE (BIT(lcd_get_context()->lcdc, 5))
#define LCDC_WIN_MAP_AREA (BIT(lcd_get_context()->lcdc, 6) ? 0x9C00 : 0x9800)
#define LCDC_LCD_ENABLE (BIT(lcd_get_context()->lcdc, 7))

/*
6    LYC int select (Read/Write): If set, selects the LYC == LY condition for the STAT interrupt.
5    Mode 2 int select (Read/Write): If set, selects the Mode 2 condition for the STAT interrupt.
4    Mode 1 int select (Read/Write): If set, selects the Mode 1 condition for the STAT interrupt.
3    Mode 0 int select (Read/Write): If set, selects the Mode 0 condition for the STAT interrupt.
2    LYC == LY (Read-only): Set when LY contains the same value as LYC; it is constantly updated.
1 0    PPU mode (Read-only): Indicates the PPU’s current status. Reports 0 instead when the PPU is disabled.
*/

#define LCDS_MODE ((lcd_mode)(lcd_get_context()->lcds & 0b11))
#define LCDS_MODE_SET(mode) { lcd_get_context()->lcds &= ~0b11; lcd_get_context()->lcds |= mode; }

#define LCDS_LYC (BIT(lcd_get_context()->lcds, 2))
#define LCDS_LYC_SET(b) (BIT_SET(lcd_get_context()->lcds, 2, b))

typedef enum {
    SS_HBLANK = (1 << 3),
    SS_VBLANK = (1 << 4),
    SS_OAM = (1 << 5),
    SS_LYC = (1 << 6),
} stat_src;

#define LCDS_STAT_INT(src) (lcd_get_context()->lcds & src)

void lcd_init();

u8 lcd_read(u16 address);
void lcd_write(u16 address, u8 value);