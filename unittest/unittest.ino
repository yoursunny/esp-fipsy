#include <fipsy.hpp>

#include <AUnit.h>
#include <Arduino.h>
#include <EpoxyFS.h>

test(ParseV1) {
  fipsy::FuseTable fuseTable;
  fuseTable.resize(73600);

  auto file1 = fs::EpoxyFS.open("v1-blinky1.jed", "r");
  auto jc1 = fipsy::parseJedec(file1, fuseTable);
  file1.close();

  assertEqual(static_cast<int>(jc1.error), 0);
  assertTrue(static_cast<bool>(jc1));
  assertEqual(jc1.features.featureRow(), static_cast<uint64_t>(0));
  assertEqual(jc1.features.feabits(), 0x0420);
  assertEqual(fuseTable.computeChecksum(), 0xA0A5);

  auto file2 = fs::EpoxyFS.open("v1-blinky2.jed", "r");
  auto jc2 = fipsy::parseJedec(file2, fuseTable);
  file2.close();

  assertEqual(static_cast<int>(jc2.error), 0);
  assertTrue(static_cast<bool>(jc2));
  assertEqual(jc2.features.featureRow(), static_cast<uint64_t>(0));
  assertEqual(jc2.features.feabits(), 0x0420);
  assertEqual(fuseTable.computeChecksum(), 0xA18A);
}

test(ParseV2) {
  fipsy::FuseTable fuseTable;
  fuseTable.resize(343936);

  auto file1 = fs::EpoxyFS.open("v2-blinky1.jed", "r");
  auto jc1 = fipsy::parseJedec(file1, fuseTable);
  file1.close();

  assertEqual(static_cast<int>(jc1.error), 0);
  assertTrue(static_cast<bool>(jc1));
  assertEqual(jc1.features.featureRow(), static_cast<uint64_t>(0));
  assertEqual(jc1.features.feabits(), 0x0420);
  assertEqual(fuseTable.computeChecksum(), 0x99AE);

  auto file2 = fs::EpoxyFS.open("v2-blinky2.jed", "r");
  auto jc2 = fipsy::parseJedec(file2, fuseTable);
  file2.close();

  assertEqual(static_cast<int>(jc2.error), 0);
  assertTrue(static_cast<bool>(jc2));
  assertEqual(jc2.features.featureRow(), static_cast<uint64_t>(0));
  assertEqual(jc2.features.feabits(), 0x0420);
  assertEqual(fuseTable.computeChecksum(), 0x9F31);
}

test(ParseWrongQF) {
  fipsy::FuseTable fuseTable;
  fuseTable.resize(73600);

  auto file1 = fs::EpoxyFS.open("v2-blinky1.jed", "r");
  auto jc1 = fipsy::parseJedec(file1, fuseTable);
  file1.close();

  assertNotEqual(static_cast<int>(jc1.error), 0);
  assertFalse(static_cast<bool>(jc1));
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
