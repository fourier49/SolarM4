/*
 * Copyright (c) 2013, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *    ======== tcpEcho.c ========
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Log.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* NDK Header files */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "Board.h"

/* AM_LIB RS-485 Header files */
#include "./AM_LIB/rs485.h"
#include "./Bus_Raw_Protocol/Bus_Raw_Protocol.h"
#include "./Bus_Raw_Protocol/Table.h"
#include "AM_LIB/TCP_UDP_network.h"

/* AM_LIB UART Header files */
#include "./AM_LIB/UART_PRINTF.h"

#include <stdio.h>
#include <stdlib.h>

//print IP
//#include <NETTOOLS.H>

//#include "RTC_Timer.h"

UART_Handle uart;
/*    Function Prototype   */
void Create_uartHandler();
Void NEthuartHandler(UArg arg0, UArg arg1);
unsigned char task_Event;
extern int JB_Count;
extern int JB_Number;
extern int Bus_ID;
ENVCONFIG G_ENVCONFIG;
int Periodic_Count = 2;
//debug
int error_count = 0;
bool JB_DVAL = 0;
FILE *error_list;

bool Time_DVAL = 0;

Array_Time AM_time;
Array_Time Periodic_Update_time;
Periodic_Data TCP_Periodic_Link_time;

Member_Table member_table[Number_of_JB];
PV_Value_Table pv_value_table[Number_of_JB];
PV_Info_Table pv_info_table[Number_of_JB];

char* ptr1 = (char*) 0x80000;
//int test_count = 0;
short test_merge = 0;

#if 1
int  Gab_test=5;
void time_testFunction(void)
{
	Gab_test++;
}

#endif
/*
 *  ======== tcpHandler ========
 *  Creates new Task to handle new TCP connections.
 */
