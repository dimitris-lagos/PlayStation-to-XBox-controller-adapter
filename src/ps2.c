#include <util/delay.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include "ps2.h"
#include "gamePadDefinitions.h"
#include "oddebug.h"






static uchar ps2buffer[9]={0,0,0,0,0,0,0,0,0};

void spi_mInit()
{
	/*When configured as a master, the SPI interface has no automatic control of the SS line. This must be handled by user
	software before communication can start. When this is done, writing a byte to the SPI data register starts the SPI clock
	generator, and the hardware shifts the eight bits into the Slave. After shifting one byte, the SPI clock generator stops, setting
	the end of transmission flag (SPIF). If the SPI interrupt enable bit (SPIE) in the SPCR register is set, an interrupt is
	requested. The master may continue to shift the next byte by writing it into SPDR, or signal the end of packet by pulling high
	the slave select, SS line. The last incoming byte will be kept in the buffer register for later use.
	*/
	// SPI, master, clock/32=500khz
	//When the DORD bit is written to one, the LSB of the data word is transmitted first.
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(0<<SPR0)|(1<<DORD)|(1<<CPHA)|(1<<CPOL);
	SPSR = (1<<SPI2X);
	
	// set CS, MOSI and SCK to output
	DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5); //CS=PB2, MOSI=PB3, SCK=PB5 OUTPUTS
	DDRB &=~(1 << PB4); //MISO=PB4 MISO INPUT
	PORTB |= (1 << PB4); //cs high
}

