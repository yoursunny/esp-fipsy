# Fipsy FPGA Programmer for ESP32

**esp-fipsy** library allows programming a [Fipsy FPGA breakout board](https://www.fipsyfpga.com/) from an ESP32 microcontroller.

This library can be used in the Arduino IDE.

When using this project directly:
1. Visit the documentation for the ESP32 programmer on [https://www.fipsyfpga.com](https://www.fipsyfpga.com)
2. Program the ESP32 as a programmer using the [TcpFipsyLoader](examples/TcpFipsyLoader/). This file demonstrates how to use this library.
3. Once you have the IP address for the ESP32 use the [sendJed.py](scripts/sendJed.py) file to program the device. For example:

```
python sendJed.py --file "C:\Users\myuser\Projects\Fipsy-FPGA-edu\Examples\FipsyV2 - XO2-1200\1. Blinky\project_files\Implementation\FipsyBaseline_Implementation.jed" 192.168.1.194
```

For more information please see: [FipsyFPGA.com](https://www.fipsyfpga.com)




## Acknowledgements

We would like to thank Junxiao (Sunny) Shi - [yoursunny.com](https://yoursunny.com/) for significant contributions to the ESP32 programmer!