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

  bool hasSlaveSPI() const {
    return (at(9) & 0x40) != 0;
  }

  size_t printTo(Print& p) const override {
    return p.printf("%02x%02x%02x%02x%02x%02x%02x%02x/%02x%02x", at(0), at(1), at(2), at(3), at(4),
                    at(5), at(6), at(7), at(8), at(9));
  }
};

} // namespace fipsy

#endif // FIPSY_FEATURES_HPP
