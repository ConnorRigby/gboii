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
  debug_print("hello??\r\n");
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
  //memcpy(&mem[addr], &data, sizeof(uint8_t));
  mem[addr] = data;
}

void Memory::inspect() {
  debug_print_q("\r\n\e[39m 0x0000: ");
  for(int addr = 0; addr<MEMORY_SIZE; addr++) {
    if(((addr % 20) == 0) && (addr != 0)) debug_print_q("\r\n\e[39m 0x%04x: ", addr + 20);
    debug_print_q("e[34m 0x%04x e[39m", read16(addr));
  }
  debug_print_q("\r\n");
}
