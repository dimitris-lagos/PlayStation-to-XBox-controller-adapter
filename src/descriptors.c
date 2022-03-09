#include "descriptors.h"
#include "usbdrv.h"

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */


PROGMEM const char usbDescriptorDevice[] =
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

 
PROGMEM const int usbDescriptorStringVendor[]  ={
	'M', 'i', 'c', 'r', 'o', 's', 'o', 'f', 't', ' ', 'C', 'o', 'r', 'p', '.'
};
PROGMEM const int usbDescriptorStringDevice[] ={
	'X', 'b', 'o', 'x', ' ', 'C', 'o', 'n', 't', 'r', 'o', 'l', 'l', 'e', 'r'
};


PROGMEM const char usbDescriptorConfiguration[] =
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

//The XID Descriptor Report
PROGMEM const char usbDescriptorHidReport[] =
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
 