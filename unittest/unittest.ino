#include <fipsy.hpp>

#include <AUnit.h>
#include <Arduino.h>
#include <EpoxyFS.h>

test(ParseV1) {
  // For FispyV1 - MachXO2-256
  fipsy::FuseTable fuseTable;
  fuseTable.resize(73600);

  auto file = fs::EpoxyFS.open("v1-blinky1.jed", "r");
  auto jc = fipsy::parseJedec(file, fuseTable);
  file.close();

  assertEqual(static_cast<int>(jc.error), 0);
  assertTrue(static_cast<bool>(jc));
  assertEqual(jc.features.featureRow(), static_cast<uint64_t>(0));
  assertEqual(jc.features.feabits(), 0x0420);
  assertEqual(fuseTable.computeChecksum(), 0xA0A5);
}

test(ParseV2) {
  // For FispyV2 - MachXO2-1200
  fipsy::FuseTable fuseTable;
  fuseTable.resize(343936);

  auto file = fs::EpoxyFS.open("v2-blinky1.jed", "r");
  auto jc = fipsy::parseJedec(file, fuseTable);
  file.close();

  assertEqual(static_cast<int>(jc.error), 0);
  assertTrue(static_cast<bool>(jc));
  assertEqual(jc.features.featureRow(), static_cast<uint64_t>(0));
  // assertEqual(jc.features.feabits(), 0x0420);
  assertEqual(fuseTable.computeChecksum(), 0x99AE);
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