Void tcpHandler(UArg arg0, UArg arg1) {
	task_Event = AM_Join;
	unsigned char Join_Flag;
	unsigned char *RTC_Result;
	//unsigned char Link_Flag;
	UINT16 Link_Time;
	char *temp;
	int i = 0;
	int JB_Table_Index;
	int PV_Value_Head;
	TaskSleep(5000);	//wait 5 seconds for DHCP ready
	// Allocate the file descriptor environment for this Task
	fdOpenSession((HANDLE) Task_self());
//	network_initial();	//initial network parameter

#if 1

	//// only for alex m4 cpu register test

	int myflashtestfunciton(void);
	int DynFlashProgram(unsigned int *pui32Data, unsigned int ui32Address,
			unsigned int ui32Count);
	int DynFlashErase(unsigned int ui32Address);
	int a;
	bool connect = 1;

	a = myflashtestfunciton();
	UARTprintf("a=%d \n", a);

	uint16_t value = 0;

	//value = EMACPHYRead(0x400EC000, PHY_PHYS_ADDR, EPHY_MISR1);
	// value = EMACPHYRead(0x400EC000, PHY_PHYS_ADDR, EPHY_MISR2);

	value = EMACPHYRead(0x400EC000, 0, 0x00000010);
	UARTprintf("a=%d \n", value);

	connect = EMACSnow_isLinkUp(0);
	UARTprintf("a=%d \n", connect);




#endif
	while (1) {
		switch (task_Event) {
		case AM_Join:
			UARTprintf((const char*) "----Start AM Join (AMtoServer)----\n");
			Join_Flag = Join_Failed;

			do {
				Join_Flag = AM_JoinRequest();
				UARTprintf((const char*) "Join_Flag: %d  \n", Join_Flag);
				TaskSleep(1000);

			} while (Join_Flag != Join_Success);
			task_Event = Update_RTC;
			task_Event = TCPPeriodicLink;
			JB_DVAL = 1;
			//Create_uartHandler();
			UARTprintf((const char*) "----End AM Join (AMtoServer)----\n");
			break;

		case JB_Join:
			UARTprintf((const char*) "----Start JB Join (JBtoServer)----	\n");
			Join_Flag = Join_Failed;
			JB_Table_Index = 0;
			//error_list = fopen("MacList0.txt","w");
			while (JB_Table_Index < JB_Count) {
				if ((member_table[JB_Table_Index].state == JB_Join2AM)
						&& (member_table[JB_Table_Index].Valid > 0)) {
					Join_Flag = JB_JoinRequest(JB_Table_Index);
					if (Join_Flag == Join_Success) {
						member_table[JB_Table_Index].state = JB_Join2Server;
						//UARTprintf((const char*)"AM_Mac[%d] %x %x %x %x %x %x\n",JB_Table_Index,member_table[JB_Table_Index].MAC[5],member_table[JB_Table_Index].MAC[4],member_table[JB_Table_Index].MAC[3],member_table[JB_Table_Index].MAC[2],member_table[JB_Table_Index].MAC[1],member_table[JB_Table_Index].MAC[0]);
						JB_Table_Index++;
					} else {
						;
					}
					Join_Flag = Join_Failed;
					TaskSleep(1000);
				} else
					JB_Table_Index++;
			}
			//fclose(error_list);
			UARTprintf((const char*) "----End JB Join (JBtoServer)----	\n");
			//task_Event = PV_Periodic;
			task_Event = Nothing;
			//JB_DVAL = 1;
			break;

		case PV_Periodic:
			UARTprintf(
					(const char*) "----Start PV Periodic (JBtoServer)----	\n");
			PV_Value_Head = 0;
			while (PV_Value_Head < JB_Count) {
				PV_Value_Head = PV_Periodic_Trans(PV_Value_Head, JB_Count - 1);
				TaskSleep(1000);
			}

			task_Event = Nothing;
			//UARTprintf((const char*)"----==PV_Value_Head %d	==----\n",PV_Value_Head);
			UARTprintf((const char*) "----End PV Periodic (JBtoServer)----	\n");
			break;

		case Update_RTC:
			RTC_Result = Update_RTC_Request();
			if (RTC_Result != NULL) {
				Time_DVAL = 0;
				// RTC Setting
				AM_time.year = RTC_Result[0];
				AM_time.mon = RTC_Result[1];
				AM_time.mday = RTC_Result[2];
				AM_time.hour = RTC_Result[3];
				AM_time.min = RTC_Result[4];
				AM_time.sec = RTC_Result[5];
				free(RTC_Result);
			}
			Time_DVAL = 1;
			task_Event = Nothing;
			UARTprintf((const char*) "Time: %d/%d/%d-%d:%d:%d \n", AM_time.year,
					AM_time.mon, AM_time.mday, AM_time.hour, AM_time.min,
					AM_time.sec);
			Create_uartHandler();

			UARTprintf((const char*) "Old_IP=");
			for (i = 0; i < 4; i++)
				UARTprintf((const char*) ":%d", G_ENVCONFIG.IP[i]);
			UARTprintf((const char*) "\n");

			UARTprintf((const char*) "Old_SN=");
			for (i = 0; i < 8; i++)
				UARTprintf((const char*) ":%d", G_ENVCONFIG.SeriesNumber[i]);
			UARTprintf((const char*) "\n");

			UARTprintf((const char*) "Old_Polling_Time=");
			for (i = 0; i < 2; i++)
				UARTprintf((const char*) ":%d", G_ENVCONFIG.Pollingtime[i]);
			UARTprintf((const char*) "\n");

			UARTprintf((const char*) "Old_MAC=");
			for (i = 0; i < 6; i++)
				UARTprintf((const char*) ":%x", G_ENVCONFIG.Mac[i]);
			UARTprintf((const char*) "\n");

			break;

		case TCPPeriodicLink:
			Time_DVAL = 0;
			//Link_Flag = TCP_Periodic_Link();
			TCP_Periodic_Link();
			//task_Event = Nothing;
			//Time_DVAL = 1;
			task_Event = Update_RTC;
			break;

		case Environment:
			Rewrite_Environment();
			UARTprintf((const char*) "New_IP=");
			for (i = 0; i < 4; i++)
				UARTprintf((const char*) ":%d", G_ENVCONFIG.IP[i]);
			UARTprintf((const char*) "\n");

			UARTprintf((const char*) "New_SN=");
			for (i = 0; i < 8; i++)
				UARTprintf((const char*) ":%d", G_ENVCONFIG.SeriesNumber[i]);
			UARTprintf((const char*) "\n");

			UARTprintf((const char*) "New_Polling_Time=");
			for (i = 0; i < 2; i++)
				UARTprintf((const char*) ":%d", G_ENVCONFIG.Pollingtime[i]);
			UARTprintf((const char*) "\n");

			UARTprintf((const char*) "New_MAC=");
			for (i = 0; i < 6; i++)
				UARTprintf((const char*) ":%x", G_ENVCONFIG.Mac[i]);
			UARTprintf((const char*) "\n");

			TCP_Periodic_Link_time.Updata_Period[1] =
					G_ENVCONFIG.Pollingtime[0];
			TCP_Periodic_Link_time.Updata_Period[0] =
					G_ENVCONFIG.Pollingtime[1];

			temp = (char *) &Link_Time;
			*temp = TCP_Periodic_Link_time.Updata_Period[0];
			*(temp + 1) = TCP_Periodic_Link_time.Updata_Period[1];
			Clock_setPeriod(Periodic_Handle, Link_Time * Time_Tick);
			UARTprintf((const char*) "\n\n== Period Change : %d sec==\n\n",
					Link_Time);

			UARTprintf((const char*) "I'm new 0x80000 = %x \n", *(ptr1));
			UARTprintf((const char*) "I'm new 0x80001 = %x \n", *(ptr1 + 1));
			UARTprintf((const char*) "I'm new 0x80002 = %x \n", *(ptr1 + 2));
			UARTprintf((const char*) "I'm new 0x80003 = %x \n", *(ptr1 + 3));
			task_Event = AM_Join;
			break;

		default:
			UARTprintf((const char*) "tcpHandler Do Nothing T_T \n");
			break;

		} /*  end switch  */
		TaskSleep(7000);
	} /* end while  */
}

