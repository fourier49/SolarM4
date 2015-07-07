/*
 * Table.h
 *
 *  Created on: 2015/1/14
 *      Author: Eric
 */
#include "../Board.h"
#include <ti/sysbios/knl/Clock.h>

#ifndef TABLE_H_
#define TABLE_H_
//Table
#define Number_of_JB 256
//(sec)
#define PollingTime 3*60 //1 min
//Timer (sec)
#define Time_Tick 1000
#define Periodic_time 30*Time_Tick
#define First_Periodic_time 90*Time_Tick
#define TimeStamp_time 1*Time_Tick
#define Cycle 0x02;
#define SJBStartCycle 0xAA


/////////////////sjb status
#define JB_Offline	1
#define JB_Online   0
#define JB_Free		0
#define JB_Join2AM  1
#define JB_Join2Server 2
Clock_Handle AM_Timer_Handle;
Clock_Handle Periodic_Handle;

/////////////AM FW version

#define AM_FWversion	"AM_FW_0.01b"

////////////////////////////////////////////
typedef struct Time_Stamp_Arrray
{
	char sec,min,hour,mday,mon,year;
}Array_Time;

typedef struct Member_Table_JB
{
	char MAC[6];
	int Valid;
	int state;
}Member_Table;

typedef struct PV_Value_Table_JB
{
	char Diode_Temperature;
	char Voltage[2];
	char Current[2];
	char Power_Energy[4];
	char Alert_State[4];
}PV_Value_Table;

typedef struct PV_Info_Table_JB
{
	char MAC[6];
	char Serial_Number[24];
	char Firmware_Version[24];
	char Hardware_Version[24];
	char Device_Specification[24];
	char Manufacture_Date[8];
}PV_Info_Table;

typedef struct Periodic_Data_Transmission
{
	char Updata_Period[2];
	char TCP_Link_Period[2];
}Periodic_Data;

void Check_RTC_Day();
int Check_February();


/////////////////////////////////////////TASK

Void TelentServer(UArg arg0, UArg arg1);

//////////////////////PROJECT OPTION
#define TelentServerPro
//#define NOETHNET



/////////////////////////////////////



#endif /* TABLE_H_ */
