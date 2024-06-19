#include "fipsy.hpp"
#include <Streaming.h>
#include <WiFi.h>

const char* WIFI_SSID = "my-ssid";
const char* WIFI_PASS = "my-pass";

Fipsy fipsy(SPI);
Fipsy::FuseTable fuseTable;
WiFiServer listener(34779);

void
setup() {
  Serial.begin(115200);
  if (!fipsy.begin(14, 12, 13, 15)) {
    Serial << "Fipsy not found" << endl;
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
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

  Fipsy::JedecError parseError = Fipsy::parseJedec(client, fuseTable);
  if (parseError != Fipsy::JedecError::OK) {
    client << "JEDEC parse error: " << static_cast<int>(parseError) << endl;
    client.stop();
    return;
  }
  client << "JEDEC OK, fuse checksum: " << _HEX(fuseTable.computeChecksum()) << endl;

  if (!fipsy.enable()) {
    client << "Cannot enable configuration mode, status: " << _HEX(fipsy.readStatus().v) << endl;
    client.stop();
    return;
  }

  uint32_t featureRow0, featureRow1;
  uint16_t feabits;
  fipsy.readFeatures(featureRow0, featureRow1, feabits);
  client << "Feature Row: " << _HEX(featureRow0) << ' ' << _HEX(featureRow1) << endl
         << "FEABITS: " << _HEX(feabits) << endl;

  client << "Programming ..." << endl;
  client.flush();
  if (fipsy.program(fuseTable)) {
    client << "Success" << endl;
  } else {
    client << "Failed" << endl;
  }

  fipsy.disable();
  client.flush();
  client.stop();
}
