#include <gbboii/gbboii.hpp>
#include <gbboii/cpu.hpp>
#include <gbboii/memory.hpp>
#include <gbboii/utils.hpp>
#include <gbboii/lua_lib.hpp>

const mem_addr_t ier = 0xffff;
const mem_addr_t irr = 0xff0f;
const int MAXCYCLES = 69905;

Gameboy::Gameboy(char* bootrom) : mem(Memory(bootrom)) {
  debug_print("Gameboy init.\r\n");
  running = true;
  frequency = 4096;
  timer_counter = CLOCKSPEED/frequency;
  divider_counter = 0;
  scanline_counter = 0;
  lua_init();
}

// bool Gameboy::is_lcd_enabled() { return nth_bit(mem.read8(0xFF40), 7); }
// bool Gameboy::is_clock_enabled() { return nth_bit(mem.read8(TMC), 2); }
bool Gameboy::is_lcd_enabled() { return true; }
bool Gameboy::is_clock_enabled() { return true; }

void Gameboy::advance_frame() {
  int cyclesThisUpdate = 0;
  while(cyclesThisUpdate < MAXCYCLES && running) {
    execute_opcode();
    cyclesThisUpdate+=cpu.cycles;
    update_timers();
    update_graphics();
    do_interupts();
  }
}

void Gameboy::render_tiles(uint8_t lcd_control) {
  uint16_t tile_data = 0;
  uint16_t background_memory =0 ;
  bool unsig = true;

  uint8_t scroll_y = mem.read8(0xFF42);
  uint8_t scroll_x = mem.read8(0xFF43);
  uint8_t window_x = mem.read8(0xFF4A);
  uint8_t window_y = mem.read8(0xFF4B) - 7;

  bool using_window = false;

  // Window is enabled
  if(nth_bit(lcd_control, 5)) {
    using_window = window_y <= mem.read8(0xFF44);
  }

  if(nth_bit(lcd_control, 4)) {
    tile_data = 0x8000;
  } else {
    tile_data = 0x8800;
    unsig = false;
  }

  if(using_window == false) {
    background_memory = nth_bit(lcd_control, 3) ? 0x9C00 : 0x9800;
  } else {
    background_memory = nth_bit(lcd_control, 6) ? 0x9C00 : 0x9800;
  }

  uint8_t y_pos = 0;
  uint8_t x_pos;
  if(using_window) {
    y_pos = mem.read8(0xFF44) - window_y;
  } else {
    y_pos = mem.read8(0xFF44) + scroll_y;
  }

  uint8_t tile_row = (((uint8_t)(y_pos/8)) * 32);
  for(int pixel = 0; pixel < 160; pixel++) {
    x_pos = pixel+scroll_x;
    if(using_window) {
      if(pixel >= window_x) x_pos = pixel - window_x;
    }

  uint16_t tile_col = (x_pos/8);
  int8_t tile_num;
  uint8_t tile_address = background_memory + tile_row + tile_col;
  if(unsig) {
    tile_num = (uint8_t)mem.read8(tile_address);
  } else {
    tile_num = (int8_t)mem.read8(tile_address);
  }

  uint16_t tile_location = tile_data;
  if(unsig) {
    tile_location += (tile_num * 16);
  } else {
    tile_location += (int8_t)((tile_num+128) * 16);
  }

  uint8_t line = y_pos % 8;
  line *= 2;
  uint8_t data1 = mem.read8(tile_location + line);
  uint8_t data2 = mem.read8(tile_location + line + 1);

  int color_bit = x_pos % 8;
  color_bit -= 7;
  color_bit *= -1;

  int color_num = (int)nth_bit(data2, color_bit);
  color_num <<= 1;
  color_num |= (int)nth_bit(data1, color_bit);

  gb_color_t col = get_color(color_num, 0xFF47);
  int red = 0;
  int green = 0;
  int blue = 0;

  switch(col) {
    case WHITE:   	 red = 255;  green = 255;  blue = 255; break;
    case LIGHT_GRAY: red = 0xCC; green = 0xCC; blue = 0xCC; break;
    case DARK_GRAY:	 red = 0x77; green = 0x77; blue = 0x77; break;
    case BLACK: break;
  }

  int finally = mem.read8(0xFF44);
  if ((finally<0) || (finally>143) || (pixel<0) || (pixel>159)) {
    return;
  }

  screen_data[pixel][finally][0] = red;
  screen_data[pixel][finally][1] = green;
  screen_data[pixel][finally][2] = blue;
  }
}

