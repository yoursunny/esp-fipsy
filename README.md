# Fipsy FPGA Programmer for ESP32 and More

[![GitHub Workflow status](https://img.shields.io/github/actions/workflow/status/yoursunny/esp-fipsy/build.yml?style=flat)](https://github.com/yoursunny/esp-fipsy/actions) [![GitHub code size](https://img.shields.io/github/languages/code-size/yoursunny/esp-fipsy?style=flat)](https://github.com/yoursunny/esp-fipsy)

**esp-fipsy** library allows programming Fipsy FPGA breakout boards from ESP32 series microcontrollers.
[TcpFipsyLoader](examples/TcpFipsyLoader/) demonstrates how to use this library.

Supported FPGA boards:

* [FipsyV1](https://www.mocomakers.com/fipsy-fpga/), MachXO2-256
* [FipsyV2](https://www.fipsyfpga.com/), MachXO2-1200

Supported microcontrollers:

* ESP32
* ESP32-C3
* RP2040

This library depends on [Adafruit Bus IO Library](https://github.com/adafruit/Adafruit_BusIO).
It must be installed before using this library.