/* Method that gets information, sends it, waits untill it's sent, then reads the same register (used for both Input and Output) and returns it */
unsigned char spi_mSend(uchar data_com)
{
/* Start transmission */
	SPDR = data_com;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

void wait100us(){
	_delay_us(25);
}


uchar sendCommandToPS2(uchar *data, uchar len){
	uchar ps2buffer[len];
	PORTB &=~(1<<PB2);  //ATT1 Low
	_delay_us(50);
	for(int i=0; i<len;i++){
		ps2buffer[i]=spi_mSend(data[i]);
		wait100us();
	}
	_delay_us(50);
	PORTB |=(1<<PB2); //ATT1 High
	wdt_reset();
	return ps2buffer[1];
}

uchar* getPS2ControllerInputData(){
	wdt_reset();
	PORTB &= ~(1<<PB2);  //ATT1 Low
	
	_delay_us(50);
	
	ps2buffer[0] = spi_mSend(0x01);	// We want data!
	wait100us();		
	ps2buffer[1] = spi_mSend(0x42);	// What's your model? (use later)
	wait100us();
	
	if (ps2buffer[1] == 0x41){		// Digital pad, possibly DDR Pad
	
		ps2buffer[2] = spi_mSend(0x00);
		wait100us();	
		ps2buffer[3] = spi_mSend(0x00); //
		wait100us();	
		ps2buffer[4] = spi_mSend(0x00); //
		wait100us();
		ps2buffer[5] = spi_mSend(0x00);
		wait100us();
		ps2buffer[6] = spi_mSend(0x00);
		wait100us();
		ps2buffer[7] = spi_mSend(0x00);
		wait100us();
		ps2buffer[8] = spi_mSend(0x00);
		//setup_actuator();
	
	} else if (ps2buffer[1]==0x73){// Standard Analog pad in RED mode	
	
		ps2buffer[2] = spi_mSend(0x00);
		wait100us();	
		ps2buffer[3] = spi_mSend(actuator[1]); //
		wait100us();
		ps2buffer[4] = spi_mSend(actuator[0]); //
		wait100us();	
		ps2buffer[5] = spi_mSend(0x00);
		wait100us();
		ps2buffer[6] = spi_mSend(0x00);
		wait100us();			
		ps2buffer[7] = spi_mSend(0x00);
		wait100us();			
		ps2buffer[8] = spi_mSend(0x00);

	}
	else{
		while(1);//no pad detected, wait till the wdt resets the AVR
	}
	DBG2(0xA1, &actuator[0], 1);
	DBG2(0xA2, &actuator[1], 1);
	_delay_us(50);
	PORTB |= (1<<PB2); //ATT1 High
	wdt_reset();
	return ps2buffer;
}

USB_JoystickReport_Data_t translatePS2toXbox(uchar *ps2buffer) {

	int temp1=255-ps2buffer[3];
	int temp2=255-ps2buffer[4];
	
	USB_JoystickReport_Data_t gamepad_state;
	gamepad_state.rsize = 0x14;
	if (ps2buffer[1] == 0x73){		// Decode analog pad
		
		// Right stick, X axis
		gamepad_state.r_x = ps2buffer[5]-0x80;
		// Right stick, Y axis
		gamepad_state.r_y = ~(ps2buffer[6]-0x80);
		// Left stick,  X axis
		gamepad_state.l_x = ps2buffer[7]-0x80;
		// Left stick,  Y axis
		gamepad_state.l_y = ~(ps2buffer[8]-0x80);


		gamepad_state.digital_buttons=
			((temp1&PS2_DPAD_UP) >> 4)    |
			((temp1&PS2_DPAD_DOWN) >> 5)  |
			((temp1&PS2_DPAD_LEFT) >> 5)  |
			((temp1&PS2_DPAD_RIGHT) >> 2) |
			((temp1&PS2_START) << 1)      |
			((temp1&PS2_SELECT) << 5)     |
			((temp1&PS2_L3) << 5)     	|
			((temp1&PS2_R3) << 5)     	;
			
		gamepad_state.a = 
			(temp2&PS2_X)*0xff; //xbox_A
		gamepad_state.b = 
			(temp2&PS2_O)*0xff; //xbox_B
		gamepad_state.x = 
			(temp2&PS2_S)*0xff; //xbox_X
		gamepad_state.y = 
			(temp2&PS2_T)*0xff; //xbox_Y
		gamepad_state.black = 
			(temp2&PS2_L2)*0xff; //xbox_Black
		gamepad_state.white = 
			(temp2&PS2_R2)*0xff; //xbox_White	
		gamepad_state.l = 
			(temp2&PS2_L1)*0xff; //xbox_Left_Trigger
		gamepad_state.r = 
			(temp2&PS2_R1)*0xff; //xbox_Left_Trigger
		
		
	}
	else if(ps2buffer[1] == 0x41){		// Digital pad
		
		gamepad_state.digital_buttons=
			((temp1&PS2_DPAD_UP)>>4)    |
			((temp1&PS2_DPAD_DOWN)>>5)  |
			((temp1&PS2_DPAD_LEFT)>>5)  |
			((temp1&PS2_DPAD_RIGHT)>>2) |
			((temp1&PS2_START)<<1)      |
			((temp1&PS2_SELECT)<<5)     |
			((temp1&PS2_L3)<<5)     	|
			((temp1&PS2_R3)<<5)     	;
			
		gamepad_state.a=
			(temp2&PS2_X)*0xff; //xbox_A
		gamepad_state.b=
			(temp2&PS2_O)*0xff; //xbox_B
		gamepad_state.x=
			(temp2&PS2_S)*0xff; //xbox_X
		gamepad_state.y=
			(temp2&PS2_T)*0xff; //xbox_Y
		gamepad_state.black=
			(temp2&PS2_L2)*0xff; //xbox_Black
		gamepad_state.white=
			(temp2&PS2_R2)*0xff; //xbox_White	
		gamepad_state.l=
			(temp2&PS2_L1)*0xff; //xbox_Left_Trigger
		gamepad_state.r=
			(temp2&PS2_R1)*0xff; //xbox_Left_Trigger

	 }
	return gamepad_state;
}

/*
* Enable the actuators on the ps2 controller 
*/
void setup_actuator(){
	uchar mode=0x00;
	do{
		uchar conf1[9]={0x01,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//enter config mode
		mode = sendCommandToPS2(conf1,5);
	}while(mode!=0x41 && mode!=0x73);

	uchar conf1[9]={0x01,0x43,0x00,0x01,0x00,0x00,0x00,0x00,0x00};//enter config mode
	sendCommandToPS2(conf1,5);
	uchar conf2[9]={0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00}; //turn on analog mode
	sendCommandToPS2(conf2,9);
	uchar conf3[9]={0x01,0x4D,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xFF}; //setup motor command mapping
	sendCommandToPS2(conf3,9);
	uchar conf4[9]={0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A}; //exit config
	sendCommandToPS2(conf4,9);
}