#ifndef LIBGBBOII_H
#define LIBGBBOII_H

#include <gbboii/cpu.hpp>

#include <stdbool.h>

class Gameboy {
private:
  CPU cpu;
  Memory mem;
public:
  bool running;
  Gameboy();
};


#endif
