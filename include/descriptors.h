#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H
#include <stdint-gcc.h> /* required for uint8_t usage */
#include <avr/pgmspace.h>   /* required for PROGMEM usage */

/**
 * @brief Struct definition for the HID_GET_REPORT request.
 */
typedef struct USB_JoystickReport_Data_t{
	uint8_t rid;//[0]
	uint8_t rsize; //[1]
	uint8_t digital_buttons;//[2]
	uint8_t reserved_1;//[3]
	uint8_t a;//[4]
	uint8_t b;//[5]
	uint8_t x;//[6]
	uint8_t y;//[7]
	uint8_t black;//[8]
	uint8_t white;//[9]
	uint8_t l;//[10]
	uint8_t r;//[11]
	uint8_t  l_x0;//[12] useless
	uint8_t  l_x;//[13]
	uint8_t  l_y0;//[14] useless
	uint8_t  l_y;//[15]
	uint8_t  r_x0;//[16] useless
	uint8_t  r_x;//[17]
	uint8_t  r_y0;//[18] useless
	uint8_t  r_y;//[19]
} USB_JoystickReport_Data_t;

/**
 * @brief Struct definition for the XID Get Report request
 */
typedef struct XIDGamepadOutputReport{
    uint8_t  rid; 
    uint8_t  rsize; // Has to be 0x06
	uint8_t left_actuator_strength0; //useless
    uint8_t left_actuator_strength;
	uint8_t right_actuator_strength0; //useless
    uint8_t right_actuator_strength;
} XIDGamepadOutputReport; 

#endif