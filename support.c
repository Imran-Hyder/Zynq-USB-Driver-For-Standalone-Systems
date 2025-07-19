#include "xparameters.h"
#include "sleep.h"
#include "xil_misc_psreset_api.h"
#include "support.h"

volatile uint32_t *mask_data_reg = (volatile uint32_t *)MIO_47_TO_32_ADDRESS;

void reset_usb_phy(void) {
    *mask_data_reg = USB_RESET_MASK;
    usleep(1000);
    *mask_data_reg = USB_ENABLE_MASK;
}

void usb_CTRL_reset(uint8_t port) {

	uint32_t	reset;
	/* Unlock the slcr register access lock */
	Xil_Out32(XSLCR_UNLOCK_ADDR, XSLCR_UNLOCK_CODE);

	if( !port ) reset = PS_USB0_RST;
	else reset = PS_USB1_RST;

	Xil_Out32(PS_USB_RST_CTRL_REG, reset);
	usleep(1000);
	Xil_Out32(PS_USB_RST_CTRL_REG, 0);

}
