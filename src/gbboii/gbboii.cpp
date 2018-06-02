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
  debug_print("Gameboy tick: instr: %#04x PC: %#04x\r\n", instr, addr);

  switch(instr) {

    // DEC B 1/4 z1h-
    case 0x05: {
      uint8_t data = cpu.read_registerh(REG_BC)-1;
      cpu.write_registerh(REG_BC, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.set_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, ((data & 0xF) == 0xF));
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=4;
    } break;

    // LD B,d8 2/8 ----
    case 0x06: {
      cpu.write_register(REG_PC, addr+1);
      addr = cpu.read_register(REG_PC);
      uint8_t data = mem.read8(addr);
      cpu.write_registerh(REG_BC, data);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=8;
    } break;

    // DEC C 1/4 Z1H-
    case 0x0D: {
      uint8_t data = cpu.read_registerl(REG_BC)-1;
      cpu.write_registerl(REG_BC, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.set_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, ((data & 0xF) == 0xF));
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=4;
    } break;

    // LD C,d8 2/8 ----
    case 0x0E: {
      cpu.write_register(REG_PC, addr+1);
      addr = cpu.read_register(REG_PC);
      uint8_t data = mem.read8(addr);
      cpu.write_registerl(REG_BC, data);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=8;
    } break;

    // LD DE,d16 3/16 ----
    case 0x11: {
      uint16_t data = mem.read16(addr+1);
      cpu.write_register(REG_DE, data);
      cpu.write_register(REG_PC, addr+3);
      cpu.cycles+=16;
    } break;

    // INC DE 1/8 ----
    case 0x13: {
      cpu.write_register(REG_DE, cpu.read_register(REG_DE)-1);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=8;
    } break;

    // RLA 1/4 000C
    case 0x17: {
      bool carry = cpu.read_flag(FLG_C);
      uint8_t reg_value = cpu.read_registerh(REG_AF);
      uint8_t reg_shift = ((reg_value << 1) | (reg_value >> (8 - 1))) & 255; /* (0b11111111) */
      reg_shift = write_bit(reg_shift, 0, carry);
      cpu.write_flag(FLG_Z, nth_bit(reg_value, 7) == 0);
      cpu.reset_flag(FLG_H);
      cpu.reset_flag(FLG_N);
      cpu.write_flag(FLG_C, nth_bit(reg_value, 7));
      cpu.write_registerl(REG_BC, reg_shift);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=4;
    } break;

    // JR r8 2/12 ----
    case 0x18: {
      cpu.write_register(REG_PC, addr+1);
      uint16_t pc = cpu.read_register(REG_PC);
      // TODO(Connor) not sure why i have to do this negative thing.
      uint8_t jmp_offset = signed_int8(mem.read8(pc));
      // debug_print("\tJR %#04x + (%d) = %#04x\r\n", pc, jmp_offset, (pc+1)-jmp_offset);
      cpu.write_register(REG_PC, (pc+1)-jmp_offset);
      cpu.cycles+=12;
    } break;

    // LD A,(DE) 1/8 ----
    case 0x1A: {
      mem_addr_t read_addr = cpu.read_register(REG_DE);
      uint8_t data = mem.read8(read_addr);
      cpu.write_registerh(REG_AF, data);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=8;
    } break;

    //JR NZ,r8 2/12|8 ----
    case 0x20: {
      cpu.write_register(REG_PC, addr+1);
      uint16_t pc = cpu.read_register(REG_PC);
      bool zero_flag_is_reset = cpu.flag_is_reset(FLG_Z);
      // debug_print("\tZero flag %s reset: %d\r\n", zero_flag_is_reset ? "is" : "is not", zero_flag_is_reset);
      if(zero_flag_is_reset == 1) {
        // TODO(Connor) not sure why i have to do this negative thing.
        uint8_t jmp_offset = signed_int8(mem.read8(pc));
        // debug_print("\tJR %#04x + (%d) = %#04x\r\n", pc, jmp_offset, (pc+1)-jmp_offset);
        cpu.write_register(REG_PC, (pc+1)-jmp_offset);
        cpu.cycles+=12;
      } else {
        // debug_print("\tNot JMP: %#04x\r\n", pc+1);
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

    // LD (HL+),A 1/8 ----
    case 0x22: {
      mem_addr_t load_addr = cpu.read_register(REG_HL);
      mem.write8(load_addr, cpu.read_registerh(REG_AF));
      cpu.write_register(REG_HL, load_addr + 1);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles+=8;
    } break;

    // INC HL 1/8 ----
    case 0x23: {
      cpu.write_register(REG_HL, cpu.read_register(REG_HL) + 1);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles+=8;
    } break;

    // JR Z,r8 2/12|8 ----
    case 0x28: {
      cpu.write_register(REG_PC, addr+1);
      uint16_t pc = cpu.read_register(REG_PC);
      bool zero_flag_is_set = cpu.flag_is_set(FLG_Z);
      // debug_print("\tZero flag %s set: %d\r\n", zero_flag_is_set ? "is" : "is not", zero_flag_is_set);
      if(zero_flag_is_set == 1) {
        // TODO(Connor) not sure why i have to do this negative thing.
        uint8_t jmp_offset = signed_int8(mem.read8(pc));
        // debug_print("\tJR %#04x + (%d) = %#04x\r\n", pc, jmp_offset, (pc+1)-jmp_offset);
        cpu.write_register(REG_PC, (pc+1)-jmp_offset);
        cpu.cycles+=12;
      } else {
        // debug_print("\tNot JMP: %#04x\r\n", pc+1);
        cpu.write_register(REG_PC, pc+1);
        cpu.cycles+=8;
      }
    } break;

    // LD A,(HL+)
    case 0x2A: {
      mem_addr_t addr = cpu.read_register(REG_HL);
      uint8_t val = mem.read16(addr);
      cpu.write_registerh(REG_AF, val);
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=8;
    } break;

    // LD L,d8 2/8 ----
    case 0x2E: {
      cpu.write_register(REG_PC, addr+1);
      uint8_t data = mem.read8(addr+1);
      cpu.write_registerl(REG_HL, data);
      cpu.write_register(REG_PC, addr+2);
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
      mem.write8(write_addr, data);
      cpu.write_register(REG_HL, write_addr-1);
      cpu.write_register(REG_PC, cpu.read_register(REG_PC) + 1);
      cpu.cycles+=8;
    } break;

    // DEC A 1/4 Z1H-
    case 0x3D: {
      uint8_t data = cpu.read_registerh(REG_AF)-1;
      cpu.write_registerh(REG_AF, data);
      cpu.write_flag(FLG_Z, data == 0);
      cpu.set_flag(FLG_N);
      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, ((data & 0xF) == 0xF));
      cpu.write_register(REG_PC, addr+1);
      cpu.cycles+=4;
    } break;

    // LD A,d8 2/8 ----
    case 0x3E: {
      addr = cpu.read_register(REG_PC) + 1;
      uint8_t data = mem.read8(addr);
      cpu.write_registerh(REG_AF, data);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles+=8;
    } break;

    // LD C,A 1/4 ----
    case 0x4F: {
      cpu.write_registerl(REG_BC, cpu.read_registerh(REG_AF));
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles+=4;
    } break;

    // LD (HL),A 1/8 ----
    case 0x77: {
      mem_addr_t write_addr = cpu.read_register(REG_HL);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(write_addr, data);
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles+=8;
    } break;

    // LD A,E 1/4 ----
    case 0x7B: {
      cpu.write_registerh(REG_AF, cpu.read_registerl(REG_DE));
      cpu.write_register(REG_PC, addr + 1);
      cpu.cycles+=4;
    } break;

    // XOR A 1/4 z000
    case 0xAF: {
      uint8_t val = cpu.read_registerh(REG_AF);
      cpu.xora(val);
      cpu.write_register(REG_PC, cpu.read_register(REG_PC) + 1);
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
      cpu.cycles+=4;
      addr = cpu.read_register(REG_PC);
      instr = mem.read8(addr);
      switch(instr) {
        // RL C 2/8 Z00C
        case 0x11: {
          bool carry = cpu.read_flag(FLG_C);
          uint8_t reg_value = cpu.read_registerl(REG_BC);
          uint8_t reg_shift = ((reg_value << 1) | (reg_value >> (8 - 1))) & 255; /* (0b11111111) */
          reg_shift = write_bit(reg_shift, 0, carry);
          cpu.write_flag(FLG_Z, nth_bit(reg_value, 7) == 0);
          cpu.reset_flag(FLG_H);
          cpu.reset_flag(FLG_N);
          cpu.write_flag(FLG_C, nth_bit(reg_value, 7));
          cpu.write_registerl(REG_BC, reg_shift);
          cpu.write_register(REG_PC, addr+1);
          cpu.cycles+=4;
        } break;

        // BIT 7,H 2/8 Z01-
        case 0x7C: {
          uint8_t value = cpu.read_registerh(REG_HL);
          cpu.bit(value, 7);
          cpu.write_register(REG_PC, addr+1);
          cpu.cycles+=4;
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
      mem_addr_t return_addr = cpu.read_register(REG_PC);

      cpu.write_register(REG_SP, cpu.read_register(REG_SP) - 2);
      cpu.write_register(REG_PC, call_addr);
      cpu.cycles += 24;
      debug_print("CALL: %#04x (%#04x)\r\n", call_addr, return_addr);
    } break;

    // LDH (a8),A 2/12 ---
    // LD A,($FF00+a8) 2/12 ---
    case 0xE0: {
      mem_addr_t write_addr = 0xFF00 + mem.read8(addr + 1);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(write_addr, data);
      cpu.write_register(REG_PC, addr+2);
      cpu.cycles += 4;
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

    // LD (a16),A 3/16 ----
    case 0xEA: {
      cpu.write_register(REG_PC, addr+1);
      mem_addr_t load_addr = mem.read16(addr+1);
      uint8_t data = cpu.read_registerh(REG_AF);
      mem.write8(load_addr, data);

      cpu.write_register(REG_PC, addr+3);
      cpu.cycles+=16;
    } break;

    // CP d8 2/8 Z1HC
    case 0xFE: {
      cpu.write_register(REG_PC, addr+1);
      uint8_t data = mem.read8(cpu.read_register(REG_PC));
      uint8_t a_reg_val = cpu.read_registerh(REG_AF);

      cpu.write_flag(FLG_Z, data == a_reg_val);
      cpu.write_flag(FLG_C, a_reg_val < data);
      cpu.set_flag(FLG_N);

      // TODO(Cononor) - I have no idea if this is correct.
      cpu.write_flag(FLG_H, ( (((data+a_reg_val) & 0xf) - (1 & 0xf)) & 0x10) == 0x10);

      cpu.write_register(REG_PC, addr+2);
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
