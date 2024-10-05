#ifndef FIPSY_JEDEC_HPP
#define FIPSY_JEDEC_HPP

#include "features.hpp"
#include "fusetable.hpp"
#include <Stream.h>

namespace fipsy {

enum class JedecError {
  OK,
  NO_STX,
  NO_ETX,
  BAD_QF,
  BAD_F,
  BAD_L,
  BAD_C,
  BAD_E,
  WRONG_CHECKSUM,
};

class JedecContent {
public:
  explicit operator bool() const {
    return error == JedecError::OK;
  }

public:
  JedecError error = JedecError::NO_STX;
  Features features;
};

/** @brief Parse JEDEC file. */
JedecContent
parseJedec(Stream& input, FuseTable& fuseTable);

} // namespace fipsy

#endif // FIPSY_JEDEC_HPP
