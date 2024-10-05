#ifndef FIPSY_FUSETABLE_HPP
#define FIPSY_FUSETABLE_HPP

#include <cstdint>
#include <vector>

namespace fipsy {

/**  @brief Fuse table of MachXO2 series. */
class FuseTable : public std::vector<bool> {
public:
  /**  @brief Compute fuse checksum. */
  uint16_t computeChecksum() const;
};

} // namespace fipsy

#endif // FIPSY_FUSETABLE_HPP
