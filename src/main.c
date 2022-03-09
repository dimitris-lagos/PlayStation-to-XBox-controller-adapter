/*
 * VUSB_XBOX.c
 *
 * Created: 15/07/2021 10:58:54 πμ
 * Author : Jiml
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include <string.h>
#include <stdbool.h>
#include <stdint-gcc.h>

#include "usbdrv.h"
#include "oddebug.h"
#include "descriptors.h"
#include "ps2.h"

/*
* -----Usefull Notes--------------------------------------------------------------------------
* Communicating through Control endpoint 0
* This is the most basic functionality of USB. Messages sent to control endpoints consist
* of 8 bytes structured setup data (each byte has a particular meaning) and a block of 
* arbitrary length which is either received from the device (control-in) or sent to the 
* device (control-out). Since the length can be zero, the data block is optional.
* 
* The 8 bytes setup data are described by the C type usbRequest_t which is declared in usbdrv.h:
* 
* ******usbWord_t is 2 bytes long
* 
* typedef struct usbRequest{
*     uchar       bmRequestType;
*     uchar       bRequest;
*     usbWord_t   wValue;
*     usbWord_t   wIndex;
*     usbWord_t   wLength;
* }usbRequest_t;
* 
*  bmRequestType field in USB setup:
*  * d t t r r r r r, where
*  * d ..... direction: 0=host->device, 1=device->host
*  * t ..... type: 0=standard, 1=class, 2=vendor, 3=reserved
*  * r ..... recipient: 0=device, 1=interface, 2=endpoint, 3=other
* 
* -bmRequestType is a bitmask which contains the direction of the following data block
* (control-in or control-out) and a context for the bRequest. The context describes the 
* recipient of the message (device, interface or endpoint and whether it's targeted at 
* the driver, the device class or your code). For arbitrary messages in a custom class
* device, the recipient should be your code (which means "vendor" in USB slang). You 
* don't need to distinguish between device, class and endpoint since it's up to you 
* how vendor type messages are interpreted.
* 
* -bRequest identifies the request. If your device supports multiple functions (e.g. 
* turn LED on, turn LED off, query input status), each function can be assigned a 
* request number. Which numbers you use is up to you, as long as they match between 
* the host side driver and the device.
* 
* -wValue and wIndex only have a meaning for requests defined in the USB specification 
* or a device class specification. For vendor requests, you can send arbitrary data.
* 
* -wLength is the length of the data block which is sent or received.
* 
* When the host sends or receives a control message on endpoint 0 which is addressed to
* "vendor" or "class", the function usbFunctionSetup() is called in your code. It has 
* one parameter: A pointer to the 8 bytes of setup data. It is your duty to check which 
* functionality should be performed and whether more data should be received or returned
*  to the host.
* 
*  -----XBox USB Sequence----------------------------------------------------------------
* 
*  bmRequestType: c1
*  bRequest: 	  06
*  wValue: 		  00 42 
*  wIndex: 		  00
*  wLength: 	  10
*  
*  bmRequestType: a1
*  bRequest: 	  01
*  wValue: 		  00 01
*  wIndex: 		  00
*  wLength: 	  14
*  
*  bmRequestType: c1
*  bRequest: 	  01
*  wValue: 		  00 02
*  wIndex: 		  00
*  wLength: 	  06
*  
*  bmRequestType: c1
*  bRequest: 	  01
*  wValue: 		  00 01
*  wIndex: 		  00
*  wLength: 	  14
*/

uchar actuator[2] = {0x00,0x00};

bool mode = 0;


void setup_10ms_int();
void fakeDisconnect();
void xbox_reset_pad_status(void);
void xbox_set_pad_status(void);
void ps2_to_xbox();
void pin_check();

USB_JoystickReport_Data_t gamepad_state;
XIDGamepadOutputReport out_XID_report;