/*
 *  ======== uartHandler ========
 *  Creates new Task to handle new UART connections.
 */
Void uartHandler(UArg arg0, UArg arg1) {
	int i = 0;
	int j = 0;
	//int start_time,end_time;
	int packet_num;
	JB_DVAL = 1;

	rs485_init();

	rs485_write((const uint8*) "RS-485 testing message", 22);
	UARTprintf((const char*) "Debug testing message \n");

	//Reset JB
	for (i = 0; i < 3; i++) {
		Reset_JB();
		TaskSleep(500);
	}
	for (j = 0; j < Number_of_JB; j++) {
		pv_value_table[j].Diode_Temperature = 0xFF;
		for (i = 0; i < 2; i++)
			pv_value_table[j].Voltage[i] = 0xFF;
		for (i = 0; i < 2; i++)
			pv_value_table[j].Current[i] = 0xFF;
		for (i = 0; i < 4; i++)
			pv_value_table[j].Power_Energy[i] = 0xFF;
		for (i = 0; i < 4; i++)
			pv_value_table[j].Alert_State[i] = 0xFF;
		for (i = 0; i < 6; i++)
			pv_info_table[j].MAC[i] = 0xFF;
		for (i = 6; i < 30; i++)
			pv_info_table[j].Serial_Number[i - 6] = 0xFF;
		for (i = 30; i < 54; i++)
			pv_info_table[j].Firmware_Version[i - 30] = 0xFF;
		for (i = 54; i < 78; i++)
			pv_info_table[j].Hardware_Version[i - 54] = 0xFF;
		for (i = 78; i < 102; i++)
			pv_info_table[j].Device_Specification[i - 78] = 0xFF;
		for (i = 102; i < 110; i++)
			pv_info_table[j].Manufacture_Date[i - 102] = 0xFF;
		for (i = 0; i < 6; i++)
			member_table[j].MAC[i] = 0x00;
		member_table[j].Valid = 0;
		member_table[j].state = JB_Free;
	}
	j = 0;

	while (1) {
		if (JB_DVAL == 1) {
			JB_DVAL = 0;
			//UARTprintf((const char*)"P[0] = %d\t P[1] = %d\n",G_ENVCONFIG.Pollingtime[0],G_ENVCONFIG.Pollingtime[1]);
			UARTprintf((const char*) "----Test_merge = %d----\n", test_merge);
			UARTprintf(
					(const char*) "\n\n\n--Update_Time: %d/%d/%d-%d:%d:%d --\n\n\n",
					AM_time.year, AM_time.mon, AM_time.mday, AM_time.hour,
					AM_time.min, AM_time.sec);
			UARTprintf((const char*) "----Start JB Join (JBtoAM)----\n");
			//start_time =  Clock_getTicks();  //calculate JB Join AM Time
			for (i = 0; i < 3; i++)					// Broadcast n times
					{
				//Broadcast_Packet();
				New_Broadcast_Packet_with_Pollingtime();
				packet_num = Rs4852Array(total_array);
				Array2Packet(packet_num, uart);
			}
			//end_time =  Clock_getTicks(); //calculate JB Join AM Time

			//UARTprintf((const char*)"execution time=%d \n",end_time-start_time);

			Check_JB_Number();
			UARTprintf((const char*) "\n----=JB_Number=%d JB_Count=%d=----\n",
					JB_Number, JB_Count);
//			UARTprintf((const char*)"\n----Number_of_JB=%d ----\n",JB_Number);
			UARTprintf((const char*) "----END JB Join (JBtoAM)---- \n");

			UARTprintf((const char*) "----Start PV Info (JB & AM)----\n");
			Request_PV_Info(uart);
			UARTprintf((const char*) "----End PV Info (JB & AM)----\n");

			//start JB Join (JBtoServer)
			while (task_Event != Nothing)
				;
			task_Event = JB_Join;

			UARTprintf((const char*) "----Start PV Value (JB & AM)----\n");
			Periodic_Update_time.year = AM_time.year;
			Periodic_Update_time.mon = AM_time.mon;
			Periodic_Update_time.mday = AM_time.mday;
			Periodic_Update_time.hour = AM_time.hour;
			Periodic_Update_time.min = AM_time.min;
			Periodic_Update_time.sec = AM_time.sec;
			Request_PV_Value(uart);
			UARTprintf((const char*) "----End PV Value (JB & AM)----\n");

			//start PV_Periodic(JBtoServer)
			while (task_Event != Nothing)
				;
			task_Event = PV_Periodic;

			//end_time =  Clock_getTicks(); //calculate JB Join AM Time

			//UARTprintf((const char*)"\n\n >>total execution time=%d<< \n\n",end_time-start_time);
			UARTprintf((const char*) "----Delay END----\n");
			/*if(j>Periodic_Count)
			 {
			 task_Event = TCPPeriodicLink;
			 UARTprintf((const char*)"\n\n--Update_Time: %d/%d/%d-%d:%d:%d --\n\n",AM_time.year,AM_time.mon,AM_time.mday,AM_time.hour,AM_time.min,AM_time.sec);
			 //UARTprintf((const char*)"\n\n\n\n----Debug i = %d----\n\n\n\n\n",i);
			 j=0;
			 }
			 else
			 {
			 j++;
			 //UARTprintf((const char*)"\n\n\n\n----Debug j = %d----\n\n\n\n\n",j);
			 }*/

		}
	}
}

