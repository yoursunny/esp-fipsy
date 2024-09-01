#ifndef FIPSY_FUSETABLE_HPP
#define FIPSY_FUSETABLE_HPP

#include <Stream.h>
#include <bitset>
#include <cstdint>
#include <vector>

namespace fipsy {

/**  @brief Fuse table of MachXO2 series. */
class FuseTable : public std::vector<bool> {
public:
  /**  @brief Compute fuse checksum. */
  uint16_t computeChecksum() const;
};

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

#endif // FIPSY_FUSETABLE_HPP