void Gameboy::render_sprites(uint8_t lcd_control) {
  bool use8x16 = false;
  use8x16 = nth_bit(lcd_control, 2);

  for(uint8_t sprite = 0; sprite < 40; sprite++) {
    uint8_t index = sprite*4;
    uint8_t y_pos = mem.read8(0xFE00+index) - 16;
    uint8_t x_pos = mem.read8(0xFE00+index+1) - 8;
    uint8_t tile_location = mem.read8(0xFE00+index+2);
    uint8_t attributes = mem.read8(0xFE00 + index+3);

    bool y_flip = nth_bit(attributes, 6);
    bool x_flip = nth_bit(attributes, 5);

    int scanline = mem.read8(0xFF44);
    int y_size = 8;
    if(use8x16) y_size = 16;

    if((scanline >= y_pos) && (scanline < (y_pos + y_size))) {
      int line = scanline - y_pos;

      if(y_flip) {
        line-=y_size;
        line *= -1;
      }

      line *= 2;
      uint16_t data_address = (0x8000 + (tile_location * 16)) + line;
      uint8_t data1 = mem.read8(data_address);
      uint8_t data2 = mem.read8(data_address + 1);

      for(int tile_pixel = 7; tile_pixel >= 0; tile_pixel--) {
        int color_bit = tile_pixel;
        if(x_flip) {
          color_bit -= 7;
          color_bit *= -1;
        }

        int color_num = nth_bit(data2, color_bit);
        color_num <<= 1;
        color_num |= nth_bit(data1, color_bit);

        uint16_t color_address = nth_bit(attributes, 4) ? 0xFF49 : 0xFF48;
        gb_color_t col = get_color(color_num, color_address);

        if(col == WHITE) return;

        int red = 0;
        int green = 0;
        int blue = 0;

        switch(col) {
          case WHITE:      red =255;  green=255;  blue=255; break;
          case LIGHT_GRAY: red =0xCC; green=0xCC; blue=0xCC; break;
          case DARK_GRAY:  red =0x77; green=0x77; blue=0x77; break;
          case BLACK: break;
        }

        int x_pix = 0 - tile_pixel;
        x_pix += 7;

        int pixel = x_pos+x_pix;
        if ((scanline<0) || (scanline>143) || (pixel<0) || (pixel>159)) return;

         screen_data[pixel][scanline][0] = red;
         screen_data[pixel][scanline][1] = green;
         screen_data[pixel][scanline][2] = blue;
      }
    }
  }
}

gb_color_t Gameboy::get_color(uint8_t color_num, uint16_t address) {
  gb_color_t res = WHITE;
  uint8_t palette = mem.read8(address);
  int hi = 0, lo = 0;

  switch(color_num) {
    case 0: { hi = 1; lo = 0; } break;
    case 1: { hi = 3; lo = 2; } break;
    case 2: { hi = 5; lo = 4; } break;
    case 3: { hi = 7; lo = 6; } break;
  }
  int color = 0;
  color = (int)nth_bit(palette, hi) << 1;
  color |= (int)nth_bit(palette, lo);

  switch(color) {
    case 0: {res = WHITE;} break;
    case 1: {res = LIGHT_GRAY;} break;
    case 2: {res = DARK_GRAY;} break;
    case 3: {res = BLACK;} break;
  }
  return res;
}

void Gameboy::draw_scanline() {
  uint8_t lcd_control = mem.read8(0xFF40);
  // debug_print("draw scanline: %#04x\r\n", lcd_control);
  if(nth_bit(lcd_control, 0)) render_tiles(lcd_control);
  if(nth_bit(lcd_control, 1)) render_sprites(lcd_control);
}

