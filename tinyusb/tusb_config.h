/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif
//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------
#include "../zynq_usb.h"

#define CFG_TUSB_MCU				OPT_MCU_ZYNQ7000
#define CFG_TUH_ENDPOINT_MAX		8 //XUSBPS_MAX_ENDPOINTS

/* Uncomment this to have loads of debug messages from the serial port */
//#define CFG_TUSB_DEBUG			3
//#define CFG_TUSB_DEBUG_PRINTF		xwrap_printf

#define TUP_USBIP_EHCI					// Our USB is a generic EHCI by Chip Idea with a couple of enhancements see Xilinx docs.
#define TUP_USBIP_CHIPIDEA_HS	   			// We can enable this to save memory but we don't really need to!

#define CFG_TUSB_OS_INC_PATH		E:/GitRB/FreeRTOS-Kernel/include/

#ifndef CFG_TUSB_MCU
  #error CFG_TUSB_MCU must be defined
#endif

// Enable Host stack, Default is max speed that hardware controller could support with on-chip PHY

#define CFG_TUH_ENABLED       	1
#define CFG_TUH_MAX_SPEED     	OPT_MODE_DEFAULT_SPEED
#define CFG_TUSB_RHPORT0_MODE		(OPT_MODE_HOST | CFG_TUH_MAX_SPEED)
#define CFG_TUH_TASK_QUEUE_SZ		256

#define CFG_TUSB_OS 	OPT_OS_NONE
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_NONE
#endif


/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#define CFG_TUSB_MEM_SECTION		__attribute__ (( section(".usb_mem") ))
#define CFG_TUSB_MEM_ALIGN		TU_ATTR_ALIGNED(4)

//--------------------------------------------------------------------
// CONFIGURATION
//--------------------------------------------------------------------

// Size of buffer to hold descriptors and other data used for enumeration
#define CFG_TUH_ENUMERATION_BUFSIZE 1024

#define CFG_TUH_HUB		0
#define CFG_TUH_CDC		0
#define CFG_TUH_HID		4 // typical keyboard + mouse device can have 3-4 HID interfaces
#define CFG_TUH_MSC		0
#define CFG_TUH_VENDOR	0

// max device support (excluding hub device)
// 1 hub typically has 4 ports
#define CFG_TUH_DEVICE_MAX          (CFG_TUH_HUB ? 4 : 1)

//------------- HID -------------//
#define CFG_TUH_HID_EPIN_BUFSIZE 64
#define CFG_TUH_HID_EPOUT_BUFSIZE 64

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
