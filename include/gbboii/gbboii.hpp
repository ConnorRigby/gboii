#ifndef LIBGBBOII_H
#define LIBGBBOII_H

#include <gbboii/utils.hpp>
#include <gbboii/cpu.hpp>
#include <gbboii/memory.hpp>
#include <gbboii/lua_lib.hpp>

#include <stdbool.h>

#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07
#define CLOCKSPEED 4194304

typedef enum COLOR {
  WHITE,
  LIGHT_GRAY,
  DARK_GRAY,
  BLACK
} gb_color_t;

class Gameboy {
private:
  int timer_counter;
  int divider_counter;
  int scanline_counter;
  int frequency;
  bool interupt_master;
  lua_State *L;
public:
  CPU cpu;
  Memory mem;
  uint8_t screen_data[160][144][3];
  Gameboy(char* bootrom);
  bool running;
  void advance_frame();
  void execute_opcode();
  void update_timers();
  bool is_clock_enabled();
  void set_clock_freq(uint8_t new_freq);
  void request_interrupt(int id);
  void do_interupts();
  void service_interupt(int interupt);
  void update_graphics();
  bool is_lcd_enabled();
  void set_lcd_status();
  void draw_scanline();
  void render_tiles(uint8_t lcd_control);
  void render_sprites(uint8_t lcd_control);
  gb_color_t get_color(uint8_t color_num, uint16_t address);

  void lua_init();
  int load_script(const char* filename);
};


#endif
