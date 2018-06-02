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

void Memory::write8(mem_addr_t address, uint8_t data) {
  // dont allow any writing to the read only memory
   if ( address < 0x8000 ) {
     debug_print("Memory write below 0x8000 forbidden\r\n");
   }

   // writing to ECHO ram also writes in RAM
   else if ( ( address >= 0xE000 ) && (address < 0xFE00) )  {
     mem[address] = data ;
     write8(address-0x2000, data) ;
   }

   // this area is restricted
   else if ( ( address >= 0xFEA0 ) && (address < 0xFEFF) ) {
     debug_print("Memory write between 0xFEA0 and 0xFEFF is forbidden\r\n");
   }

   //trap the divider register
   else if (address == 0xFF04) {
     mem[0xFF04] = 0;
   }

   // reset the current scanline if the game tries to write to it
   else if (address == 0xFF44) {
     mem[address] = 0 ;
   }

   else if (address == 0xFF46) {
     do_dma_transfer(data);
   }

   // no control needed over this area so write to memory
   else {
     mem[address] = data ;
   }
}

void Memory::do_dma_transfer(uint8_t data) {
  uint16_t address = data << 8 ; // source address is data * 100
  for (uint8_t i = 0; i < 0xA0; i++) {
    write8(0xFE00+i, read8(address + i));
  }
}

void Memory::inspect() {
  debug_print_q("\r\n\e[39m 0x0000: ");
  for(int addr = 0; addr<MEMORY_SIZE; addr++) {
    if(((addr % 20) == 0) && (addr != 0)) debug_print_q("\r\n\e[39m 0x%04x: ", addr + 20);
    debug_print_q("e[34m 0x%04x e[39m", read16(addr));
  }
  debug_print_q("\r\n");
}