//--------------------------------------------------------------------------------------------//
//							usbdrv function definitions										 //
//------------------------------------------------------------------------------------------//
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (void *)data;
	DBG1(0x0a, &rq->bmRequestType, 1);DBG1(0x1a, &rq->bRequest, 1);DBG1(0x2a, &rq->wValue.bytes[0], 1);
	DBG1(0x3a, &rq->wValue.bytes[1], 1);DBG1(0x4a, &rq->wIndex, 1);DBG1(0x4a, &rq->wLength, 1);
	
	if (rq->bmRequestType == (USBRQ_RCPT_INTERFACE | USBRQ_TYPE_VENDOR | USBRQ_DIR_DEVICE_TO_HOST)){//0xc1
		if (rq->bRequest == USBRQ_GET_DESCRIPTOR){//0x06	 XID Descriptor Report
			 if(rq->wValue.bytes[1] == 0x42){
				//DBG1(0xc1, &rq->bmRequestType,1);
				usbMsgPtr = (usbMsgPtr_t ) usbDescriptorHidReport;
				usbMsgFlags = USB_FLG_MSGPTR_IS_ROM;
				return 16;
			 }
		}
		else if (rq->bRequest == USBRQ_HID_GET_REPORT){//0x01 XID_GET_CAPABILITIES
			 /*The data will be similar to the GET_REPORT, but instead of storing actual values, 
			 it will have bits set (1) where the bit is valid in the respective report. If the bit
			 is auto-generated, it will be cleared (0).  */
			 if(rq->wValue.bytes[1] == USBDESCR_DEVICE){//0x01 in_state_capabilities
			 //tell the host what type of data should it be waiting to receive from our device
			 	usbMsgPtr = (usbMsgPtr_t ) &gamepad_state;
			 	pad = 1;
			 	DBG1(0xf2, &rq->bmRequestType,1);
			 	return sizeof(gamepad_state);	
 
			 }
			 else if(rq->wValue.bytes[1] == USBDESCR_CONFIG){//0x02 out_state_capabilities n1
			 //tell the host what type of data we are expecting from him
			 	DBG1(0xc2, &rq->wLength, 1);
			 	usbMsgPtr = (usbMsgPtr_t ) &out_XID_report;
			 	return sizeof(out_XID_report);
			 	
			 }

		}

	}
	//Need to send ACK back to the Host if the GET_REPORT is supported by our device
	else if (rq->bmRequestType == (USBRQ_TYPE_CLASS | USBRQ_DIR_DEVICE_TO_HOST | USBRQ_RCPT_INTERFACE)){//0xa1
		 if (rq->bRequest == USBRQ_HID_GET_REPORT){//0x01
		 		usbMsgPtr = (usbMsgPtr_t ) &gamepad_state;
		 		DBG1(0xf1, &rq->bmRequestType,1);
		 		TCCR0B |= (1<<CS02) | (1<<CS00); //start the interrupt timer
		 		return sizeof(gamepad_state);
	 
		 }
	}
	else if (rq->bmRequestType == 0x21){// Bulk-Out from host with actuator data
		if(rq->bRequest == USBRQ_HID_SET_REPORT){//0x09
			if(rq->wValue.bytes[1] == 0x02){
				DBG1(0xCC, 0, 0); //21
				return USB_NO_MSG;//return USB_NO_MSG so that the driver knows that usbFunctionWrite() should be called
								 //aka the devices is waiting for the host to write(send to the device) some data
			}
		}
	}
	DBG1(0x0b, &rq->bmRequestType, 1); //21
	DBG1(0x1b, &rq->bRequest, 1);  //09
	DBG1(0x2b, &rq->wValue.bytes[1], 1); //02
    return 0;   /* default for not implemented requests: return no data back to host */
}

/*
* usbFunctionDescriptor() is not called at all on our setup, but it's left here
* for completion
*/	
USB_PUBLIC usbMsgLen_t usbFunctionDescriptor(struct usbRequest *rq) {
	DBG1(0xBa, 0, 0);
	if (rq->bRequest == USBRQ_GET_DESCRIPTOR)
	{
		DBG1(0xBC, 0, 0);
		switch (rq->wValue.bytes[1])
		{
			case USBDESCR_CONFIG: 
				usbMsgPtr = (usbMsgPtr_t ) usbDescriptorConfiguration;
				usbMsgFlags = USB_FLG_MSGPTR_IS_ROM;
				return 32;
		 
			case USBDESCR_DEVICE:
				usbMsgPtr = (usbMsgPtr_t ) usbDescriptorDevice;
				usbMsgFlags = USB_FLG_MSGPTR_IS_ROM;
				return 18;
				
		}
	}
	
	return 0; 
	DBG1(0xBd, 0, 0);
}

