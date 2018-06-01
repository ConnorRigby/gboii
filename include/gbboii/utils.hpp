#ifndef GBBOII_UTILS_H
#define GBBOII_UTILS_H

#include <inttypes.h>
#include <stdbool.h>

#if defined(DEBUG)

  #include <stdio.h>
  #define debug_print(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
      __FILE__, __LINE__, __func__, ##args)

  #define debug_print_q(fmt, args...) fprintf(stderr, fmt, ##args)

#else

  #define debug_print(fmt, args...)
  #define debug_print_q(fmt, args...)

#endif

uint16_t write_high_byte(uint16_t current, uint8_t value);
uint16_t write_low_byte(uint16_t current, uint8_t value);

uint8_t read_high_byte(uint16_t data);
uint8_t read_low_byte(uint16_t data);

uint8_t set_bit(uint8_t data, int n);
uint8_t reset_bit(uint8_t data, int n);

bool nth_bit(uint8_t number, int n);

uint8_t signed_int8(uint8_t number);

#endif
