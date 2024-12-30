# TcpFipsyLoader example

In this example, the microcontroller receives a JEDEC file via TCP, and programs Fipsy once the file has been verified.

## Wiring

Fipsy pin   | MCU pin
------------|--------
**1** +3.3V | 3V3
**2** GND   | GND
**3** SCK   | 14
**4** MISO  | 12
**5** MOSI  | 13
**6** SS    | 15

## Usage

1. Modify WiFi setting in this sketch, and upload to microcontroller.
2. Look for the IP address in serial output.
3. Send the JEDEC file with command `nc mcu-ip-address 34779 < filename.jed`.

Sample interaction:

```shell
$ nc 192.168.5.77 34779 < v1-blinky1.jed
Connected device is FipsyV1
JEDEC OK, fuse checksum a0a5, features 0000000000000000/0420
On-chip features 0000000000000000/0420
Programming ...
Success
```