/*
 * Bulk-out (actuator data) from the host.
 */
USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len)
{	
/*
* USB Driver dictates:
* You must define USB_CFG_IMPLEMENT_FN_WRITE to 1 in usbconfig.h if you use this method.
* To tell the driver to use usbFunctionWrite(),return USB_NO_MSG; inside usbFunctionSetup()
* The endpoint number can be found in the
* global variable 'usbRxToken'.
*/
	actuator[0] = data[3];//Left actuator strength 
	actuator[1] = data[5];//Right actuator strength
	DBG2(0xAA, &actuator[0], 1);
	DBG2(0xAB, &actuator[1], 1);
/*If you have received the entire payload  successfully, return 1. If you expect more data, return 0*/
	return 1;
}

//--------------------------------------------------------------------------------------------//
//							Block End										                 //
//------------------------------------------------------------------------------------------//


/* Useless atm, those pins are not used for anything */
 void pin_setup(){

	DDRC  &= ~((1<<PC0 | 1<<PC1 | 1<<PC2));//make pins input
	PORTC |= (1<<PC0 | 1<<PC1 | 1<<PC2);// turn on pullups
}

/* Read button input from the ps2 controller */
void pin_check(){
	//xbox_set_pad_status(); //Zero the report struct
	gamepad_state = translatePS2toXbox(getPS2ControllerInputData()); //Read button input from the ps2 controller
	
}




int __attribute__((noreturn)) main(void)
{
	_delay_ms(5);
	odDebugInit();
	setup_10ms_int();// interrupt for ingame controller setup
	pad = 0;
	uint8_t cntr = 0;
	spi_mInit(); //initialize spi interface for ps2 controller
	xbox_reset_pad_status(); // 0xff the report struct
    wdt_enable(WDTO_1S);
	DBG1(0x00, 0, 0);
	cli();
	usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
	fakeDisconnect();
	setup_actuator(); //setup rumbling on the controller
    usbDeviceConnect();
	sei();
	while(pad !=1 ){
		wdt_reset();
		usbPoll();
	}
	while(cntr<250){ //wait some time for the usb device setup sequence to finish
		wdt_reset(); usbPoll(); cntr++;
	}
	setup_actuator();
	for(;;){
		while(!usbInterruptIsReady3()){ wdt_reset(); usbPoll();}
		pin_check();//read controller inputs
		//send Interrupt IN data for endpoint 3
		usbSetInterrupt3((uchar *) &gamepad_state, 20); //send controller inputs to the host
		/* To send 20 bytes of interrupt-in changes have been made to "usbdrv/usbdrv.h", "usbdrv/usbrv.c", "usbdrv/asmcommon.inc" and "usbconfig.h" */
	}
}

/*
* The data will be similar to the GET_REPORT, but instead of storing actual values, it will
* have bits set (1) where the bit is valid in the respective report. If the bit is
* auto-generated, it will be cleared (0). 
*/
void xbox_reset_pad_status(void) {
	//0xff makes the byte active
	//0x00 makes xbox ignore the byte
		wdt_reset();
		memset(&gamepad_state, 0xFF, sizeof(gamepad_state));
		gamepad_state.rid = 0x00;
		gamepad_state.rsize = 0x14;
		gamepad_state.reserved_1 = 0x00;
		gamepad_state.l_x0 = 0;
		gamepad_state.l_y0 = 0;
		gamepad_state.r_x0 = 0;
		gamepad_state.r_y0 = 0;
		out_XID_report.rid = 0x00;
		out_XID_report.rsize = 0x06;
		out_XID_report.left_actuator_strength0 = 0x00;
		out_XID_report.left_actuator_strength = 0xff;
		out_XID_report.right_actuator_strength0 = 0x00;
		out_XID_report.right_actuator_strength = 0xff;
	}

/* Zero the controller input report structs before we read values from the ps2 controller */
void xbox_set_pad_status(void) {
		wdt_reset();
		//memset(&gamepad_state, 0x00, sizeof(gamepad_state));
	
		out_XID_report.rsize = 0x06;
		out_XID_report.rid = 0x00;
		out_XID_report.left_actuator_strength=0x00;
		out_XID_report.right_actuator_strength=0x00;
	}

