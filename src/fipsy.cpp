#include "fipsy.hpp"

Fipsy::Fipsy(SPIClass& spi)
  : m_spi(spi)
  , m_ss(-1) {}

bool
Fipsy::begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1) {
  m_ss = ss;
  pinMode(ss, OUTPUT);
  m_spi.begin(sck, miso, mosi, ss);

  auto resp = spiTrans<8>({0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
  uint32_t deviceId = (resp[4] << 24) | (resp[5] << 16) | (resp[6] << 8) | (resp[7] << 0);
  // 0x012B8043 is for MachXO2-256 and 0x012BA043 is MachXO2-1200HC
  return deviceId == 0x012B8043 || deviceId == 0x012BA043;
}

uint32_t Fipsy::getID(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1) {
    m_ss = ss;
    pinMode(ss, OUTPUT);
    m_spi.begin(sck, miso, mosi, ss);

    auto resp = spiTrans<8>({0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    uint32_t deviceId = (resp[4] << 24) | (resp[5] << 16) | (resp[6] << 8) | (resp[7] << 0);
    return deviceId; // Return the Device ID directly
}

void
Fipsy::end() {
  m_spi.end();
}

Fipsy::Status
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