void Create_uartHandler() {
	// Create UART task (RS-485)
	Task_Handle UART_taskHandle;
	Task_Params UART_taskParams;
	Error_Block UART_eb;
	Task_Params_init(&UART_taskParams);
	Error_init(&UART_eb);
	UART_taskParams.stackSize = 2048;
	UART_taskParams.priority = 1;
	UART_taskParams.arg0 = 1000;
	UART_taskHandle = Task_create((Task_FuncPtr) uartHandler, &UART_taskParams,
			&UART_eb);
	if (UART_taskHandle == NULL) {
		System_printf("main: Failed to create uartHandler Task\n");
	}
}

//timer
void TimeTick_Periodic(UArg arg0) {
	JB_DVAL = 1;
}

void TimeTick_TimeStamp(UArg arg0) {
	if (GPIO_read(Board_BUTTON0) == 0)
		task_Event = Environment;

	if (Time_DVAL) {
		if (AM_time.sec < 59)
			AM_time.sec++;
		else {
			AM_time.sec = 0;
			if (AM_time.min < 59)
				AM_time.min++;
			else {
				AM_time.min = 0;
				if (AM_time.hour < 23)
					AM_time.hour++;
				else {
					AM_time.hour = 0;
					if (AM_time.mon == 1 || AM_time.mon == 3 || AM_time.mon == 5
							|| AM_time.mon == 7 || AM_time.mon == 8
							|| AM_time.mon == 10 || AM_time.mon == 12)
						if (AM_time.mday < 31)
							AM_time.mday++;
						else {
							AM_time.mday = 1;
							Check_RTC_Day();
						}
					else if (AM_time.mon == 2)
						if (AM_time.mday < Check_February())
							AM_time.mday++;
						else {
							AM_time.mday = 1;
							Check_RTC_Day();
						}
					else {
						if (AM_time.mday < 30)
							AM_time.mday++;
						else {
							AM_time.mday = 1;
							Check_RTC_Day();
						}
					}
				}
			}
		}
	}
}

