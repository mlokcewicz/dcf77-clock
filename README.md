# dcf77-clock 

DCF77 controlled radio clock based on ATmega88 microcontroller, MAS6181B AM receiver, DS1307 RTC. 

Capabilities:
* Time and date displaying
* Alarm handling 
* Automatic and manual remote time and date synchronization to DCF77 signal
* Time zone configuration
* Time and date retention by CR2032 battery

Hardware:
* ATmega88
* MAS6181B DCF77 receiver
* DS1307 RTC + CR2032
* 2x16 HD44760 LCD
* Rotary Encoder + LED + Buzzer

Software:
* C
* Protothreads
* Own MCU peripherals and external circuits drivers

## Tools
* CMake 3.20.0
* Ninja 1.11.1 
* AVR_8_bit_GNU_Toolchain_3.7.0.1796 GNU 7.3.0
* AVRDUDE v7.0
* USBASP drivers libusb_1.2.4.0

## Build

### Generate ninja files
`cmake --preset <hw_version>`

### Build project
`cmake --build --preset <hw_version>`

## External links
* Hardware repository: https://github.com/mlokcewicz/dcf77-clock-pcb
