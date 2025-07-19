#include <stdio.h>
#include <sleep.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xstatus.h"
#include "xgpio.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xil_io.h"

#include "ZYNQ_USB_DRIVER/tinyusb/tusb.h"
#include "ZYNQ_USB_DRIVER/zynq_usb.h"

XScuGic	xInterruptController;
static XScuGic_Config *interruptControllerConfig;

cpu0_globals_t	*cpu0_globals;

bool Interrupt_System_Initialization(){

	bool status;

	//Interrupt Controller Initialisation
	interruptControllerConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if(NULL == interruptControllerConfig){
		return XST_FAILURE;
	}
	status = XScuGic_CfgInitialize(&xInterruptController, interruptControllerConfig, interruptControllerConfig->CpuBaseAddress);
	if(status != XST_SUCCESS){
		return XST_FAILURE;
	}
	//Interrupt Controller Registration
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler , &xInterruptController);
	Xil_ExceptionEnable();
	return 0;
}

int main() {

    	init_platform();
    	Interrupt_System_Initialization();
	cpu0_globals = (cpu0_globals_t *)malloc(sizeof(cpu0_globals_t));
	memset( (void*)cpu0_globals, 0, sizeof(cpu0_globals_t));
	tusb_init();
	while(1){
		tuh_task();
	}
}


