#include "fipsy.hpp"
#include <WiFi.h>

const char* WIFI_SSID = "my-ssid";
const char* WIFI_PASS = "my-pass";

fipsy::Fipsy fpga(SPI);
fipsy::FuseTable fuseTable;
WiFiServer listener(34779);

void
setup() {
  Serial.begin(115200);

  // Choose either fuseTable.resize(343936) for MachX02-1200 or fuseTable.resize(73600) for MachXO2-256.
  // These are the QF values from the .jed file.
  // fuseTable.resize(73600);
  fuseTable.resize(343936);

  // Known alternate pinouts for SPI include:
  // 18, 19, 23, 5
  // 14, 12, 13, 15
  // These pins are sck, miso, mosi, ss
  if (!fpga.begin(18, 19, 23, 5)) {
    Serial.println("Fipsy not found");
    return;
  } else {
    uint32_t deviceID = fpga.getID();
    Serial.print("Device ID: ");
    Serial.printf("0x%08lx", deviceID);
    Serial.println();
  }

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

  auto jc = fipsy::parseJedec(client, fuseTable);
  if (!jc) {
    client.printf("JEDEC parse error %d", static_cast<int>(jc.error));
    client.println();
    client.stop();
    return;
  }
  client.printf("JEDEC OK, fuse checksum %04" PRIx16 ", features ", fuseTable.computeChecksum());
  client.println(jc.features);

  if (!fpga.enable()) {
    client.printf("Cannot enable configuration mode, status %08" PRIx32, fpga.readStatus().v);
    client.println();
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
  client.stop();
}