void Check_RTC_Day() {
	if (AM_time.mon < 12)
		AM_time.mon++;
	else {
		AM_time.mon = 1;
		AM_time.year++;
	}
}
int Check_February() {
	int day;
	if (AM_time.year % 4000 == 0)
		day = 28;
	else if (AM_time.year % 400 == 0)
		day = 29;
	else if (AM_time.year % 100 == 0)
		day = 28;
	else if (AM_time.year % 4 == 0)
		day = 29;
	else
		day = 28;
	return day;
}
int g_MckClock;
/*
 *  ======== main ========
 */
Int main(Void) {
	/*Task_Handle taskHandle;
	 Task_Params taskParams;
	 Error_Block eb;*/
	short merge = 0;
	char *temp = (char *) &merge;

	Get_EvnString();					/// got enviroment form flash 0x80000

	/* Call board init functions */
	Board_initGeneral();
	Board_initGPIO();
	Board_initEMAC();
	Board_initUART();


	g_MckClock=SysCtlClockGet();

	INIT_UART1_Printf();

	System_printf("Starting the Array Mang\nSystem provider is set to "
			"SysMin. Halt the target and use ROV to view output.\n");
	/* SysMin will only print to the console when you call flush or exit */
	System_flush();

	Log_info0("Create TCP task");


#ifndef NOETHNET
	// Create TCP task
	Task_Handle TCP_taskHandle;
	Task_Params TCP_taskParams;
	Error_Block TCP_eb;
	Task_Params_init(&TCP_taskParams);
	Error_init(&TCP_eb);
	TCP_taskParams.stackSize = 10240;					//8192;
	TCP_taskParams.priority = 2;
	TCP_taskHandle = Task_create((Task_FuncPtr) tcpHandler, &TCP_taskParams,
			&TCP_eb);
	if (TCP_taskHandle == NULL) {
		UARTprintf("main: Failed to create tcpHandler Task\n");
	}
#else

		// Create UART task (RS-485)
		Task_Handle UART_taskHandle;
		Task_Params UART_taskParams;
		Error_Block UART_eb;
		Task_Params_init(&UART_taskParams);
		Error_init(&UART_eb);
		UART_taskParams.stackSize = 2048;
		UART_taskParams.priority = 1;
		UART_taskParams.arg0 = 1000;
		UART_taskHandle = Task_create((Task_FuncPtr) NEthuartHandler, &UART_taskParams,
				&UART_eb);
		if (UART_taskHandle == NULL) {
			System_printf("main: Failed to create uartHandler Task\n");
		}

#endif



	TCP_Periodic_Link_time.Updata_Period[1] = G_ENVCONFIG.Pollingtime[0];
	TCP_Periodic_Link_time.Updata_Period[0] = G_ENVCONFIG.Pollingtime[1];
	*temp = TCP_Periodic_Link_time.Updata_Period[0];
	*(temp + 1) = TCP_Periodic_Link_time.Updata_Period[1];

	test_merge = merge;
	//UARTprintf((const char*)"\n\n\n\n----Array Pollingtime = %d----\n\n\n\n\n",merge);

	TCP_Periodic_Link_time.TCP_Link_Period[0] = 0x3c;	// TCP LINK Period
	TCP_Periodic_Link_time.TCP_Link_Period[1] = 0x00;

	//Timer-periodic
	Clock_Params clockParams;
	Error_Block eb_timer;
	Error_init(&eb_timer);
	Clock_Params_init(&clockParams);
	clockParams.period = merge * Time_Tick;
	clockParams.startFlag = TRUE;
	Periodic_Handle = Clock_create(TimeTick_Periodic, First_Periodic_time,
			&clockParams, &eb_timer);
	if (Periodic_Handle == NULL) {
		System_abort("Clock create failed");
	}

	//Timer-Time_Stamp
	Clock_Params_init(&clockParams);
	clockParams.period = TimeStamp_time;
	clockParams.startFlag = TRUE;
	AM_Timer_Handle = Clock_create(TimeTick_TimeStamp, 1, &clockParams,
			&eb_timer);
	if (AM_Timer_Handle == NULL) {
		System_abort("Clock create failed");
	}


#ifdef TelentServerPro
	Task_Handle TCP_Tlent_taskHandle;
	Task_Params TCP_Tlent_Params;
	Error_Block TCP_TlentError_eb;
	Task_Params_init(&TCP_Tlent_Params);
	Error_init(&TCP_TlentError_eb);
	TCP_Tlent_Params.stackSize = 4096;
	TCP_Tlent_Params.priority = 1;
	TCP_Tlent_taskHandle = Task_create((Task_FuncPtr)TelentServer, &TCP_Tlent_Params, &TCP_TlentError_eb);
	if (TCP_Tlent_taskHandle == NULL) {
		UARTprintf("main: Failed to create TCP_Tlent_taskHandle Task\n");
	}
#endif



	/* Start BIOS */
	BIOS_start();

	return (0);
}


