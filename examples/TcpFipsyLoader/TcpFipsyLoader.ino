#include "fipsy.hpp"
#include <Streaming.h>
#include <WiFi.h>

const char* WIFI_SSID = "my-ssid";
const char* WIFI_PASS = "my-pass";

fipsy::Fipsy fpga(SPI);
fipsy::FuseTable fuseTable;
WiFiServer listener(34779);

void
setup() {
  Serial.begin(115200);
  if (!fpga.begin(14, 12, 13, 15)) {
    Serial << "Fipsy not found" << endl;
    return;
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial << "WiFi failure " << WiFi.status() << endl;
    return;
  }
  Serial << WiFi.localIP() << endl;

  listener.begin();
}

void
loop() {
  auto client = listener.accept();
  if (!client) {
    return;
  }

  auto parseError = fipsy::parseJedec(client, fuseTable);
  if (parseError != fipsy::JedecError::OK) {
    client << "JEDEC parse error: " << static_cast<int>(parseError) << endl;
    client.stop();
    return;
  }
  client << "JEDEC OK, fuse checksum: " << _HEX(fuseTable.computeChecksum()) << endl;

  if (!fpga.enable()) {
    client << "Cannot enable configuration mode, status: " << _HEX(fpga.readStatus().v) << endl;
    client.stop();
    return;
  }

  uint32_t featureRow0, featureRow1;
  uint16_t feabits;
  fpga.readFeatures(featureRow0, featureRow1, feabits);
  client << "Feature Row: " << _HEX(featureRow0) << ' ' << _HEX(featureRow1) << endl
         << "FEABITS: " << _HEX(feabits) << endl;

  client << "Programming ..." << endl;
  client.flush();
  if (fpga.program(fuseTable)) {
    client << "Success" << endl;
  } else {
    client << "Failed" << endl;
  }

  fpga.disable();
  client.flush();
  client.stop();
}
