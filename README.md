# irac63byh743x
## Introduction
The reason why I create this tutorial project is to get familiar with STM32H743x development and to evaluate the possiblility of using STM32Cube IDE in practice. Due to the hardware limitation, my test code will be mainly focused on communication by USART, CAN and ethernet along with some fundamental features of embedded MCU. 

Most of the code is actually generated by the IDE, which is supposed to be the big advantage of using STM32Cube. My test code or what I called application code is put in main.c only. BZoops folder contains the code from my private library. 

USART3 is used as a console port to hyper-terminal which is regarded as an user interface with a simple CLI support. DHCP and SNMP are implemented for Ethernet testing. The IP protocol stack is provided by LWIP which is also part of IDE standard libraries. CAN communication is tested with an extra sender/reciver board (MCAN-A 3.3v).   

## Setup
The code is run on a hardware board called NUCLEO-H743ZI2 which is an offical evaluation board from ST.   

## Usage
TBC

## Conclusion

!!DON'T COUNT ON THE CODE GENERATED BY STM32CUBE IDE!!!

1. ethernet pin configuration is missing
2. MPU configuration has to follow the rules from ST
3. link script has to be modified MANUALLY
4. heap memory size needs to be adjusted depends on application and it can only be found after tracing the program

it's better to find a technical solution either well tested or commercial to build your own product.

