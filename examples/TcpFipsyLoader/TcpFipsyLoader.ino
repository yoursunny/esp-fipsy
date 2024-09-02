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
  if (!fpga.begin(14, 12, 13, 15)) {
    Serial.println("Fipsy not found");
    return;
  }

  fuseTable.resize(73600);

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

  auto features = fpga.readFeatures();
  client.print("On-chip features ");
  client.println(features);

  client.println("Programming ...");
  if (fpga.program(fuseTable)) {
    client.println("Success");
  } else {
    client.println("Failed");
  }

  fpga.disable();
  client.stop();
}
