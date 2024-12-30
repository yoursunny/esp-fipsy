#include "fipsy.hpp"
#include <WiFi.h>

// Modify WiFi settings, required.
const char* WIFI_SSID = "my-ssid";
const char* WIFI_PASS = "my-pass";

// MCU will wait for serial port to become ready before starting.
// Changing to false allows the MCU to start without attaching to USB.
const bool WAIT_SERIAL = true;

// Change SPI pins, if necessary.
const int8_t PIN_SCK = 14;
const int8_t PIN_MISO = 12;
const int8_t PIN_MOSI = 13;
const int8_t PIN_SS = 15;

fipsy::Fipsy fpga(PIN_SS);
const fipsy::Variant* variant = nullptr;
fipsy::FuseTable fuseTable;
WiFiServer listener(34779);

void
setup() {
  Serial.begin(115200);
  while (WAIT_SERIAL && !Serial) {
    delay(1);
  }
  delay(100);
  Serial.println();

#if defined(ARDUINO_ARCH_ESP32)
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1);
#elif defined(ARDUINO_ARCH_RP2040)
  SPI.setSCK(PIN_SCK);
  SPI.setRX(PIN_MISO);
  SPI.setTX(PIN_MOSI);
  SPI.begin();
#endif
  if (!(variant = fpga.begin())) {
    Serial.println("Fipsy not found");
    return;
  }
  Serial.print("Found device: ");
  Serial.println(variant->desc);

  fuseTable.resize(variant->qf);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi failure %d", static_cast<int>(WiFi.status()));
    Serial.println();
    return;
  }
  Serial.println(WiFi.localIP());

  listener.begin();
}

void
loop() {
  auto client = listener.accept();
  if (!client) {
    return;
  }

  client.print("Connected device is ");
  client.println(variant->desc);

  auto jc = fipsy::parseJedec(client, fuseTable);
  if (!jc) {
    client.printf("JEDEC parse error %d", static_cast<int>(jc.error));
    client.println();
    delay(100);
    client.stop();
    return;
  }
  client.printf("JEDEC OK, fuse checksum %04" PRIx16 ", features ", fuseTable.computeChecksum());
  client.println(jc.features);

  if (!fpga.enable()) {
    client.printf("Cannot enable configuration mode, status %08" PRIx32, fpga.readStatus().v);
    client.println();
    delay(100);
    client.stop();
    return;
  }

  client.print("On-chip features ");
  client.println(fpga.readFeatures());

  client.println("Programming ...");
  auto result = fpga.program(fuseTable, jc.features);
  if (result) {
    client.println("Success");
  } else {
    client.print("Failed ");
    client.println(result.error);
  }

  fpga.disable();
  delay(100);
  client.stop();
}
