#ifndef FIPSY_HPP
#define FIPSY_HPP

#include "internal/jedec.hpp"
#include "internal/status.hpp"

#include <Arduino.h>
#include <SPI.h>
#include <array>

namespace fipsy {

/** @brief Fipsy FPGA programmer. */
class Fipsy {
public:
  /**
   * @brief Constructor.
   * @param spi the SPI bus.
   */
  explicit Fipsy(SPIClass& spi);

  /**
   * @brief Detect Fipsy.
   * @return Whether expected Device ID is found.
   */
  bool begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);

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
  bool program(const FuseTable& fuseTable);

private:
  template<int N>
  std::array<uint8_t, N> spiTrans(const std::array<uint8_t, N>& req);

  void waitIdle();

private:
  SPIClass& m_spi;
};

template<int N>
std::array<uint8_t, N>
Fipsy::spiTrans(const std::array<uint8_t, N>& req) {
  std::array<uint8_t, N> resp;
#ifndef EPOXY_DUINO
  m_spi.beginTransaction(SPISettings(400000, SPI_MSBFIRST, SPI_MODE0));
  m_spi.transferBytes(req.data(), resp.data(), req.size());
  m_spi.endTransaction();
#endif
  return resp;
}

} // namespace fipsy

#endif // FIPSY_HPP
