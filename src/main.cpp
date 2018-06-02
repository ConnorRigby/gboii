#include <gbboii/gbboii.hpp>
#include <gbboii/utils.hpp>

#include <stdio.h>
#include <cstdlib>
#include <string.h>


#include <SDL2/SDL.h>
#include "gpu/gui.hpp"
#include "gpu/gui.cpp"
#include "gpu/gpu.hpp"
#include "gpu/gpu.cpp"

int load_file(const char* filename, char** buffer);
int main(int argc, char const *argv[]);

int fc = 0;
void emulate(Gameboy* c, gpu* g) {
    //int max_cycles = 69905; //  frequency of gameboy / 60
    int cycles = 0;
    c->tick();
    g->step();

    cycles += c->cpu.cycles;
    fc++;
    //printf("fc = %d\n", fc);
}

int main(int argc, char const *argv[]) {
  if(argc != 2) {
    debug_print("usage: %s bootrom.bin rom.gb\r\n", argv[0]);
    return(1);
  }

  char* bootrom_buffer = NULL;
  if(load_file(argv[1], &bootrom_buffer) != 0) {
    debug_print("Failed to open bootrom.\r\n");
    return -1;
  }

  if(bootrom_buffer == NULL) {
    debug_print("bootrom failed to read\r\n");
    return -1;
  }

  Gameboy gb(bootrom_buffer);
  free(bootrom_buffer);
  gui screen;
  gpu g(&gb, &screen);

  screen.init();

  int quit = 0;
  SDL_Event e;
  //LTimer fps;
  while (!quit)
  {
      while(SDL_PollEvent(&e) != 0)
      {
          //User requests quit
          if(e.type == SDL_QUIT)
          {
              quit = 1;
          }
      }
      emulate(&gb, &g);
  }
  return 0;
}

int load_file(const char* filename, char** buffer) {
  FILE *fp = fopen(filename, "r");
  if (fp != NULL) {
      /* Go to the end of the file. */
      if (fseek(fp, 0L, SEEK_END) == 0) {
          /* Get the size of the file. */
          size_t bufsize = ftell(fp);
          if (bufsize == (size_t)-1) { return -1; }

          /* Allocate our buffer to that size. */
          debug_print("Allocating file buffer\r\n");
          *buffer = (char*) malloc(sizeof(char) * (bufsize));

          /* Go back to the start of the file. */
          if (fseek(fp, 0L, SEEK_SET) != 0) { return -1; }

          /* Read the entire file into memory. */
          size_t new_len = fread(*buffer, sizeof(char), bufsize, fp);
          if(new_len != bufsize) {
            debug_print("Failed to read file: %ld\r\n", new_len);
            return -1;
          }
      }
      fclose(fp);
      return 0;
  }
  return -1;
}
