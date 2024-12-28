#ifndef FIPSY_VARIANTS_HPP
#define FIPSY_VARIANTS_HPP

#include <cstdint>

namespace fipsy {

struct Variant {
  uint32_t deviceId;
  uint32_t qf;
  const char* desc;
};

const Variant VARIANTS[] = {
  {0x012B8043, 73600, "FipsyV1"},
  {0x012BA043, 343936, "FipsyV2"},
};

} // namespace fipsy

#endif // FIPSY_VARIANTS_HPP