/*
 * Setup a 10ms timeout. When the controller is connected to the MS Dashboard, the USB enumeration process stops
 * at the bmRequestType = 0xa1 and no other requests are received by the controller(when connected to xbox games 
 * or other dashboards the enumeration process continues with 2 more requests ). We start the 10ms timeout timer
 * when the 0xa1 request is completed. 10ms is enough time for any type of enumeration to be completed.
 */
void setup_10ms_int(){
	TCCR0A |= (1<<WGM01); //CTC MODE
	//TCCR0B |=(1<<CS02) | (1<<CS00);// clk/1024 Selecting clock source starts the timer/counter0. Don't want to start it here
	TCCR0B = 0; //stop the timer
	OCR0A = 155;// (16*10^6)/1024=15,625kHz .....10ms=100hz.....15,625k/500=156....156-1cycle=155
	TIMSK0 |= (1<<OCIE0A); //When the OCIE0A bit is written to one, and the I-bit in the status register is set, the Timer/Counter0 compare match A interrupt is enabled.
	TCNT0 = 0; //reset the timer/counter0
	TIFR0 =(1<<OCF0A);//clear interrupt Output Compare Match Flag
}

/* fake USB disconnect for > 250 ms */
void fakeDisconnect(){
    uchar i = 0;
    while(--i){
		wdt_reset();             
        _delay_ms(1);
    }
	DBG1(0x01, 0, 0);
}

/* The ISR interrupt vector for the 10ms interrupt */
ISR(TIMER0_COMPA_vect){
	pad = 1;
	//DBG1(0x12, 0, 0);
	TCCR0B = 0; //stop the timer
	TIFR0 = (1<<OCF0A);//clear interrupt Output Compare Match Flag
	TIMSK0 &= ~(1<<OCIE0A); //disabled interrupt
}

















/*
* Interrupt-In endpoints are used to send data to the host spontaneously. Bulk-In endpoints 
* are for stream type data.
*/

/*
* VUSB driver can only send Interrupt-In packets of maximun 8 bytes of length.
* The Xbox controller needs to send pad data(buttons pressed) in packets of 20 bytes length.
* To emulate an 20 bytes packet we need to send it in chunks of 8, 8 and last 4 bytes long packages(8+8+4=20).
* getInterruptData uses a pointer to build the packets.
*/
	//uchar chunk = 0;
	//uchar *p;
// uchar getInterruptData(){
	// p=input_capabilities +(chunk*8);chunk++;
	// if(chunk==3){
		// //pad=0;
		// chunk=0;
		// return 4;
	// }
	// return 8;
// }
/*
* You must define USB_CFG_IMPLEMENT_FN_READ to 1 in usbconfig.h if you use this method.
* To tell the driver to use usbFunctionRead(), return USB_NO_MSG; inside usbFunctionSetup()
*/
//uint8_t *bytePtr;
// uchar   usbFunctionRead(uchar *data, uchar len)
// {
	// bytePtr = (uint8_t*)&gamepad_state;
	// bytePtr+=currentAddress;
	// DBG1(0x00, &bytesRemaining, 1);
	// wdt_reset();
    // if(len > bytesRemaining){
		// len = bytesRemaining;}
    // for(int i=0;i<len;i++){
		// bytePtr+=i;
		// data[i]=*bytePtr;
		// }
    // currentAddress += len;
    // bytesRemaining -= len;
    // return len;
// }
// ISR(WDT_vect) // Watchdog timer interrupt
// {
	// DBG1(0x90,0,0);
// }

/* Interrupt- and Bulk-Out endpoints are used to send stream type data to the device. When the 
* host sends a chunk of data on the endpoint, *the function usbFunctionWriteOut() is called. 
* If you use more than one interrupt- or bulk-out endpoint, the endpoint number is passed in 
* the global variable usbRxToken. You must define USB_CFG_IMPLEMENT_FN_WRITEOUT to 1 in 
* usbconfig.h when you use this feature.
*
* usbRxToken holds the Interrupt-Out address.
*/
// USB_PUBLIC void usbFunctionWriteOut(uchar *data, uchar len)
// {
	// DBG1(0x04, data, len);
	// if(usbRxToken==0x02){pad=1;};
	// DBG1(0x02, &usbRxToken, 1);
// }

