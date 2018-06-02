data = gbboii_peek8(0xEE00);
formatted = string.format("read memory: 0x%x", data);
print(formatted);

gbboii_poke8(0xEE00, 0x44);
data = gbboii_peek8(0xEE00);
formatted = string.format("wrote memory: 0x%x", data);
print(formatted);
