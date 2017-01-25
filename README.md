# ARRL_AlarmClockKit

## Introduction
This is the source code for the micro-controller (MCU) used in NM0S's alarm clock kit produced for the ARRL.  

The target MCU is the Microchip [PIC16F18854](http://www.microchip.com/wwwproducts/en/PIC16F18854).

This source code is writtin in ANSI C for Microchip's XC8 compiler suite.  
Development was conducted using MPLAB X IDE v3.35 and XC8 v1.38 on an 64-bit PC running Ubuntu Linux.  An ICD3 was utilized for programming the MCU's flash memory.

## Usage Instructions
See [USAGE.md](USAGE.md)

## License
The source code is published under the [MIT](https://opensource.org/licenses/MIT) open source license so that curious minds may easily obtain, inspect, and modify the firmware with few restrictions.

## Flashing the Firmware
The clock kit is primarily marketed as a learn-to-solder kit. To meet design and cost requirements on-board programming hardware is not included.

Individuals wishing to experiment with the firmware will require external programming hardware tools. Microchip's ICD3 was used during development. The PICkit3 and other lower-cost programmers will probably work but have not been tested by the author. YMMV.
