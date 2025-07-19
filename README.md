# Zynq-USB-Driver-For-Standalone-Systems

It is a modified driver derived from https://github.com/lvgl/lv_port_xilinx_zedboard_vitis/tree/master which was created for RTOS systems. It was tested on mouse, keyboard and Raspberry Pi touchscreen.
It was developed for the Arty Z7-20 development board.

Different development boards have different USB reset procedures. Change support.c and support.h according to your board. Also, you can add parsing function for your specific device in hcd_zynq.c. Connect to a serial terminal to view some notifications like device mounted or unmounted.
