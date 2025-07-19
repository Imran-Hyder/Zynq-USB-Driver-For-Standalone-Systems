#include "zynq_usb.h"


#define MIO_47_TO_32_ADDRESS 0xE000A008
#define USB_RESET_MASK 0xBFFF0000
#define USB_ENABLE_MASK 0xBFFF4000

#define PS_USB0_RST	0x00000001
#define PS_USB1_RST	0x00000002
#define PS_USB_RST_CTRL_REG	(XPS_SYS_CTRL_BASEADDR + 0x210)

void reset_usb_phy( void );
void usb_CTRL_reset( uint8_t port );
