#include "fipsy.hpp"
#include <algorithm>

namespace fipsy {

Fipsy::Fipsy(int8_t cs, ::SPIClass* theSPI)
  : m_spi(cs, 100000, SPI_BITORDER_MSBFIRST, SPI_MODE0, theSPI) {}

const Variant*
Fipsy::begin() {
  if (!m_spi.begin()) {
    return nullptr;
  }

  auto rsp = spiTrans<8>({0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
  uint32_t deviceId = (rsp[4] << 24) | (rsp[5] << 16) | (rsp[6] << 8) | (rsp[7] << 0);

  for (size_t i = 0; i < sizeof(VARIANTS) / sizeof(VARIANTS[0]); ++i) {
    const Variant* variant = &VARIANTS[i];
    if (deviceId == variant->deviceId) {
      return variant;
    }
  }
  return nullptr;
}

void
Fipsy::end() {}

void
Fipsy::spiTrans(const uint8_t* req, uint8_t* rsp, uint32_t len) {
  std::copy_n(req, len, rsp);
  m_spi.write_and_read(rsp, len);
}

Status
Fipsy::readStatus() {
  auto rsp = spiTrans<8>({0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
  return Status{(static_cast<uint32_t>(rsp[4]) << 24) | (rsp[5] << 16) | (rsp[6] << 8) |
                (rsp[7] << 0)};
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
  erase(0x01);

  auto status = readStatus();
  return !status.fail() && status.enabled();
}

void
Fipsy::disable() {
  // refresh
  spiTrans<3>({0x79, 0x00, 0x00});
  delay(10);
}

Features
Fipsy::readFeatures() {
  Features result;

  // read Feature Row
  auto fr = spiTrans<12>({0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
  memcpy(&result[0], &fr[4], 8);

  // read FEABITS
  auto fb = spiTrans<12>({0xFB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
  memcpy(&result[8], &fb[4], 2);

  return result;
}

void
Fipsy::erase(uint8_t y) {
  spiTrans<4>({0x0E, y, 0x00, 0x00});
  waitIdle();
}

void
Fipsy::cleanup() {
  // erase flash
  erase(0x0E);
}

ProgramResult
Fipsy::program(const FuseTable& fuseTable, const Features& features) {
  // check features
  if (!features.hasSlaveSPI()) {
    return ProgramResult{__LINE__};
  }

  // erase flash
  erase(0x0E);
  if (readStatus().fail()) {
    return ProgramResult{__LINE__};
  }

  // program configuration flash
  programPages(0x46, fuseTable);

  // program UFM: not implemented

  // program & verify usercode: not implemented

  // write and verify Feature Row
  if (!programFeatures(features)) {
    cleanup();
    return ProgramResult{__LINE__};
  }

  // program DONE bit
  spiTrans<4>({0x5E, 0x00, 0x00, 0x00});
  waitIdle();
  if (readStatus().fail()) {
    return ProgramResult{__LINE__};
  }
  return ProgramResult{0};
}

static inline void
fillPage(uint8_t* output, const std::vector<bool>& input, size_t i) {
  for (int j = 0; j < 16; ++j) {
    for (int k = 0; k < 8; ++k) {
      output[j] |= input[i + j * 8 + k] << (7 - k);
    }
  }
}

void
Fipsy::programPages(uint8_t command, const std::vector<bool>& input) {
  // set address to zero
  spiTrans<4>({command, 0x00, 0x00, 0x00});
  waitIdle();

  // program pages
  for (size_t i = 0; i < input.size(); i += 128) {
    std::array<uint8_t, 20> req;
    req.fill(0x00);
    req[0] = 0x70;
    req[3] = 0x01;
    fillPage(&req[4], input, i);
    spiTrans<20>(req);
    waitIdle();
  }
}

bool
Fipsy::programFeatures(const Features& features) {
  if (!features.hasSlaveSPI()) {
    return false;
  }

  // write Feature Row
  std::array<uint8_t, 12> fr;
  fr.fill(0x00);
  fr[0] = 0xE4;
  memcpy(&fr[4], &features[0], 8);
  spiTrans<12>(fr);
  waitIdle();

  // write FEABITS
  std::array<uint8_t, 6> fb;
  fb.fill(0x00);
  fb[0] = 0xF8;
  memcpy(&fb[4], &features[8], 2);
  spiTrans<6>(fb);
  waitIdle();

  // read Feature Row and FEABITS
  auto read = readFeatures();
  return read == features && read.hasSlaveSPI();
}

} // namespace fipsy
