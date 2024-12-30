# embedded-project-name 

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