void Gameboy::set_lcd_status() {
  uint8_t status = mem.read8(0xFF41);
  if(is_lcd_enabled() == false) {
    scanline_counter = 456;
    mem.mem[0xFF44] = 0;
    status &= 252;
    status = set_bit(status, 0);
    mem.write8(0xFF41, status);
    return;
  }

  uint8_t ly = mem.read8(0xFF44);
  uint8_t current_line = mem.read8(0xFF44);
  uint8_t current_mode = status & 0x3;
  uint8_t mode = 0;
  bool req_int = false;

  // in vblank so set mode to 1
  if (current_line >= 144) {
    mode = 1;
    status = set_bit(status, 0);
    status = set_bit(status, 1);
    req_int = nth_bit(status, 4);
  } else {
    int mode2bounds = 456-80;
    int mode3bounds = mode2bounds - 172;

    // mode 2
    if(scanline_counter >= mode2bounds) {
      mode = 2;
      status = set_bit(status, 1);
      status = reset_bit(status, 0);
      req_int = nth_bit(status, 5);
    // mode 3
    } else if(scanline_counter >= mode3bounds) {
      mode = 3;
      status = set_bit(status, 1);
      status = reset_bit(status, 0);
    // mode 0
    } else {
      mode = 0;
      status = reset_bit(status, 1);
      status = reset_bit(status, 0);
      req_int = nth_bit(status, 3);
    }
  }

  // just entered a new mode so request interupt
  if(req_int && (mode != current_mode)) request_interrupt(1);

  // check the conincidence flag
  if (mem.read8(0xFF45) == ly) {
    status = set_bit(status, 2);
    if (nth_bit(status, 6)) request_interrupt(1);
  } else {
    status = reset_bit(status, 2);
  }
  mem.write8(0xFF41, status);
}

void Gameboy::update_graphics() {
  int cycles = cpu.cycles;
  set_lcd_status();
  if(is_lcd_enabled())  {
    scanline_counter -= cycles;
  } else {
    return;
  }

  if (scanline_counter <= 0) {
    // time to move onto next scanline
    mem.mem[0xFF44]++;
    uint8_t current_line = mem.read8(0xFF44);

    scanline_counter = 456;

    // we have entered vertical blank period
    if (current_line == 144) request_interrupt(0);

    // if gone past scanline 153 reset to 0
    else if (current_line > 153) mem.mem[0xFF44]=0;

    // draw the current scanline
    else if (current_line < 144) draw_scanline();
   }
}

void Gameboy::request_interrupt(int id)  {
	uint8_t req = mem.read8(irr);
  req = set_bit(req, id);
	mem.write8(irr, req);
}

void Gameboy::do_interupts() {
  if(interupt_master == true) {
    uint8_t req = mem.read8(irr);
    uint8_t enabled = mem.read8(ier);
    if(req > 0) {
      for(int i = 0; i<5; i++) {
        if(nth_bit(req, i) == 1) {
          if(nth_bit(enabled, i) == 1) {
            service_interupt(i);
          }
        }
      }
    }
  }
}

void Gameboy::service_interupt(int interupt) {
  interupt_master = false;
  uint8_t req = mem.read8(irr);
  req = reset_bit(req, interupt);
  mem.write8(irr, req);

  /// we must save the current execution address by pushing it onto the stack
  // PushWordOntoStack(m_ProgramCounter);
  mem_addr_t stack_addr = cpu.read_register(REG_SP);
  mem.write8(stack_addr, cpu.read_registerl(REG_PC));
  mem.write8(stack_addr-1, cpu.read_registerh(REG_PC));
  cpu.write_register(REG_SP, cpu.read_register(REG_SP) - 2);

  switch(interupt) {
    case 0: cpu.write_register(REG_PC, 0x40); break;
    case 1: cpu.write_register(REG_PC, 0x48); break;
    case 2: cpu.write_register(REG_PC, 0x50); break;
    case 4: cpu.write_register(REG_PC, 0x60); break;
  }
}

void Gameboy::set_clock_freq(uint8_t new_freq) {
  switch (new_freq) {
    case 0: timer_counter = 1024; break; // freq 4096
    case 1: timer_counter = 16; break; // freq 262144
    case 2: timer_counter = 64; break; // freq 65536
    case 3: timer_counter = 256; break; // freq 16382
  }
}

void Gameboy::update_timers() {
  // Check if the timer frequency needs to be changed.
  uint8_t current_freq = frequency;
  uint8_t new_freq = mem.read8(TMC) & 0x3;
  if(current_freq != new_freq) { set_clock_freq(new_freq); }


  int cycles = cpu.cycles;
  divider_counter+=cycles;
  if (divider_counter >= 255) {
     divider_counter = 0;
     // this address is protected, so write it manually.
     mem.mem[0xFF04]++;
  }
  // the clock must be enabled to update the clock
  if (is_clock_enabled()) {
    timer_counter -= cycles;

    // enough cpu clock cycles have happened to update the timer
    if (timer_counter <= 0) {
      // reset m_TimerTracer to the correct value
      set_clock_freq(mem.read8(TMC) & 0x3);

      // timer about to overflow
      if (mem.read8(TIMA) == 255) {
        mem.write8(TIMA, mem.read8(TIMA));
        request_interrupt(2);
      }
      else {
        mem.write8(TIMA, mem.read8(TIMA)+1);
      }
    }
  }
}

