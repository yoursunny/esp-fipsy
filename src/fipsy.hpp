#ifndef FIPSY_HPP
#define FIPSY_HPP

#include "internal/jedec.hpp"
#include "internal/status.hpp"
#include "internal/variants.hpp"

#include <Adafruit_SPIDevice.h>
#include <Arduino.h>
#include <array>

namespace fipsy {

class ProgramResult {
public:
  explicit ProgramResult(int error)
    : error(error) {}

  explicit operator bool() const {
    return error == 0;
  }

public:
  int error = 0;
};

/** @brief Fipsy FPGA programmer. */
class Fipsy {
public:
  /**
   * @brief Constructor.
   * @param cs SPI Chip Select pin.
   * @param spi SPI bus.
   */
  explicit Fipsy(int8_t cs, ::SPIClass* theSPI = &SPI);

  /**
   * @brief Detect Fipsy.
   * @return Detected variant, or nullptr if not found.
   */
  const Variant* begin();

  /**
   * @brief Release SPI bus.
   * @pre begin()
   */
  void end();

  /**
   * @brief Read status register.
   * @pre begin()
   */
  Status readStatus();

  /**
   * @brief Enable offline configuration mode.
   * @pre begin()
   */
  bool enable();

  /**
   * @brief Disable configuration mode.
   * @pre enable()
   */
  void disable();

  /**
   * @brief Read Feature Row and FEABITS.
   * @pre enable()
   */
  Features readFeatures();

  /**
   * @brief Program fuse table.
   * @pre enable()
   */
  ProgramResult program(const FuseTable& fuseTable, const Features& features);

private:
  template<uint32_t N>
  std::array<uint8_t, N> spiTrans(const std::array<uint8_t, N>& req);

  void spiTrans(const uint8_t* req, uint8_t* rsp, uint32_t len);

  void waitIdle();

  void erase(uint8_t y);

  void cleanup();

  void programPages(uint8_t command, const std::vector<bool>& input);

  bool programFeatures(const Features& features);

private:
  Adafruit_SPIDevice m_spi;
};

template<uint32_t N>
std::array<uint8_t, N>
Fipsy::spiTrans(const std::array<uint8_t, N>& req) {
  std::array<uint8_t, N> rsp;
  spiTrans(req.data(), rsp.data(), N);
  return rsp;
}

} // namespace fipsy

#endif // FIPSY_HPP
