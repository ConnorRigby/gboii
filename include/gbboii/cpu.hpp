#ifndef LIBGBBOII_CPU_H
#define LIBGBBOII_CPU_H

#include <inttypes.h>
#include <cstddef>

typedef uint16_t gb_register_t;
typedef uint8_t opcode_t;

typedef enum REG_NAMES {
  REG_AF,
  REG_BC,
  REG_DE,
  REG_HL,
  REG_SP,
  REG_PC
} reg_name_t;

class CPU {
private:
  gb_register_t AF;
  gb_register_t BC;
  gb_register_t DE;
  gb_register_t HL;
  gb_register_t SP;
  gb_register_t PC;

public:
  size_t cycles;

  CPU();
  uint16_t read_register(reg_name_t reg_name);
  void write_register(reg_name_t reg_name, uint16_t value);
  void write_registerh(reg_name_t reg_name, uint8_t value);
  void write_registerl(reg_name_t reg_name, uint8_t value);
};


#endif
