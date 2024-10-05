#ifndef FIPSY_STATUS_HPP
#define FIPSY_STATUS_HPP

#include <cstdint>

namespace fipsy {

/** @brief Status register value. */
class Status {
public:
  bool enabled() const {
    return v & (1 << 9);
  }

  bool busy() const {
    return v & (1 << 12);
  }

  bool fail() const {
    return v & (1 << 13);
  }

public:
  uint32_t v;
};

} // namespace fipsy

#endif // FIPSY_STATUS_HPP