void Gameboy::execute_opcode() {
  uint16_t addr = cpu.read_register(REG_PC);
  opcode_t instr = mem.read8(addr);
  debug_print("Gameboy tick: instr: %#04x PC: %#04x\r\n", instr, addr);

  switch(instr) {

    // LD (BC),A 1/8 ----
    case 0x02: {
      mem_addr_t addr = cpu.read_register(REG_BC);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(addr, data);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=8;
    } break;

    // INC B 1/4 Z0H-
    case 0x04: {
      uint8_t before = cpu.read_registerh(REG_BC);
      uint8_t data = before + 1;
      cpu.write_registerh(REG_BC, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.reset_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, ((before & 0xF) == 0xF));
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
    } break;

    // DEC B 1/4 Z1H-
    case 0x05: {
      uint8_t before = cpu.read_registerh(REG_BC);
      uint8_t data = before-1;
      cpu.write_registerh(REG_BC, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.set_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, (before & 0x0F) == 0);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
    } break;

    // LD B,d8 2/8 ----
    case 0x06: {
      cpu.write_register(REG_PC, addr+1);
      uint8_t data = mem.read8(cpu.read_register(REG_PC));
      cpu.write_registerh(REG_BC, data);
      cpu.write_register(REG_PC, addr+2);
      cpu.cycles=8;
    } break;

    // DEC C 1/4 Z1H-
    case 0x0D: {
      uint8_t before = cpu.read_registerl(REG_BC);
      uint8_t data = before-1;
      cpu.write_registerl(REG_BC, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.set_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, (before & 0x0F) == 0);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
    } break;

    // LD C,d8 2/8 ----
    case 0x0E: {
      cpu.write_register(REG_PC, addr+1);
      addr = cpu.read_register(REG_PC);
      uint8_t data = mem.read8(addr);
      cpu.write_registerl(REG_BC, data);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=8;
    } break;

    // LD DE,d16 3/16 ----
    case 0x11: {
      uint16_t data = mem.read16(addr+1);
      cpu.write_register(REG_DE, data);
      cpu.write_register(REG_PC, addr+3);
      cpu.cycles=16;
    } break;

    // INC DE 1/8 ----
    case 0x13: {
      cpu.write_register(REG_DE, cpu.read_register(REG_DE)-1);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=8;
    } break;

    // DEC D 1/4 Z1H-
    case 0x15: {
      uint8_t before = cpu.read_registerh(REG_DE);
      uint8_t data = before-1;
      cpu.write_registerh(REG_DE, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.set_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, (before & 0x0F) == 0);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
    } break;

    // RLA 1/4 000C
    case 0x17: {
      uint8_t reg = cpu.read_registerh(REG_AF);
      bool is_carry_set = cpu.read_flag(FLG_C);
      bool is_msb_set = nth_bit(reg, 7);
      cpu.write_registerl(REG_AF, 0);
      uint8_t value = reg;
      value <<= 1;

      if(is_msb_set)
        cpu.set_flag(FLG_C);

      if(is_carry_set)
        value = set_bit(value, 0);

      if(value == 0) cpu.set_flag(FLG_Z);

      cpu.write_registerh(REG_AF, value);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
    } break;

    // JR r8 2/12 ----
    case 0x18: {
      cpu.write_register(REG_PC, addr+1);
      uint16_t pc = cpu.read_register(REG_PC);
      int8_t jmp_offset = (int8_t)mem.read8(pc);
      cpu.write_register(REG_PC, (pc+1)+jmp_offset);
      cpu.cycles=12;
    } break;

    // LD A,(DE) 1/8 ----
    case 0x1A: {
      mem_addr_t read_addr = cpu.read_register(REG_DE);
      uint8_t data = mem.read8(read_addr);
      cpu.write_registerh(REG_AF, data);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=8;
    } break;

    // DEC E 1/4 Z1H-
    case 0x1D: {
      uint8_t before = cpu.read_registerl(REG_DE);
      uint8_t data = before-1;
      cpu.write_registerl(REG_DE, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.set_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, (before & 0x0F) == 0);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
    } break;

    // LD E,d8 2/8 ----
    case 0x1E: {
      cpu.write_register(REG_PC, addr+1);
      mem_addr_t read_addr = cpu.read_register(REG_PC);
      uint8_t data = mem.read8(read_addr);
      cpu.write_registerl(REG_DE, data);
      cpu.write_register(REG_PC, addr+2);
      cpu.cycles=8;
    } break;

    //JR NZ,r8 2/12|8 ----
    case 0x20: {
      cpu.write_register(REG_PC, addr+1);
      uint16_t pc = cpu.read_register(REG_PC);
      bool zero_flag_is_reset = cpu.flag_is_reset(FLG_Z);
      // debug_print("\tZero flag %s reset: %d\r\n", zero_flag_is_reset ? "is" : "is not", zero_flag_is_reset);
      if(zero_flag_is_reset == 1) {
        int8_t jmp_offset = (int8_t)mem.read8(pc);
        // debug_print("\tJR %#04x + (%d) = %#08x\r\n", pc, jmp_offset, (pc+1)+jmp_offset);
        cpu.write_register(REG_PC, (pc+1+jmp_offset));
        cpu.cycles=12;
      } else {
        // debug_print("\tNot JMP: %#04x\r\n", pc+1);
        cpu.write_register(REG_PC, pc+1);
        cpu.cycles=8;
      }
    } break;

    // LD HL,d16 3/12 ----
    case 0x21: {
      uint16_t pc = cpu.read_register(REG_PC);
      uint16_t val = mem.read16(pc+1);
      cpu.write_register(REG_HL, val);
      cpu.write_register(REG_PC, pc+3);
      cpu.cycles=12;
    } break;

    // LD (HL+),A 1/8 ----
    case 0x22: {
      mem_addr_t load_addr = cpu.read_register(REG_HL);
      mem.write8(load_addr, cpu.read_registerh(REG_AF));
      cpu.write_register(REG_HL, load_addr + 1);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=8;
    } break;

    // INC HL 1/8 ----
    case 0x23: {
      cpu.write_register(REG_HL, cpu.read_register(REG_HL) + 1);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=8;
    } break;

    // INC H 1/4 Z0H-
    case 0x24: {
      uint8_t before = cpu.read_registerh(REG_HL);
      uint8_t data = before + 1;
      cpu.write_registerh(REG_HL, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.reset_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, ((before & 0xF) == 0xF));
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
    } break;

    // JR Z,r8 2/12|8 ----
    case 0x28: {
      cpu.write_register(REG_PC, addr+1);
      uint16_t pc = cpu.read_register(REG_PC);
      bool zero_flag_is_set = cpu.flag_is_set(FLG_Z);
      // debug_print("\tZero flag %s set: %d\r\n", zero_flag_is_set ? "is" : "is not", zero_flag_is_set);
      if(zero_flag_is_set == 1) {
        int8_t jmp_offset = (int8_t)mem.read8(pc);
        // debug_print("\tJR %#04x + (%d) = %#04x\r\n", pc, jmp_offset, (pc+1)-jmp_offset);
        cpu.write_register(REG_PC, (pc+1)+jmp_offset);
        cpu.cycles=12;
      } else {
        // debug_print("\tNot JMP: %#04x\r\n", pc+1);
        cpu.write_register(REG_PC, pc+1);
        cpu.cycles=8;
      }
    } break;

    // LD A,(HL+)
    case 0x2A: {
      mem_addr_t addr = cpu.read_register(REG_HL);
      uint8_t val = mem.read16(addr);
      cpu.write_registerh(REG_AF, val);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=8;
    } break;

    // LD L,d8 2/8 ----
    case 0x2E: {
      cpu.write_register(REG_PC, addr+1);
      uint8_t data = mem.read8(addr+1);
      cpu.write_registerl(REG_HL, data);
      cpu.write_register(REG_PC, addr+2);
      cpu.cycles=8;
    } break;

    // LD SP,d16 3/12 ----
    case 0x31: {
      uint16_t pc = cpu.read_register(REG_PC);
      uint16_t val = mem.read16(pc+1);
      cpu.write_register(REG_SP, val);
      cpu.write_register(REG_PC, pc+3);
      cpu.cycles=12;
    } break;

    // LD (HL-),A 1/8 ----
    case 0x32: {
      mem_addr_t write_addr = cpu.read_register(REG_HL);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(write_addr, data);
      cpu.write_register(REG_HL, write_addr-1);
      cpu.write_register(REG_PC, cpu.read_register(REG_PC) + 1);
      cpu.cycles=8;
    } break;

    // DEC A 1/4 Z1H-
    case 0x3D: {
      uint8_t before = cpu.read_registerh(REG_AF);
      uint8_t data = before-1;
      cpu.write_registerh(REG_AF, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.set_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, (before & 0x0F) == 0);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
    } break;

    // LD A,d8 2/8 ----
    case 0x3E: {
      addr = cpu.read_register(REG_PC) + 1;
      uint8_t data = mem.read8(addr);
      cpu.write_registerh(REG_AF, data);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=8;
    } break;

    // LD C,A 1/4 ----
    case 0x4F: {
      cpu.write_registerl(REG_BC, cpu.read_registerh(REG_AF));
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=4;
    } break;

    // LD D,A 1/4 ----
    case 0x57: {
      cpu.write_registerl(REG_DE, cpu.read_registerh(REG_AF));
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=4;
    } break;

    // LD H,A 1/4 ----
    case 0x67: {
      cpu.write_registerh(REG_HL, cpu.read_registerh(REG_AF));
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=4;
    } break;

    // LD (HL),A 1/8 ----
    case 0x77: {
      mem_addr_t write_addr = cpu.read_register(REG_HL);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(write_addr, data);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=8;
    } break;

    // LD A,E 1/4 ----
    case 0x7B: {
      cpu.write_registerh(REG_AF, cpu.read_registerl(REG_DE));
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=4;
    } break;

    // LD A,H 1/4 ----
    case 0x7C: {
      cpu.write_registerh(REG_AF, cpu.read_registerh(REG_HL));
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles=4;
    } break;

    // SUB B 1/4 Z1HC
    case 0x90: {
      uint8_t before = cpu.read_registerh(REG_AF);
      uint8_t to_subtract = cpu.read_registerh(REG_BC);

      uint8_t result = before;
      result -= to_subtract;
      cpu.write_registerh(REG_AF, result);
      cpu.write_registerl(REG_AF, 0);

      if(result == 0) cpu.set_flag(FLG_Z);
      cpu.set_flag(FLG_N);
      if(before < to_subtract) cpu.set_flag(FLG_C);

      int8_t htest = (int8_t)(before & 0xF);
      htest -= (to_subtract & 0xF) ;
      if (htest < 0) cpu.set_flag(FLG_H);

      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles += 4;
    } break;

    // XOR A 1/4 z000
    case 0xAF: {
      uint8_t val = cpu.read_registerh(REG_AF);
      cpu.xora(val);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles += 4;
    } break;

    // POP BC 1/12 ----
    case 0xC1: {
      mem_addr_t stack_addr = cpu.read_register(REG_SP);
      uint16_t data = mem.read16(stack_addr);
      cpu.write_register(REG_BC, data);
      cpu.write_register(REG_SP, cpu.read_register(REG_SP) + 2);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles += 12;
    } break;

    // PUSH BC 1/16 ----
    case 0xC5: {
      cpu.write_register(REG_PC, addr+1);
      mem_addr_t stack_addr = cpu.read_register(REG_SP);
      mem.write8(stack_addr, cpu.read_registerl(REG_BC));
      mem.write8(stack_addr-1, cpu.read_registerh(REG_BC));
      cpu.write_register(REG_SP, cpu.read_register(REG_SP) - 2);
      cpu.cycles += 16;
    } break;

    // RET 1/16 ----
    case 0xC9: {
      cpu.write_register(REG_PC, addr+1);
      cpu.write_register(REG_SP, cpu.read_register(REG_SP) + 2);

      mem_addr_t ret_addr_addr = cpu.read_register(REG_SP);
      uint16_t ret_addr = mem.read16(ret_addr_addr);
      cpu.write_register(REG_PC, ret_addr);
      cpu.cycles += 16;
      // debug_print("\t RETURN TO: SP:(%#04x) => %#04x\r\n", ret_addr_addr, ret_addr);
    } break;

    // PREFIX CB 1/4 ----
    case 0xCB: {
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles=4;
      addr = cpu.read_register(REG_PC);
      instr = mem.read8(addr);
      switch(instr) {
        // RL C 2/8 Z00C
        case 0x11: {
          uint8_t reg = cpu.read_registerl(REG_BC);
          bool is_carry_set = cpu.read_flag(FLG_C);
          bool is_msb_set = nth_bit(reg, 7);
          cpu.write_registerl(REG_AF, 0);
          uint8_t value = reg;
          value <<= 1;

          if(is_msb_set)
            cpu.set_flag(FLG_C);

          if(is_carry_set)
            value = set_bit(value, 0);

          if (value == 0)
            cpu.set_flag(FLG_Z);

          cpu.write_registerl(REG_BC, value);
          cpu.write_register(REG_PC, addr+1);
          cpu.cycles=4;
        } break;

        // BIT 7,H 2/8 Z01-
        case 0x7C: {
          uint8_t value = cpu.read_registerh(REG_HL);
          cpu.bit(value, 7);
          cpu.write_register(REG_PC, addr+1);
          cpu.cycles=4;
        } break;
        default: {
          // debug_print("Unknown 0xCB PREFIX instruction: %#04x at address: %#04x  \r\n", instr, addr);
          running = false;
        }
      }
    } break;

    // CALL a16 3/24 ----
    case 0xCD: {
      mem_addr_t call_addr = mem.read16(addr+1);
      cpu.write_register(REG_PC, addr+3);
      mem_addr_t stack_addr = cpu.read_register(REG_SP);

      mem.write8(stack_addr, cpu.read_registerl(REG_PC));
      mem.write8(stack_addr-1, cpu.read_registerh(REG_PC));
      // mem_addr_t return_addr = cpu.read_register(REG_PC);

      cpu.write_register(REG_SP, cpu.read_register(REG_SP) - 2);
      cpu.write_register(REG_PC, call_addr);
      cpu.cycles += 24;
      // debug_print("CALL: %#04x (%#04x)\r\n", call_addr, return_addr);
    } break;

    // LDH (a8),A 2/12 ---
    // LLD ($FF00+a8),A 2/12 ---
    case 0xE0: {
      mem_addr_t write_addr = 0xFF00 + mem.read8(addr + 1);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(write_addr, data);
      cpu.write_register(REG_PC, addr+2);
      cpu.cycles += 12;
    } break;

    // LD (C),A 2/8 ----
    case 0xE2: {
      cpu.write_register(REG_PC, addr+2);
      addr = 0xFF00 + cpu.read_registerl(REG_BC);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(addr, data);
      // cpu.write_registerh(REG_AF, data);
      cpu.cycles=8;
    } break;

    // LD (a16),A 3/16 ----
    case 0xEA: {
      cpu.write_register(REG_PC, addr+1);
      mem_addr_t load_addr = mem.read16(addr+1);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(load_addr, data);

      cpu.write_register(REG_PC, addr+3);
      cpu.cycles=16;
    } break;

    // LDH A,(a8) 2/12 ----
    // LD A,($FF00+a8) 2/12 ----
    case 0xF0: {
      cpu.write_register(REG_PC, addr+1);
      mem_addr_t read_addr = 0xFF00+mem.read8(cpu.read_register(REG_PC));
      uint8_t data = mem.read8(read_addr);
      cpu.write_registerh(REG_AF, data);
      cpu.write_register(REG_PC, addr+2);
      cpu.cycles=12;
    } break;

    // CP d8 2/8 Z1HC
    case 0xFE: {
      cpu.write_register(REG_PC, addr+1);
      uint8_t to_subtract = mem.read8(cpu.read_register(REG_PC));
      uint8_t reg = cpu.read_registerh(REG_AF);
      uint8_t before = reg;
      reg-=to_subtract;
      cpu.write_registerl(REG_AF, 0);

      if(reg == 0) cpu.set_flag(FLG_Z);
      cpu.set_flag(FLG_N);

      if(before < to_subtract) cpu.set_flag(FLG_C);

      // TODO(Cononor) - I have no idea if this is correct.
      int8_t htest = (int8_t)before & 0xF;
      htest -= (to_subtract & 0xF);
      if(htest < 0) cpu.set_flag(FLG_H);

      cpu.write_register(REG_PC, addr+2);
      cpu.cycles=8;
    } break;

    default: {
      debug_print("Unknown instruction: %#04x at address: %#04x  \r\n", instr, addr);
      running = false;
    } break;
  }

  if(addr == cpu.read_register(REG_PC) && running) {
    debug_print("YOU FORGOT TO INC PC!!!\r\n");
    running = false;
  }

}
