#ifndef LIBGBBOII_MEMORY_H
#define LIBGBBOII_MEMORY_H

#include <inttypes.h>

#define MEMORY_SIZE 0xFFFF

typedef uint16_t mem_addr_t;

class Memory {
private:
  uint8_t mem[MEMORY_SIZE];
public:
  Memory(char*);
  uint8_t read8(mem_addr_t addr);
  uint16_t read16(mem_addr_t addr);

  void write8(mem_addr_t addr, uint8_t data);
};


#endif
