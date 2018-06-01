#include <gbboii/utils.hpp>
#include <climits>
#include <cstdint>
#include <type_traits>
#define GB_SINT8_MAX INT_MAX
#define GB_SINT8_MIN INT_MIN

uint16_t write_high_byte(uint16_t current, uint8_t value) {
  return (current & ~(0xFF << (1 * 8))) | (value << (1 * 8));
}

uint16_t write_low_byte(uint16_t current, uint8_t value) {
  return (current & ~(0xFF << (0 * 8))) | (value << (0 * 8));
}

uint8_t read_high_byte(uint16_t data) {
  return data >> 8;
}

uint8_t read_low_byte(uint16_t data) {
  return data & 0xFF;
}


uint8_t set_bit(uint8_t number, int n) {
  return (number | (1 << n));
}

uint8_t reset_bit(uint8_t number, int n) {
  return (number | (0 << n));
}


int nth_bit(uint8_t number, int n) {
  return ( (number >> n) & 1);
  // return (number & ( 1 << n)) >> n;
}

uint8_t signed_int8(uint8_t number) {
  // return (uint16_t)(int8_t)number;
  return -std::uintmax_t(number);
}
