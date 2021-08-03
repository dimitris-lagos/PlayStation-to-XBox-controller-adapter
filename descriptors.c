

const char usbDescriptorDevice[18]PROGMEM=
{
	0x12,        // bLength
	0x01,        // bDescriptorType (Device)
	0x10, 0x01,  // bcdUSB 1.10
	0x00,        // bDeviceClass 
	0x00,        // bDeviceSubClass 
	0x00,        // bDeviceProtocol 
	0x08,        // bMaxPacketSize0 8
	0x5E, 0x04,  // idVendor 0x045E
	0x02, 0x02,  // idProduct 0x0202
	0x00, 0x01,  // bcdDevice 1.00
	0x00,        // iManufacturer (String Index)
	0x00,        // iProduct (String Index)
	0x00,        // iSerialNumber (String Index)
	0x01        // bNumConfigurations 1
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
 
const int usbDescriptorStringVendor[]PROGMEM={
	'M', 'i', 'c', 'r', 'o', 's', 'o', 'f', 't', ' ', 'C', 'o', 'r', 'p', '.'
};
const int usbDescriptorStringDevice[]PROGMEM={
	'X', 'b', 'o', 'x', ' ', 'C', 'o', 'n', 't', 'r', 'o', 'l', 'l', 'e', 'r'
};


const char usbDescriptorConfiguration[]PROGMEM=
{
//Configuration Descriptor:	(Config)
	0x09,        // bLength
	0x02,        // bDescriptorType (Configuration)
	0x20, 0x00,  // wTotalLength 32
	0x01,        // bNumInterfaces 1
	0x01,        // bConfigurationValue
	0x00,        // iConfiguration (String Index)
	0x80,        // bmAttributes Remote Wakeup
	0xFA,        // bMaxPower 500mA
	
//Interface Descriptor:	(HID_Interface)
	0x09,        // bLength
	0x04,        // bDescriptorType (Interface)
	0x00,        // bInterfaceNumber 0
	0x00,        // bAlternateSetting
	0x02,        // bNumEndpoints 2
	0x58,        // bInterfaceClass (USB_CLASS_XID)
	0x42,        // bInterfaceSubClass (USB_DT_XID)
	0x00,        // bInterfaceProtocol
	0x00,        // iInterface (String Index)
//Endpoint Descriptor:	(HID_ReportINEndpoint)	
	0x07,        // bLength
	0x05,        // bDescriptorType (Endpoint)
	0x82,        // bEndpointAddress (IN/D2H)
	0x03,        // bmAttributes (Interrupt)
	0x20, 0x00,  // wMaxPacketSize 32!!!(Although xbox controller marks 32bytes, on VUSB only 8bytes can be sent! But this version is modded to send 20 bytes) change 0x20 to 0x08 for windows support, but no xbox
	0x04,        // bInterval 4 (unit depends on device speed)
	
//Endpoint Descriptor:	(HID_ReportOUTEndpoint)	
	0x07,        // bLength
	0x05,        // bDescriptorType (Endpoint)
	0x02,        // bEndpointAddress (OUT/H2D)
	0x03,        // bmAttributes (Interrupt)
	0x20, 0x00,  // wMaxPacketSize 32 change 0x20 to 0x08 for windows support, but no xbox
	0x04       // bInterval 4 (unit depends on device speed)

};

//This is an example XID descriptor report
const char usbDescriptorHidReport[]PROGMEM=
{
 0x10, //bLength(16 bytes)              
 0x42, //bDescriptorType      
 0x00, 0x01, //bcdXid (USB 1.0)              
 0x01, //bType 
 0x02, //bSubType
 0x14, //bMaxInputReportSize (20 bytes)
 0x06, //bMaxOutputReportSize (6 bytes) 
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF //wAlternateProductIds (4 x 0xFFFF)
 };
 

typedef struct {
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

typedef struct {
    uint8_t  rid; 
    uint8_t  rsize; // Has to be 0x06
	uint8_t left_actuator_strength0; //useless
    uint8_t left_actuator_strength;
	uint8_t right_actuator_strength0; //useless
    uint8_t right_actuator_strength;
} XIDGamepadOutputReport; 
 
#define XBOX_DPAD_UP		0x01
#define XBOX_DPAD_DOWN		0x02
#define XBOX_DPAD_LEFT		0x04
#define XBOX_DPAD_RIGHT		0x08
#define XBOX_START			0x10
#define XBOX_BACK			0x20
#define XBOX_LEFT_STICK		0x40
#define XBOX_RIGHT_STICK	0x80

#define PS2_DPAD_UP			(1<<4)
#define PS2_DPAD_DOWN		(1<<6)
#define PS2_DPAD_LEFT		(1<<7)
#define PS2_DPAD_RIGHT		(1<<5)
#define PS2_START			(1<<3)
#define PS2_SELECT			(1<<0)
#define PS2_L3				(1<<1)
#define PS2_R3				(1<<2)
							
#define PS2_S				(1<<7)
#define PS2_X				(1<<6)
#define PS2_O				(1<<5)
#define PS2_T				(1<<4)
#define PS2_R1				(1<<3)
#define PS2_L1				(1<<2)
#define PS2_R2				(1<<1)
#define PS2_L2				(1<<0)


