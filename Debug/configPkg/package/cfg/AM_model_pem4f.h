/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-A44
 */

#include <xdc/std.h>

#include <ti/sysbios/family/arm/lm4/Timer.h>
extern const ti_sysbios_family_arm_lm4_Timer_Handle ti_sysbios_family_arm_lm4_Timer0;

#define TI_DRIVERS_EMAC_INCLUDED 1

#define TI_DRIVERS_GPIO_INCLUDED 1

#define TI_DRIVERS_I2C_INCLUDED 0

#define TI_DRIVERS_SDSPI_INCLUDED 0

#define TI_DRIVERS_SPI_INCLUDED 0

#define TI_DRIVERS_UART_INCLUDED 1

#define TI_DRIVERS_USBMSCHFATFS_INCLUDED 0

#define TI_DRIVERS_WATCHDOG_INCLUDED 0

#define TI_DRIVERS_WIFI_INCLUDED 0

extern int xdc_runtime_Startup__EXECFXN__C;

extern int xdc_runtime_Startup__RESETFXN__C;

#ifndef ti_sysbios_knl_Task__include
#ifndef __nested__
#define __nested__
#include <ti/sysbios/knl/Task.h>
#undef __nested__
#else
#include <ti/sysbios/knl/Task.h>
#endif
#endif

extern ti_sysbios_knl_Task_Struct TSK_idle;

