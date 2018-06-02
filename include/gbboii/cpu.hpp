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

typedef enum FLAG_NAMES {
  FLG_Z = 3,
  FLG_N = 2,
  FLG_H = 1,
  FLG_C = 0
} flag_name_t;

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
  uint8_t read_registerh(reg_name_t reg_name);
  uint8_t read_registerl(reg_name_t reg_name);
  void write_register(reg_name_t reg_name, uint16_t value);
  void write_registerh(reg_name_t reg_name, uint8_t value);
  void write_registerl(reg_name_t reg_name, uint8_t value);

  void set_flag(flag_name_t flag);
  void reset_flag(flag_name_t flag);
  void write_flag(flag_name_t flag, bool value);
  bool read_flag(flag_name_t flag);
  bool flag_is_set(flag_name_t flag);
  bool flag_is_reset(flag_name_t flag);

  void xora(uint8_t val);

  void bit(uint8_t val, int n);
};


#endif
