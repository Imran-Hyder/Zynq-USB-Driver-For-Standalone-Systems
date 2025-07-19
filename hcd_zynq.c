/*
 * Zynq USB HID Host implementation using TinyUSB stack.
 * This is pretty basic right now and needs plenty of work in
 * the future - be warned.
 *
 * Connect devices to the OTG port on Zedboard. I have only used
 * one device at a time with no hubs.  You will have to modify the
 * code to use a hub and support multiple devices simultaneously!
 *
 * See Github here: https://github.com/hathach/tinyusb
 * Most of the credit needs to go to hathach for his USB lib :-)
 *
 *
 * So far it does seem to work with a generic USB mouse,
 * generic keyboard and a couple of Waveshare Ilitek based touch screen
 * displays with thes values for defines in hid_touch.h:
 * 13.3" touch parameters
 * #define ILI_MAX_X			16384
 * #define ILI_MAX_Y			9600
 * https://www.waveshare.com/product/displays/lcd-oled/lcd-oled-1/13.3inch-hdmi-lcd-h-with-case.htm
 *
 * 15.6" screen parameters
 * #define ILI_MAX_X			9600
 * #define ILI_MAX_Y			9600
 * https://www.waveshare.com/product/displays/lcd-oled/lcd-oled-1/15.6inch-hdmi-lcd-h-with-case.htm
 *
 *
 * NOTE: for Waveshare screen VGA connection this cable is also required:
 * https://www.waveshare.com/Mini-HDMI-Male-to-VGA-Female-Cable.htm
 *
 *
 * Copyright (c) 2021 Pete J. Bone - PJBES
 *
 */
#include "tinyusb/common/tusb_common.h"


#if CFG_TUH_ENABLED && (CFG_TUSB_MCU == OPT_MCU_ZYNQ7000)


//--------------------------------------------------------------------+
// INCLUDE
//--------------------------------------------------------------------+
#include "xil_mmu.h"
#include "support.h"
#include "hid_touch.h"
#include "tinyusb/host/usbh.h"
#include "zynq_usb.h"
#include "tinyusb/portable/ehci/ehci_api.h"
#include "tinyusb/portable/ehci/ehci.h"
#include "tinyusb/class/hid/hid_host.h"

extern XScuGic 					xInterruptController; 	/* Interrupt controller instance */
extern cpu0_globals_t			*cpu0_globals;
extern uint8_t					_usb_mem[];					/* Get the address of the usb memory allocated in linker file */

#define MAX_REPORT  8

//--------------------------------------------------------------------+
// HCD API
//--------------------------------------------------------------------+
bool hcd_init(uint8_t rhport) {

	Xil_SetTlbAttributes( (uint32_t)_usb_mem, NORM_NONCACHE );  // No Cache on this block of memory for USB DMA transfers
	if( !zynq_usb_init() ) return false;
	return( ehci_init(rhport, (uint32_t) &cpu0_globals->usb.regs->CapLen_HciVersion, (uint32_t) &cpu0_globals->usb.regs->Cmd) );
}

void hcd_int_enable(uint8_t rhport) {

	XScuGic_Enable(&xInterruptController, XPS_USB0_INT_ID);
}

void hcd_int_disable(uint8_t rhport) {

	XScuGic_Disable(&xInterruptController, XPS_USB0_INT_ID);
}

// Each HID instance can have multiple reports CFG_TUSB_MEM_SECTION CFG_TUSB_MEM_ALIGN
struct {

	uint8_t report_count;
	tuh_hid_report_info_t report_info[MAX_REPORT];
} hid_info[CFG_TUH_HID];

static void process_kbd_report(hid_keyboard_report_t const *report);
static void process_mouse_report(hid_mouse_report_t const * report);
static void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);

