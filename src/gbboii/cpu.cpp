#include <gbboii/cpu.hpp>

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
// void CPU::write_registerh(reg_name_t reg_name, uint8_t value) {
// }
// void CPU::write_registerl(reg_name_t reg_name, uint8_t value) {
// }
