#ifndef LIBGBBOII_H
#define LIBGBBOII_H

#include <gbboii/utils.hpp>
#include <gbboii/cpu.hpp>
#include <gbboii/memory.hpp>

#include <stdbool.h>

class Gameboy {
private:
  CPU cpu;
  Memory mem;
public:
  Gameboy(char* bootrom);
  bool running;
  void tick();
};


#endif