Void NEthuartHandler(UArg arg0, UArg arg1) {
	int i = 0;
	int j = 0;
	//int start_time,end_time;
	int packet_num;
	JB_DVAL = 1;

	task_Event = Nothing;

	rs485_init();

	rs485_write((const uint8*) "RS-485 testing message", 22);
	UARTprintf((const char*) "Debug testing message \n");

	//Reset JB
	for (i = 0; i < 3; i++) {
		Reset_JB();
		TaskSleep(500);
	}
	for (j = 0; j < Number_of_JB; j++) {
		pv_value_table[j].Diode_Temperature = 0xFF;
		for (i = 0; i < 2; i++)
			pv_value_table[j].Voltage[i] = 0xFF;
		for (i = 0; i < 2; i++)
			pv_value_table[j].Current[i] = 0xFF;
		for (i = 0; i < 4; i++)
			pv_value_table[j].Power_Energy[i] = 0xFF;
		for (i = 0; i < 4; i++)
			pv_value_table[j].Alert_State[i] = 0xFF;
		for (i = 0; i < 6; i++)
			pv_info_table[j].MAC[i] = 0xFF;
		for (i = 6; i < 30; i++)
			pv_info_table[j].Serial_Number[i - 6] = 0xFF;
		for (i = 30; i < 54; i++)
			pv_info_table[j].Firmware_Version[i - 30] = 0xFF;
		for (i = 54; i < 78; i++)
			pv_info_table[j].Hardware_Version[i - 54] = 0xFF;
		for (i = 78; i < 102; i++)
			pv_info_table[j].Device_Specification[i - 78] = 0xFF;
		for (i = 102; i < 110; i++)
			pv_info_table[j].Manufacture_Date[i - 102] = 0xFF;
		for (i = 0; i < 6; i++)
			member_table[j].MAC[i] = 0x00;
		member_table[j].Valid = 0;
		member_table[j].state = JB_Free;
	}
	j = 0;

	while (1) {
		if (JB_DVAL == 1) {
			JB_DVAL = 0;
			//UARTprintf((const char*)"P[0] = %d\t P[1] = %d\n",G_ENVCONFIG.Pollingtime[0],G_ENVCONFIG.Pollingtime[1]);
			UARTprintf((const char*) "----Test_merge = %d----\n", test_merge);
			UARTprintf(
					(const char*) "\n\n\n--Update_Time: %d/%d/%d-%d:%d:%d --\n\n\n",
					AM_time.year, AM_time.mon, AM_time.mday, AM_time.hour,
					AM_time.min, AM_time.sec);
			UARTprintf((const char*) "----Start JB Join (JBtoAM)----\n");
			//start_time =  Clock_getTicks();  //calculate JB Join AM Time
			for (i = 0; i < 3; i++)					// Broadcast n times
					{
				//Broadcast_Packet();
				New_Broadcast_Packet_with_Pollingtime();
				packet_num = Rs4852Array(total_array);
				Array2Packet(packet_num, uart);
			}
			//end_time =  Clock_getTicks(); //calculate JB Join AM Time

			//UARTprintf((const char*)"execution time=%d \n",end_time-start_time);

			Check_JB_Number();
			UARTprintf((const char*) "\n----=JB_Number=%d JB_Count=%d=----\n",
					JB_Number, JB_Count);
//			UARTprintf((const char*)"\n----Number_of_JB=%d ----\n",JB_Number);
			UARTprintf((const char*) "----END JB Join (JBtoAM)---- \n");

			UARTprintf((const char*) "----Start PV Info (JB & AM)----\n");
			Request_PV_Info(uart);
			UARTprintf((const char*) "----End PV Info (JB & AM)----\n");

			//start JB Join (JBtoServer)
			while (task_Event != Nothing)
				;
			//task_Event = JB_Join;

			UARTprintf((const char*) "----Start PV Value (JB & AM)----\n");
			Periodic_Update_time.year = AM_time.year;
			Periodic_Update_time.mon = AM_time.mon;
			Periodic_Update_time.mday = AM_time.mday;
			Periodic_Update_time.hour = AM_time.hour;
			Periodic_Update_time.min = AM_time.min;
			Periodic_Update_time.sec = AM_time.sec;
			Request_PV_Value(uart);
			UARTprintf((const char*) "----End PV Value (JB & AM)----\n");

			//start PV_Periodic(JBtoServer)
			while (task_Event != Nothing)
				;
			//task_Event = PV_Periodic;

			//end_time =  Clock_getTicks(); //calculate JB Join AM Time

			//UARTprintf((const char*)"\n\n >>total execution time=%d<< \n\n",end_time-start_time);
			UARTprintf((const char*) "----Delay END----\n");
			/*if(j>Periodic_Count)
			 {
			 task_Event = TCPPeriodicLink;
			 UARTprintf((const char*)"\n\n--Update_Time: %d/%d/%d-%d:%d:%d --\n\n",AM_time.year,AM_time.mon,AM_time.mday,AM_time.hour,AM_time.min,AM_time.sec);
			 //UARTprintf((const char*)"\n\n\n\n----Debug i = %d----\n\n\n\n\n",i);
			 j=0;
			 }
			 else
			 {
			 j++;
			 //UARTprintf((const char*)"\n\n\n\n----Debug j = %d----\n\n\n\n\n",j);
			 }*/

		}
	}
}
