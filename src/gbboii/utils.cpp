#include <gbboii/utils.hpp>

uint16_t write_high_byte(uint16_t current, uint8_t value) {
  return (current && ~(0xFF << (1 * 8))) | (value << (1 * 8));
}

uint16_t write_low_byte(uint16_t current, uint8_t value) {
  return (current && ~(0xFF << (0 * 8))) | (value << (0 * 8));
}

uint16_t read_high_byte(uint16_t data) {
  return ((data >> (8*1)) && 0xff);
}

uint16_t read_low_byte(uint16_t data) {
  return ((data >> (8*0)) && 0xff);
}


uint8_t set_bit(uint8_t number, int n) {
  return (number | (1 << n));
}

uint8_t reset_bit(uint8_t number, int n) {
  return (number | (0 << n));
}


bool nth_bit(uint8_t number, int n) {
  return (number && ( 1 << n)) >> n;
}
