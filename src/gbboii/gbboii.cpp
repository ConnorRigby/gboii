#include <gbboii/gbboii.hpp>
#include <gbboii/cpu.hpp>
#include <gbboii/memory.hpp>
#include <gbboii/utils.hpp>

Gameboy::Gameboy(char* bootrom) : mem(Memory(bootrom)) {
  debug_print("Gameboy init.\r\n");
  running = true;
}

void Gameboy::tick() {
  uint16_t addr = cpu.read_register(REG_PC);
  opcode_t instr = mem.read8(addr);
  debug_print("Gameboy tick: instr: %#04x %#04x\r\n", instr, addr);

  switch(instr) {
    // LD C,d8 2/8 ----
    case 0x0E: {
      cpu.write_register(REG_PC, addr+1);
      addr = cpu.read_register(REG_PC);
      uint8_t data = mem.read8(addr);
      cpu.write_registerl(REG_BC, data);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=8;
    } break;

    //JR NZ,r8 2/12|8 ----
    case 0x20: {
      cpu.write_register(REG_PC, addr+1);
      uint16_t pc = cpu.read_register(REG_PC);
      bool zero_flag_is_reset = cpu.flag_is_reset(FLG_Z);
      debug_print("\tZero flag %s reset: %d\r\n", zero_flag_is_reset ? "is" : "is not", zero_flag_is_reset);
      if(zero_flag_is_reset == 1) {
        // TODO(Connor) not sure why i have to do this negative thing.
        uint8_t jmp_offset = signed_int8(mem.read8(pc));
        debug_print("\tJR %#04x + (%d) = %#04x\r\n", pc, jmp_offset, (pc+1)-jmp_offset);
        cpu.write_register(REG_PC, (pc+1)-jmp_offset);
        cpu.cycles+=12;
      } else {
        debug_print("\tNot JMP: %#04x\r\n", pc+1);
        cpu.write_register(REG_PC, pc+1);
        cpu.cycles+=8;
      }
    } break;

    // LD HL,d16 3/12 ----
    case 0x21: {
      uint16_t pc = cpu.read_register(REG_PC);
      uint16_t val = mem.read16(pc+1);
      cpu.write_register(REG_HL, val);
      cpu.write_register(REG_PC, pc+3);
      cpu.cycles+=12;
    } break;

    // LD A,(HL+)
    case 0x2A: {
      mem_addr_t addr = cpu.read_register(REG_HL);
      uint8_t val = mem.read16(addr);
      cpu.write_registerh(REG_AF, val);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=8;
    } break;

    // LD SP,d16 3/12 ----
    case 0x31: {
      uint16_t pc = cpu.read_register(REG_PC);
      uint16_t val = mem.read16(pc+1);
      cpu.write_register(REG_SP, val);
      cpu.write_register(REG_PC, pc+3);
      cpu.cycles+=12;
    } break;

    // LD (HL-),A 1/8 ----
    case 0x32: {
      mem_addr_t write_addr = cpu.read_register(REG_HL);
      uint8_t data = cpu.read_registerh(REG_AF);
      // debug_print("write data: %#02x to address: %#04x\r\n", data, write_addr);
      // mem.write8(addr, data);

      cpu.write_register(REG_HL, write_addr-1);
      cpu.write_register(REG_PC, cpu.read_register(REG_PC) + 1);
      cpu.cycles+=8;
    } break;

    // LD A,d8 2/8 ----
    case 0x3E: {
      addr = cpu.read_register(REG_PC) + 1;
      uint8_t data = mem.read8(addr);
      cpu.write_registerh(REG_AF, data);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles+=8;
    } break;

    // XOR A 1/4 z000
    case 0xAF: {
      uint8_t val = cpu.read_registerh(REG_AF);
      cpu.xora(val);
      cpu.write_register(REG_PC, cpu.read_register(REG_PC) + 1);
      cpu.cycles += 4;
    } break;

    // PREFIX CB 1/4 ----
    case 0xCB: {
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=4;
      addr = cpu.read_register(REG_PC);
      instr = mem.read8(addr);
      switch(instr) {
        // BIT 7,H 2/8 Z01-
        case 0x7C: {
          uint8_t value = cpu.read_registerh(REG_HL);
          cpu.bit(value, 7);
          cpu.write_register(REG_PC, addr+1);
          cpu.cycles+=4;
        } break;
        default: {
          debug_print("Unknown instruction: %#04x at address: %#04x  \r\n", instr, addr);
          running = false;
        }
      }
    } break;

    // LD (C),A 2/8 ----
    case 0xE2: {
      cpu.write_register(REG_PC, addr+2);
      addr = 0xFF00 + cpu.read_registerl(REG_BC);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(addr, data);
      // cpu.write_registerh(REG_AF, data);
      cpu.cycles+=8;
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
