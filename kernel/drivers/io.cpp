#include <stdint.h>

namespace Kernel::Drivers::IO {

// 8-bit IO
uint8_t in8(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void out8(uint16_t port, uint8_t value) {
  asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// 16-bit IO
uint16_t in16(uint16_t port) {
  uint16_t ret;
  asm volatile("inw %%dx, %%ax" : "=a"(ret) : "d"(port));
  return ret;
}

void out16(uint16_t port, uint16_t value) {
  asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

// 32-bit IO
uint32_t in32(uint16_t port) {
  uint32_t ret;
  asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void out32(uint16_t port, uint32_t value) {
  asm volatile("outl %0, %1" ::"a"(value), "Nd"(port));
}
} // namespace Kernel::Drivers::IO

void outb(uint16_t port, uint8_t val) { Kernel::Drivers::IO::out8(port, val); }

void outw(uint16_t port, uint16_t val) {
  Kernel::Drivers::IO::out16(port, val);
}

uint8_t inb(uint16_t port) { return Kernel::Drivers::IO::in8(port); }

uint16_t inw(uint16_t port) { return Kernel::Drivers::IO::in16(port); }
