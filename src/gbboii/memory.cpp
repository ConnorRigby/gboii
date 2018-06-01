#include <gbboii/memory.hpp>
#include <gbboii/utils.hpp>

#include <string.h>

Memory::Memory(char* bootrom) {
  for(int i = 0; i<MEMORY_SIZE; i++) {
    if(i <= 256) {
      mem[i] = bootrom[i];
    } else {
      mem[i] = 0;
    }
  }
}

uint8_t Memory::read8(mem_addr_t addr)  {
  return mem[addr];
}

uint16_t Memory::read16(mem_addr_t addr) {
  // TODO(Connor) - Is this slow?
  int16_t result;
  memcpy(&result, &mem[addr], sizeof(int16_t));
  return result;
}

void Memory::write8(mem_addr_t addr, uint8_t data) {
  mem[addr] = data;
}
