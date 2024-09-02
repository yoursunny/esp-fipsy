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
  WRONG_CHECKSUM,
};

/** @brief Parse fuse table from JEDEC file. */
JedecError
parseJedec(Stream& input, FuseTable& fuseTable);

} // namespace fipsy

#endif // FIPSY_JEDEC_HPP
