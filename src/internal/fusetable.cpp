#include "fusetable.hpp"

namespace fipsy {

uint16_t
FuseTable::computeChecksum() const {
  uint16_t c = 0x0000;
  for (size_type i = 0; i < size(); ++i) {
    c += static_cast<uint16_t>((*this)[i]) << (i % 8);
  }
  return c;
}

} // namespace fipsy
