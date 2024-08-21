#include "fipsy.hpp"
#include <Streaming.h>
#include <WiFi.h>

const char* WIFI_SSID = "my-ssid";
const char* WIFI_PASS = "my-pass";

Fipsy fipsy(SPI);
// Choose either Fipsy::FuseTable<343936> for MachX02-1200 or Fipsy::FuseTable<73600> for MachXO2-256. These are the QF values from the .jed file.
// using FipsyClass = Fipsy::FuseTable<73600>;
using FipsyClass = Fipsy::FuseTable<343936>;
FipsyClass fuseTable;

WiFiServer listener(34779);

void
setup() {
  Serial.begin(115200);
  // Old pinout - 14, 12, 13, 15
  // New pinout - sck, miso, mosi, ss
  // 18, 19, 23, 5
  Serial << "Getting Device ID" << endl;
  // 0x012B8043 is for MachXO2-256 and 0x012BA043 is MachXO2-1200HC
  uint32_t deviceID = fipsy.getID(18, 19, 23, 5);
  Serial << "Device ID: " << endl;
  Serial.printf("0x%08lx", deviceID);
  Serial.println();
  

  if (!fipsy.begin(18, 19, 23, 5)) {
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

  FipsyClass::JedecError parseError = fuseTable.parseJedec(client);
  if (parseError !=  FipsyClass::JedecError::OK) {
    client << "JEDEC parse error: " << static_cast<int>(parseError) << endl;
    client << static_cast<int>(parseError) << endl;
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
  if (fuseTable.program(fipsy)) {
    client << "Success" << endl;
  } else {
    client << "Failed" << endl;
  }

  fipsy.disable();
  client.flush();
  client.stop();
}
