#ifndef FIPSY_FEATURES_HPP
#define FIPSY_FEATURES_HPP

#include <Print.h>
#include <Printable.h>
#include <array>
#include <cstdint>

namespace fipsy {

/** @brief MachXO2 Feature Row and FEABITS. */
class Features
  : public std::array<uint8_t, 10>
  , public Printable {
public:
  Features() {
    fill(0);
  }

  uint64_t featureRow() const {
    return (static_cast<uint64_t>(at(0)) << 56) | (static_cast<uint64_t>(at(1)) << 48) |
           (static_cast<uint64_t>(at(2)) << 40) | (static_cast<uint64_t>(at(3)) << 32) |
           (static_cast<uint64_t>(at(4)) << 24) | (static_cast<uint64_t>(at(5)) << 16) |
           (static_cast<uint64_t>(at(6)) << 8) | (static_cast<uint64_t>(at(7)) << 0);
  }

  uint16_t feabits() const {
    return (static_cast<uint16_t>(at(8)) << 8) | (static_cast<uint16_t>(at(9)) << 0);
  }

  bool hasSlaveSPI() const {
    return (at(9) & 0x40) == 0;
  }

  size_t printTo(Print& p) const override {
    return p.printf("%02x%02x%02x%02x%02x%02x%02x%02x/%02x%02x", at(0), at(1), at(2), at(3), at(4),
                    at(5), at(6), at(7), at(8), at(9));
  }
};

} // namespace fipsy

#endif // FIPSY_FEATURES_HPP
