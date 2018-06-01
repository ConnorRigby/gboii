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
  write_registerl(REG_AF, set_bit(val, flag));
}

void CPU::reset_flag(flag_name_t flag) {
  uint8_t val = read_registerl(REG_AF);
  write_registerl(REG_AF, reset_bit(val, flag));
  debug_print("flag %d value: %d\r\n", flag, read_flag(flag));
}

int CPU::read_flag(flag_name_t flag) {
  uint8_t val = read_registerl(REG_AF);
  return nth_bit(val, flag);
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
  debug_print("BIT: %#02x %d: %d\r\n", val, n, result);
  if(result == 1) { reset_flag(FLG_Z); } else { set_flag(FLG_Z); }
  reset_flag(FLG_N);
  set_flag(FLG_H);
}
