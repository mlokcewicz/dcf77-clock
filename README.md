# dcf77-clock 

DCF77 controlled radio clock based on Atmega88 microcontroller, CME6005 AM receiver, DS1302 RTC. 

Capabilities:
* Time and date displaying
* Weather information displaying
* Alarm handling 
* Automatic and manual remote time / date / weather info synchronization to DCF77 signal
* Time zone configuration
* Time and date maintaining by CR2032 battery

Hardware:
* ATmega88
* CME6005 DCF77 receiver
* DS1302 RTC + CR2032
* 2x16 HD44760 LCD
* Rotary Encoder + LED + Buzzer

Software:
* C
* Protothreads?
* Own MCU peripherals and external circuits drivers

## Tools
* CMake 3.20.0
* Ninja 1.11.1 
* AVR_8_bit_GNU_Toolchain_3.6.2_1759 5.4.0
* AVRDUDE v7.0
* USBASP drivers libusb_1.2.4.0

## Build

### Generate ninja files
`cmake --preset <hw_version>`

### Build project
`cmake --build --preset <hw_version>`

## External links
* Hardware repository: https://github.com/mlokcewicz/dcf77-clock-pcb
