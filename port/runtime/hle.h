#ifndef DL_HLE_H
#define DL_HLE_H
#include "cpu.h"
#define HLE_SEG 0xF000          /* fake BIOS segment: IVT entries point to F000:(n*4) */
#define EMS_SEG 0xF100          /* EMS driver: entry F100:0100, device name at F100:000A */
void hle_stub(u16 seg, u16 off);         /* a far call/jump landed in the fake BIOS */
void (*driver_find(u16 seg, u16 off))(void);
void hle_init(const char *gamedir, const char *exe, int argc, char **argv);
void ovl_init(const char *exe_path);
/* platform */
void plat_init(int scale);
void plat_present(const u8 *vga, const u8 *pal_rgb6);
void plat_events(void);
u32 plat_ticks_ms(void);
uint64_t plat_ticks_us(void);
void bios_init(void);
void dos_mem_init(u16 first_seg, u16 paras);
void ovl_preload(void);
int bios_kbd_avail(void);
u16 bios_kbd_get(int remove);
extern u16 psp_seg;
void plat_sleep_ms(int ms);
void plat_quit(int code);
/* input state shared with bios.c */
void kbd_push_scancode(u8 sc, int release);
void mouse_update(int x, int y, int buttons);
extern u8 vga_dac[768];
extern int video_mode;
extern char game_dir[1024];
#endif
