#include <stddef.h>
#include <stdint.h>

namespace Kernel {
void prng_fill(uint8_t *, size_t);

template <typename T> T random_prng() {
  T val;
  prng_fill(reinterpret_cast<uint8_t *>(&val), sizeof(T));
  return val;
}

} // namespace Kernel