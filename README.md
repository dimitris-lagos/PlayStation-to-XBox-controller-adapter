# PlayStation-to-XBox-controller-adapter
Use this firmware to connect a PSX/PS2 controller to the Original Xbox using an AVR or Arduino


PlayStation to XBox controller adapter.
Use this firmware to connect a PSX/PS2 controller to the Original Xbox. Analog sticks and rumble actuators work.

MAKE FILE is set for ATMEGA328P @16Mhz and USBTiny programmer. AVR-GCC has to be set in the system envirnment variable. Command "make flash" compiles and flashes the hex on the avr. Also sets the fuses.

In reference to the electrical connections, contrary to most of the VUSB circuits that have the mcu at 5V, that show 3V6 diodes to clamp the D- and D+ voltages, i found that those diodes are not needed. Worse, the ones i got (NXP BZV85 3V6), due to the high diode capacitance they have, made the USB not able to work at all. So if you decide to use clamping diodes, find small ones(low capacitance).
![Alt text](Pictures/Connections.png?raw=true "Title")


This project uses the [VUSB library](https://github.com/obdev/v-usb) that makes the usb-incapable avr microcontrollers(atmega328p/168p etc.) able to "talk" USB 1.1. The source of the VUSB library has been modified("usbdrv/usbdrv.h", "usbdrv/usbrv.c", "usbdrv/asmcommon.inc" and "usbconfig.h") in order to send Interrupt-in/Bulk-in packets of size greater than 8 bytes which is the library limit. Specifically the XBox XID Report needs to be 20 bytes long.
Any AVR with flash greater than 4kB should be compatible with VUSB and this project. I've tested it with ATMEGA328AU and ATMEGA168-20MU , both at 16Mhz  and 5V VCC. Those chips come on the pcb of Arduino Pro Mini China Clones that are sold on ebay. One should be able to compile the program for 12Mhz-20Mhz clocks, as long as the Make File is configured acordingly and the SPI clock devider in main.c is set to produce [500kHz SPI](Pictures/SPI.PNG). This is the max SPI frequency the PSX/PS2 controller can handle.


Although the VUSB library is great, it has a lot of limitations. Due to the fact that there is no callback mechanism for when a USB Setup transaction has been finished, there is no way to know when has the Controller Setup finished, in order to start sending Interrupt-in/Bulk-in Button Data. So after the "last" Setup Request the host sends, a flag(pad) is set to one. Because we can't know when the device has finished responding to this request, a delay counter is used inside the main loop to give enough time to the device to respond.  The XBOX Dash(and most other dashes) end the SETUP with a Vendor Specific HID GET REPORT request(C1 01 00 01 00 00 14 00). On the other hand, most games I tested end the Controller Setup with a Class Type HID GET REPORT request( A1 01 00 01 00 00 14 00). The problem lies on the fact that the latter request is common to both the Dash Setup sequence and the Game Setup sequence. So we can't know when the controller has been setup. To overcome this, a 10ms timer interrupt is fired when the device has made it to the latter request. 10ms should be enough time for any other transaction to be completed.

The Original USB 2.0 Xbox controller samples the buttons 4 times for every frame(4ms period). Due to the fact that the VUSB uses the USB 1.1 protocol, the Xbox system only samples the buttons 2 times for every frame([8ms period](Pictures/controller_delay.PNG)). When the Xbox system sends actuator data to our controller, the mcu is only able to sample the buttons 1 time per frame([16ms period](Pictures/controller_delay_with_rumbling.PNG)). This firmware works only for Xbox systems and not Windows PCs. The USB 1.1 protocol standard dictates 8 byte maximum HID report packets, while the Xbox report packet is actually 20 bytes long. To make the firmware work on Windows PCs, the wMaxPacketSize in the usbDescriptorConfiguration has to be edited to 0x08 for both the HID_ReportINEndpoint and the HID_ReportOUTEndpoint.

Interesting Repositories:
- [Dual-Strike](https://github.com/IvIePhisto/Dual-Strike) The VUSB source modifications for the Interrupt-In code are from this repo.
- [xpad-avr](https://github.com/ripdajacker/xpad-avr) Similar project for usb-capable avrs.
- [XBOXPadMicro](https://github.com/bootsector/XBOXPadMicro) Similar project for usb-capable avrs using LUFA library.
- [ps2usb](http://vusb.wikidot.com/project:ps2usb) A VUSB firware to convert a psx/ps2 controller to usb. Some SPI and PS2 functions are taken from this repo.

