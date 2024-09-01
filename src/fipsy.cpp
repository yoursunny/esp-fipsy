#include "fipsy.hpp"

namespace fipsy {

Fipsy::Fipsy(SPIClass& spi)
  : m_spi(spi) {}

bool
Fipsy::begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1) {
#ifndef EPOXY_DUINO
  m_spi.begin(sck, miso, mosi, ss);
  m_spi.setHwCs(true);
#endif

  auto resp = spiTrans<8>({0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
  uint32_t deviceId = (resp[4] << 24) | (resp[5] << 16) | (resp[6] << 8) | (resp[7] << 0);
  return deviceId == 0x012B8043;
}

void
Fipsy::end() {
  m_spi.end();
}

Status
Fipsy::readStatus() {
  auto resp = spiTrans<8>({0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
  return Status{(static_cast<uint32_t>(resp[4]) << 24) | (resp[5] << 16) | (resp[6] << 8) |
                (resp[7] << 0)};
}

void
Fipsy::waitIdle() {
  while (readStatus().busy()) {
    delay(1);
  }
}

bool
Fipsy::enable() {
  // enable offline configuration
  spiTrans<4>({0xC6, 0x08, 0x00, 0x00});
  waitIdle();

  // erase SRAM
  spiTrans<4>({0x0E, 0x01, 0x00, 0x00});
  waitIdle();

  auto status = readStatus();
  return !status.fail() && status.enabled();
}

void
Fipsy::disable() {
  // disable configuration
  spiTrans<3>({0x26, 0x00, 0x00});
  spiTrans<4>({0xFF, 0xFF, 0xFF, 0xFF});

  // refresh
  spiTrans<3>({0x79, 0x00, 0x00});
  delay(10);
}

void
Fipsy::readFeatures(uint32_t& featureRow0, uint32_t& featureRow1, uint16_t& feabits) {
  // read Feature Row
  auto resp =
    spiTrans<12>({0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
  featureRow0 = (resp[4] << 24) | (resp[5] << 16) | (resp[6] << 8) | (resp[7] << 0);
  featureRow1 = (resp[8] << 24) | (resp[9] << 16) | (resp[10] << 8) | (resp[11] << 0);

  // read FEABITS
  auto resp2 = spiTrans<12>({0xFB, 0x00, 0x00, 0x00, 0x00, 0x00});
  feabits = (resp2[4] << 8) | (resp2[5] << 0);
}

bool
Fipsy::program(const FuseTable& fuseTable) {
  // erase flash
  spiTrans<4>({0x0E, 0x04, 0x00, 0x00});
  waitIdle();
  if (readStatus().fail()) {
    return false;
  }

  // set address to zero
  spiTrans<4>({0x46, 0x00, 0x00, 0x00});
  waitIdle();

  // program pages
  for (int i = 0; i < fuseTable.size(); i += 128) {
    std::array<uint8_t, 20> req;
    req.fill(0x00);
    req[0] = 0x70;
    req[3] = 0x01;
    for (int j = 0; j < 16; ++j) {
      for (int k = 0; k < 8; ++k) {
        req[4 + j] |= fuseTable[i + j * 8 + k] << (7 - k);
      }
    }
    spiTrans<20>(req);
    waitIdle();
  }

  // program DONE bit
  spiTrans<4>({0x5E, 0x00, 0x00, 0x00});
  waitIdle();
  return !readStatus().fail();
}

} // namespace fipsy