void hid_app_task(void)
{
  // nothing to do
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {

	const char*						protocol_str[] = { "None", "Keyboard", "Mouse" };
	hid_interface_protocol_enum_t	itf_protocol;
	static char					msgt[128];

	itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
	snprintf( msgt, sizeof(msgt), "HID Address = %d, Instance = %d is mounted - HID Protocol = %s",
			dev_addr, instance, protocol_str[itf_protocol]);
	xil_printf( msgt );
	xil_printf( "\r\n" );

	/*Set a cursor for the mouse*/
	if ( itf_protocol == HID_ITF_PROTOCOL_NONE ) {
		hid_info[instance].report_count = tuh_hid_parse_report_descriptor(hid_info[instance].report_info, MAX_REPORT, desc_report, desc_len);
		snprintf( msgt, sizeof(msgt), "HID has %u reports", hid_info[instance].report_count);
		xil_printf( msgt );
		xil_printf( "\r\n" );
	}
	bool res = !tuh_hid_receive_report(dev_addr, instance);
	printf("%d\n",res);
	// Request to receive report
	// tuh_hid_report_received_cb() will be invoked when report is available
	if ( res ) {
		snprintf( msgt, sizeof(msgt), "Error: cannot request to receive report\r\n" );
		xil_printf( msgt);
		xil_printf( "\r\n" );
	}
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {

	hid_interface_protocol_enum_t	itf_protocol;
	static char					msgt[128];

	itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
	snprintf( msgt, sizeof(msgt), "HID device address = %d, instance = %d is unmounted", dev_addr, instance);
	xil_printf( msgt );
	xil_printf( "\r\n" );
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {

	hid_interface_protocol_enum_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

	switch( itf_protocol ) {

		case HID_ITF_PROTOCOL_KEYBOARD:
			TU_LOG2("HID receive keyboard report\r\n");
			process_kbd_report( (hid_keyboard_report_t const*) report );
			break;

		case HID_ITF_PROTOCOL_MOUSE:
			TU_LOG2("HID receive mouse report\r\n");
			process_mouse_report( (hid_mouse_report_t const*) report );
			break;

		default:
			// Generic report requires matching ReportID and contents with previous parsed report info
			process_generic_report(dev_addr, instance, report, len);
			break;
	}

	// Request to receive report
	if( !tuh_hid_receive_report(dev_addr, instance) ) xil_printf("Error: cannot request to receive report\r\n");
}

//--------------------------------------------------------------------+
// Keyboard Processing
//--------------------------------------------------------------------+
static uint8_t const keycode2ascii[128][2] =  { HID_KEYCODE_TO_ASCII };

// look up new key in previous keys
static inline bool find_key_in_report(hid_keyboard_report_t const *report, uint8_t keycode) {

	for(uint8_t i=0; i<6; i++) {
		if( report->keycode[i] == keycode )  return true;
	}
	return false;
}

static void process_kbd_report(hid_keyboard_report_t const *report) {

	static hid_keyboard_report_t prev_report = { 0, 0, {0} }; // previous report to check key released

	//------------- ignore control (non-printable) key affects for now -------------//
	for(uint8_t i=0; i<6; i++) {
		if ( report->keycode[i] ) {
			if ( find_key_in_report(&prev_report, report->keycode[i]) ) {
				// exist in previous report means the current key is holding
			} else {
				// not existed in previous report means the current key is pressed
				bool const is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
				uint8_t ch = keycode2ascii[report->keycode[i]][is_shift ? 1 : 0];
				putchar(ch);
				printf("%c",ch);
				if ( ch == '\r' ) putchar('\n'); // added new line for enter key	THIS IS TESTING TO the Serial port needs sorting for GUI!!

				fflush(stdout); // flush right away, else nanolib will wait for newline
			}
		}
	// TODO  skips key released!
	}
	prev_report = *report;
}

//--------------------------------------------------------------------+
// Mouse Processing
//--------------------------------------------------------------------+

static void process_mouse_report(hid_mouse_report_t const * report) {

	uint8_t	l_click = FALSE;

	if( report->buttons & MOUSE_BUTTON_LEFT ) l_click = TRUE;  // We only care about left click for now...

	// Cursor movement update, a USB mouse provides a delta value up to +/- 127 from previous report it's down to us to track and sanity check!
	if( report->y & 0x80 ) { // Check if negative
		printf("Up:%d",report->y);
	} else if( report->y ) {
		printf("Down:%d",report->y);
	}
	if( report->x & 0x80 ) {
		printf("Left:%d",report->x);
	} else if( report->x ) {
		printf("Right:%d",report->x);
	}
}

static void process_ilitek_touchscreen_report(hid_ilitek2302_t *report) {

	for( uint8_t i = 0; i < report->ContactCount; i++ ) {

		if( report->points[i].TouchID == 0 ) { 			// Right now we are not attempting multi-touch! so always act on for the first valid touch

			printf("%d",(int32_t)((double)((double)report->points[i].TouchX)));
			printf("%d",(int32_t)((double)((double)report->points[i].TouchX)));
		}
	}
}
//--------------------------------------------------------------------+
// Generic Report
//--------------------------------------------------------------------+

int screen_x,screen_y,touchEnabled;

static void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {

	(void) dev_addr;
	uint8_t const 				rpt_count = hid_info[instance].report_count;
	tuh_hid_report_info_t* 		rpt_info_arr = hid_info[instance].report_info;
	tuh_hid_report_info_t* 		rpt_info = NULL;
	uint16_t vid, pid;

	int combined_x = (report[4] * 256) + report[3];
	int combined_y = (report[6] * 256) + report[5];

	screen_x = (combined_x * 800) / (8 * 256); // Scale to 800
	screen_y = (combined_y * 600) / (8 * 256); // Scale to 600

	touchEnabled = report[1];
}
#endif
