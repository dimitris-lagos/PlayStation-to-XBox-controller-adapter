#ifndef PS2_H
#define PS2_H

#ifndef uchar
#   define  uchar   unsigned char
#endif
#include "descriptors.h"

extern uchar actuator[2];
extern USB_JoystickReport_Data_t gamepad_state;
extern XIDGamepadOutputReport out_XID_report;

/**
 * @brief Function that sends a command through SPI bus to the PS Controller
 * and reads the reply.
 * 
 * @param data_com uchar value to be transmitted
 * @return uchar value of the reply
 */
uchar spi_mSend(uchar data_com);

/**
 * @brief Function that setups and initializes the SPI interface of the AVR
 * for communication with the PS Controller. For more information read the 
 * implementation in ps2.c
 */
void spi_mInit();

/**
 * @brief Function used to produce a 100us delay.
 */
void wait100us();

/**
 * @brief Function that sends a command to the PS Controller
 * reads the Controller replay and then returns it.
 * 
 * @param data pointer to the uchar array of the elements of the command
 * @param len sizeof or length of the array
 * @return uchar the value of the PS Controller reply
 */
uchar sendCommandToPS2(uchar *data, uchar len);

/**
 * @brief Function that reads the user input on the PS Controller
 * 
 * @return uchar* pointer to the ps2buffer[] that holds the 
 * user input on the PS Controller. ps2buffer[9] is a static uchar
 * array.
 */
uchar* getPS2ControllerInputData();

/**
 * @brief Function that translates/maps the buttons pressed on the PS Controller
 * to the Xbox Controller buttons 
 * 
 * @param ps2buffer pointer to the uchar array that holds the user input on the
 * PS Controller, to be tralsated to Xbox Controller inputs.
 * @return USB_JoystickReport_Data_t type struck containing Xbox Controler input 
 * data to be transited to the USB Host(the XBox)
 */
USB_JoystickReport_Data_t translatePS2toXbox(uchar *ps2buffer);

/**
 * @brief Function that sets the PS Controller to receive rumble data.
 */
void setup_actuator();

#endif