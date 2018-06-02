#include <gbboii/cpu.hpp>
#include <gbboii/utils.hpp>

CPU::CPU() {
  cycles = 0;
  AF=0;
  BC=0;
  DE=0;
  HL=0;
  SP=0;
  PC=0;
}

uint16_t CPU::read_register(reg_name_t reg_name) {
  switch(reg_name) {
    case REG_AF: return AF;
    case REG_BC: return BC;
    case REG_DE: return DE;
    case REG_HL: return HL;
    case REG_SP: return SP;
    case REG_PC: return PC;
  }
  return -1; // Shut up compiler.
}

uint8_t CPU::read_registerh(reg_name_t reg_name) {
  switch(reg_name) {
    case REG_AF: return read_high_byte(AF);
    case REG_BC: return read_high_byte(BC);
    case REG_DE: return read_high_byte(DE);
    case REG_HL: return read_high_byte(HL);
    case REG_SP: return read_high_byte(SP);
    case REG_PC: return read_high_byte(PC);
  }
  return -1; // Shut up compiler.
}

uint8_t CPU::read_registerl(reg_name_t reg_name) {
  switch(reg_name) {
    case REG_AF: return read_low_byte(AF);
    case REG_BC: return read_low_byte(BC);
    case REG_DE: return read_low_byte(DE);
    case REG_HL: return read_low_byte(HL);
    case REG_SP: return read_low_byte(SP);
    case REG_PC: return read_low_byte(PC);
  }
  return -1; // Shut up compiler.
}

void CPU::write_register(reg_name_t reg_name, uint16_t value) {
  switch(reg_name) {
    case REG_AF:
      AF=value;
    break;
    case REG_BC:
      BC=value;
    break;
    case REG_DE:
      DE=value;
    break;
    case REG_HL:
      HL=value;
    break;
    case REG_SP:
      SP=value;
    break;
    case REG_PC:
      PC=value;
    break;
  }
}

void CPU::write_registerh(reg_name_t reg_name, uint8_t value) {
  switch(reg_name) {
    case REG_AF: { AF=write_high_byte(AF, value); }break;
    case REG_BC: { BC=write_high_byte(BC, value); }break;
    case REG_DE: { DE=write_high_byte(DE, value); }break;
    case REG_HL: { HL=write_high_byte(HL, value); }break;
    case REG_SP: { SP=write_high_byte(SP, value); }break;
    case REG_PC: { PC=write_high_byte(PC, value); }break;
  }
}

void CPU::write_registerl(reg_name_t reg_name, uint8_t value) {
  switch(reg_name) {
    case REG_AF: { AF=write_low_byte(AF, value); }break;
    case REG_BC: { BC=write_low_byte(BC, value); }break;
    case REG_DE: { DE=write_low_byte(DE, value); }break;
    case REG_HL: { HL=write_low_byte(HL, value); }break;
    case REG_SP: { SP=write_low_byte(SP, value); }break;
    case REG_PC: { PC=write_low_byte(PC, value); }break;
  }
}

void CPU::set_flag(flag_name_t flag) {
  uint8_t val = read_registerl(REG_AF);
  uint8_t updated = set_bit(val, flag);
  // debug_print("setting flag %d=%d old: %d\r\n", flag, updated, val);
  write_registerl(REG_AF, updated);
}

void CPU::reset_flag(flag_name_t flag) {
  uint8_t val = read_registerl(REG_AF);
  uint8_t updated = reset_bit(val, flag);
  // debug_print("resetting flag %d=%d old: %d\r\n", flag, updated, val);
  write_registerl(REG_AF, updated);
}

void CPU::write_flag(flag_name_t flag, bool value) {
  if(value == 1) { set_flag(flag); } else
  if(value == 0) { reset_flag(flag); }
}

bool CPU::read_flag(flag_name_t flag) {
  uint8_t val = read_registerl(REG_AF);
  return nth_bit(val, flag);
}

bool CPU::flag_is_set(flag_name_t flag) {
  return read_flag(flag) == 1;
}

bool CPU::flag_is_reset(flag_name_t flag) {
  return read_flag(flag) == 0;
}

void CPU::xora(uint8_t val) {
  uint8_t result = read_registerh(REG_AF) ^ val;
  write_registerh(REG_AF, result);

  if(result == 0) {
    set_flag(FLG_Z);
  } else {
    reset_flag(FLG_Z);
  }

  reset_flag(FLG_N);
  reset_flag(FLG_H);
  reset_flag(FLG_C);
}

void CPU::bit(uint8_t val, int n) {
  int result = nth_bit(val, n);
  // debug_print("BIT: %#02x %d: %d\r\n", val, n, result);
  write_flag(FLG_Z, result == 0);
  if(result == 1) { reset_flag(FLG_Z); } else { set_flag(FLG_Z); }
  reset_flag(FLG_N);
  set_flag(FLG_H);
}
