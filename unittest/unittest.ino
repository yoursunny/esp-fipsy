#include <fipsy.hpp>

#include <AUnit.h>
#include <Arduino.h>
#include <EpoxyFS.h>

test(ParseV1) {
  auto file = fs::EpoxyFS.open("v1-blinky1.jed", "r");
  fipsy::FuseTable fuseTable;
  fuseTable.resize(73600);
  auto parseError = fipsy::parseJedec(file, fuseTable);
  assertEqual(static_cast<int>(parseError), static_cast<int>(fipsy::JedecError::OK));
  assertEqual(fuseTable.computeChecksum(), 0xA0A5);
  file.close();
}

void
setup() {
  Serial.begin(115200);
  Serial.println();

  auto ok = fs::EpoxyFS.begin();
  if (!ok) {
    Serial.println("EpoxyFS error");
    exit(1);
  }
}

void
loop() {
  aunit::TestRunner::run();
}
