/*
 * Bus_Raw_Protocol.c
 *
 *  Created on: 2014/5/21
 *      Author: Ren
 */


/* AM_LIB UART Header files */
#include "../AM_LIB/UART_PRINTF.h"
#include "../AM_LIB/rs485.h"
#include "Bus_Raw_Protocol.h"
#include "Table.h"
 /* NDK Header files */
#include <ti/ndk/inc/netmain.h>
#include <stdlib.h>
#include <stdio.h>

uint8 packet_buffer[total_array];
Packet packet;
int Bus_ID =0;  //real Bus ID
int Bus_ID_for_PV_Value;
int Bus_ID_for_PV_Info;
int JB_Count;
int JB_Number;

extern Member_Table member_table[Number_of_JB];
extern PV_Value_Table pv_value_table[Number_of_JB];
extern PV_Info_Table pv_info_table[Number_of_JB];
extern Periodic_Data TCP_Periodic_Link_time;

int Generate_CheckSum (unsigned char *Pack, unsigned char Start_Index,unsigned char Lens)
{
	unsigned char i;
	unsigned long Check_Sum = 0;
	for (i=Start_Index; i< (Lens+Start_Index); i++)
		Check_Sum += Pack[i];
	//
	// Ignaor Saturation
	Check_Sum = (Check_Sum&0xff);

	return Check_Sum;
}

uint8 Broadcast_Packet(void)
{
	unsigned char broadcast_array[8]={0};
	uint8 checksum0 = 0;
	//checksum = (uint8)Generate_CheckSum(broadcast_array,BRD_Ctl_Index,(BRD_Ctl_Byte+BRD_BusID_Byte+BRD_Command_Byte));
	broadcast_array[0] = 0x4A;
	broadcast_array[1] = 0x00;
	broadcast_array[2] = 0x03;
	broadcast_array[3] = 0x07;
	broadcast_array[4] = 0x00;
	broadcast_array[5] = 0x05;
	broadcast_array[7] = 0x3B;
	checksum0 = Generate_CheckSum(broadcast_array,3,broadcast_array[2]);
	broadcast_array[6] = checksum0;
	rs485_write(broadcast_array,8);
	return 0;
}

uint8 New_Broadcast_Packet_with_Pollingtime(void)
{
	unsigned char broadcast_array[10]={0};
	uint8 checksum0 = 0;
	//uint16 Pollingtime= PollingTime;
	uint16 Pollingtime = 0;
	short merge=0;
	char *temp =  (char *) &merge;
	*temp = TCP_Periodic_Link_time.Updata_Period[0];
	*(temp+1) = TCP_Periodic_Link_time.Updata_Period[1];
	Pollingtime = merge*2.5;
	//UARTprintf((const char*)"\n\n\n\n---- JB_Pollingtime = %d   Merge = %d----\n\n\n\n\n",Pollingtime,merge);
	//checksum = (uint8)Generate_CheckSum(broadcast_array,BRD_Ctl_Index,(BRD_Ctl_Byte+BRD_BusID_Byte+BRD_Command_Byte));
	broadcast_array[0] = 0x4A;
	broadcast_array[1] = 0x00;
	broadcast_array[2] = 0x05;
	broadcast_array[3] = 0x07;
	broadcast_array[4] = 0x00;
	broadcast_array[5] = 0x05;
	broadcast_array[6] = (Pollingtime>>8);
	broadcast_array[7] = (Pollingtime & 0x00ff);
	//UARTprintf((const char*)"\n\n----==PollingTime = %d s==----\n\n",broadcast_array[7]);
	broadcast_array[9] = 0x3B;
	checksum0 = Generate_CheckSum(broadcast_array,3,broadcast_array[2]);
	broadcast_array[8] = checksum0;
	rs485_write(broadcast_array,10);
	return 0;
}

void Reset_JB(void)
{
	unsigned char Reset_JB_Array[8] = {0};
	uint8 checksum0 = 0;
	int i, j;
	Reset_JB_Array[0] = 0x4A;
	Reset_JB_Array[1] = 0x00;
	Reset_JB_Array[2] = 0x03;
	Reset_JB_Array[3] = 0x07; // Command Type
	Reset_JB_Array[4] = 0x00;
	Reset_JB_Array[5] = ctReset;
	Reset_JB_Array[7] = 0x3B;
	checksum0 = Generate_CheckSum(Reset_JB_Array,3,Reset_JB_Array[2]);
	Reset_JB_Array[6] = checksum0;
	rs485_write(Reset_JB_Array,8);
	Bus_ID=0;

	for(i=0;i<Number_of_JB;i++)
	{
		member_table[i].Valid=0;
		member_table[i].state=0;
		for(j=0;j<6;j++)
			member_table[i].MAC[j]=0;
	}
}

int Rs4852Array(int size)
{
	int rs485_num=0;
	rs485_num = rs485_read(packet_buffer,size);
	return rs485_num;
}

int Array2Packet(int rs485_num,UART_Handle uart)
{
		//UARTprintf((const char*)"\nI'm in Array2Packet\n");
		unsigned int j = 0;
		int packet_num=0;
	///Ren_Mod20150408[
		//uint8 *rs485_ptr = packet_buffer;
		uint8 *rs485_ptr;
		uint8 receive_array[128];
		uint8 checksum;
		//uint8 * old_ptr = rs485_ptr;
		uint8 *old_ptr;
		uint8 *process_array = (uint8*)malloc(rs485_num*sizeof(uint8));
		memcpy(process_array,packet_buffer,rs485_num);
		rs485_ptr = process_array;
		old_ptr = rs485_ptr;
	///Ren_Mod20150408]

		while( rs485_ptr-old_ptr < rs485_num)
		{

		while(*rs485_ptr!=Header_Code && rs485_ptr-old_ptr < rs485_num)
			rs485_ptr++;
		//read length

		packet_num++;
		receive_array[0] = *rs485_ptr;				//header_code
		receive_array[1] = *(rs485_ptr+1);
		receive_array[2] = *(rs485_ptr+2);				//length

		if( (receive_array[2]>120) || receive_array[0]!=Header_Code ){     //set receive length
			rs485_ptr ++; //if this condition is true, we take next ptr
			goto leave;
		}

		for(j=3;j<=receive_array[2]+4;j++){
			receive_array[j] = *(rs485_ptr+j);
		}



		// Judge packet is correct or not
		if(receive_array[receive_array[2]+4]==Tail_Code) //&& receive_array[0]==Header_Code)
			{
				checksum = Generate_CheckSum((unsigned char*)receive_array,3,receive_array[2]);
				rs485_ptr += (receive_array[2]+3);			// move ptr to check sum

				if(*(rs485_ptr)==checksum)
				{
					rs485_ptr+=2;

					switch(receive_array[5])
					{
					case ctPV_Val:	//Response PV Value
						{
							Response_PV_Value(receive_array);
							break;
						}
					case ctPV_Info: //Response PV Info
						{
							Response_PV_Info(receive_array);
							//UARTprintf((const char*)"PV_Info_OK!\n");
							break;
						}
					case ctJOIN_Req:	//Join Request
						{
							//UARTprintf((const char*)"\nI'm in case6\n");
							int rs485_num=0;
							Assign_BusID(receive_array);
							rs485_num = Rs4852Array(14);
							//UARTprintf((const char*)"\nI'm out case6\n");
							if(rs485_num==14)
							{
								//UARTprintf((const char*)"\nI'm calling array again\n");
								Array2Packet(rs485_num,uart);
								//UARTprintf((const char*)"\nI'm out calling array again\n");
							}
							else
							{
								UARTprintf((const char*)"Join_Request(Assign BUS ID)Command Type:0x06 Response ERROR\n");
							}
							break;
						}
						case ctAsign_Ack:	//ACK Response
						{
							//UARTprintf((const char*)"\nI'm in case8\n");
							ACKResponse(receive_array,uart);
							//UARTprintf((const char*)"\nI'm out case8\n");
							break;
						}
						default : break;
					}
				}
				else
				{
					rs485_ptr+=2;
				}
			}
		else
			{
				rs485_ptr ++;

			}
leave:
		;
		}
		//UARTprintf((const char*)"\nI'm out Array2Packet\n");
	///Ren_Mod20150408[
		if( process_array )
			free( process_array );
	///Ren_Mod20150408]
		return 0;
}


int Assign_BusID(uint8* Buffer_Array)
{
	///Ren_Mod20150408[
	//uint8 Send_Array[14];
	uint8 Send_Array[15];
	//uint8 *rs485_ptr = Send_Array;
	///Ren_Mod20150408]
	uint8 *buffer_ptr = Buffer_Array;
	int i;
	int ID;
	ID=Check_Mac(Buffer_Array); //return assign Bus_ID

	///Ren_Mod20150408[
		/**rs485_ptr = 0x4A;
		rs485_ptr++;
		*rs485_ptr = 0x00;
		rs485_ptr++;
		*rs485_ptr = 0x0A;
		rs485_ptr++;
		*rs485_ptr = 0x03;
		rs485_ptr++;
		*rs485_ptr = ID;  //Bus_ID;
		rs485_ptr++;
		*rs485_ptr = 0x07;   //Command Type(Assign BUS ID)
		rs485_ptr++;
		*rs485_ptr = 0x00;
		rs485_ptr++;
		buffer_ptr+=6; //Read Payload
		for(i=0;i<6;i++)
		{
			*rs485_ptr = *buffer_ptr;
			//UARTprintf((const char*)"%x ",*rs485_ptr);
			rs485_ptr++;
			buffer_ptr++;
		}
		//UARTprintf((const char*)"\n");
		*rs485_ptr = Generate_CheckSum((unsigned char*)Send_Array,3,10);
		rs485_ptr++;
		*rs485_ptr = 0x3B;
		rs485_ptr++;*/
	///Ren_Mod20150408]

	Send_Array[0] = 0x4A;
	Send_Array[1] = 0x00;
	Send_Array[2] = 0x0A;
	Send_Array[3] = 0x03;
	Send_Array[4] = ID;  //Bus_ID;
	Send_Array[5] = ctAssignID;   //Command Type(Assign BUS ID)
	Send_Array[6] = 0x00;
	buffer_ptr+=6; //Read Payload
	for(i=0;i<6;i++)
	{
		Send_Array[i+7] = *buffer_ptr;
		//UARTprintf((const char*)"%x ",*rs485_ptr);
		buffer_ptr++;
	}
	//UARTprintf((const char*)"\n");
	Send_Array[13] = Generate_CheckSum((unsigned char*)Send_Array,3,10);
	Send_Array[14] = 0x3B;
	rs485_write(Send_Array,15);
	return 0;
}


int ACKResponse (uint8* Buffer_Array , UART_Handle uart)
{
	uint8 ACK_buffer[8];

	ACK_buffer[0] = 0x4A;
	ACK_buffer[1] = 0x00;
	ACK_buffer[2] = 0x03;
	ACK_buffer[3] = 0x02;
	ACK_buffer[4] = Buffer_Array[4];//Bus_ID;
	ACK_buffer[5] = ctACK_Resp;
	ACK_buffer[6] = Generate_CheckSum(ACK_buffer,3,ACK_buffer[2]);
	ACK_buffer[7] = 0x3B;
	rs485_write(ACK_buffer,8);
	member_table[Buffer_Array[4]].MAC[0] = Buffer_Array[6];
	member_table[Buffer_Array[4]].MAC[1] = Buffer_Array[7];
	member_table[Buffer_Array[4]].MAC[2] = Buffer_Array[8];
	member_table[Buffer_Array[4]].MAC[3] = Buffer_Array[9];
	member_table[Buffer_Array[4]].MAC[4] = Buffer_Array[10];
	member_table[Buffer_Array[4]].MAC[5] = Buffer_Array[11];
	member_table[Buffer_Array[4]].Valid = SJBStartCycle;
	member_table[Buffer_Array[4]].state = JB_Join2AM;
	TaskSleep(500);
	return 0;
}

int Request_PV_Value(UART_Handle uart)
{
	uint8 Request_PV_Value_Buffer[8];
	int num=0;
	int i;
	Request_PV_Value_Buffer[0] = 0x4A;
	Request_PV_Value_Buffer[1] = 0x00;
	Request_PV_Value_Buffer[2] = 0x03;
	Request_PV_Value_Buffer[3] = 0x02;
	Request_PV_Value_Buffer[5] = 0x03;
	Request_PV_Value_Buffer[7] = 0x3B;
	for(Bus_ID_for_PV_Value=0;Bus_ID_for_PV_Value<JB_Count;Bus_ID_for_PV_Value++)
	{
		if(member_table[Bus_ID_for_PV_Value].Valid== SJBStartCycle)
		{
			pv_value_table[Bus_ID_for_PV_Value].Diode_Temperature = 0x00;
			for(i=0;i<4;i++)
			{
				pv_value_table[Bus_ID_for_PV_Value].Alert_State[i] = 0x00;
				pv_value_table[Bus_ID_for_PV_Value].Power_Energy[i] = 0x00;
			}
			for(i=0;i<2;i++)
			{
				pv_value_table[Bus_ID_for_PV_Value].Voltage[i] = 0x00;
				pv_value_table[Bus_ID_for_PV_Value].Current[i] = 0x00;
			}

			member_table[Bus_ID_for_PV_Value].Valid = Cycle;

			continue;

		}
		else if(member_table[Bus_ID_for_PV_Value].Valid>0)
		{
			Request_PV_Value_Buffer[4] = Bus_ID_for_PV_Value;
			Request_PV_Value_Buffer[6] = Generate_CheckSum(Request_PV_Value_Buffer,3,Request_PV_Value_Buffer[2]);
			rs485_write(Request_PV_Value_Buffer,8);
			num = Rs4852Array(21);
			if(num==21)
			{
				Array2Packet(num,uart);
				member_table[Bus_ID_for_PV_Value].Valid = Cycle;
			}
			else
			{
				UARTprintf((const char*)"Bus_ID %d Incomplete packets for Request_PV_Value \n",Bus_ID_for_PV_Value);
				member_table[Bus_ID_for_PV_Value].Valid =  member_table[Bus_ID_for_PV_Value].Valid - 1;
			}
		}
		else
		{
				//member_table[Bus_ID_for_PV_Value].state = 0;
				pv_value_table[Bus_ID_for_PV_Value].Diode_Temperature = 0x00;
				for(i=0;i<4;i++)
				{
					pv_value_table[Bus_ID_for_PV_Value].Alert_State[i] = 0x00;
					pv_value_table[Bus_ID_for_PV_Value].Power_Energy[i] = 0x00;
				}
				for(i=0;i<2;i++)
				{
					pv_value_table[Bus_ID_for_PV_Value].Voltage[i] = 0x00;
					pv_value_table[Bus_ID_for_PV_Value].Current[i] = 0x00;
				}
				pv_value_table[Bus_ID_for_PV_Value].Alert_State[0] = JB_Offline; // JB_Offline
		}
	}
	return 0;
}

int Response_PV_Value(uint8* Buffer_Array)
{
	int i;  /////define value over 150  -1 -2 .........

		pv_value_table[Bus_ID_for_PV_Value].Diode_Temperature = Buffer_Array[6];
	for(i=0;i<2;i++)
		pv_value_table[Bus_ID_for_PV_Value].Voltage[i] = Buffer_Array[7+i];
	for(i=0;i<2;i++)
		pv_value_table[Bus_ID_for_PV_Value].Current[i] = Buffer_Array[9+i];
	for(i=0;i<4;i++)
		pv_value_table[Bus_ID_for_PV_Value].Power_Energy[i] = Buffer_Array[11+i];
	for(i=0;i<4;i++)
		pv_value_table[Bus_ID_for_PV_Value].Alert_State[i] = Buffer_Array[15+i];
	return 0;
}

int Request_PV_Info(UART_Handle uart){

	uint8 Request_PV_Info_Buffer[8];
	int num=0;
	int error;
	//int i;

	Request_PV_Info_Buffer[0] = 0x4A;
	Request_PV_Info_Buffer[1] = 0x00;
	Request_PV_Info_Buffer[2] = 0x03;
	Request_PV_Info_Buffer[3] = 0x02;
	Request_PV_Info_Buffer[5] = 0x04;
	Request_PV_Info_Buffer[7] = 0x3B;

	for(Bus_ID_for_PV_Info=0;Bus_ID_for_PV_Info<JB_Count;Bus_ID_for_PV_Info++)
	{
		if(member_table[Bus_ID_for_PV_Value].Valid>0)
		{
			Request_PV_Info_Buffer[4] = Bus_ID_for_PV_Info;
			Request_PV_Info_Buffer[6] = Generate_CheckSum(Request_PV_Info_Buffer,3,Request_PV_Info_Buffer[2]);
			/*rs485_write(Request_PV_Info_Buffer,8);
			num = Rs4852Array(118);*/
			error=0;
			do{
				rs485_write(Request_PV_Info_Buffer,8);
				num = Rs4852Array(118);
				if(num==118)
				{
					Array2Packet(num,uart);
					error++;
					if(pv_info_table[Bus_ID_for_PV_Info].Serial_Number[0]!=0xFF)
						break;
					else
						UARTprintf((const char*)">>>>>Bus_ID %d Incomplete packets for Request_PV_Info <<<< \n",Bus_ID_for_PV_Info);
				}
				else
				{
					error++;
					UARTprintf((const char*)">>>>>Bus_ID %d Incomplete packets for Request_PV_Info<<<<< \n",Bus_ID_for_PV_Info);
					/*for(i=0;i<6;i++)
						pv_info_table[Bus_ID_for_PV_Info].MAC[i] = 0xFF;
					for(i=6;i<30;i++)
						pv_info_table[Bus_ID_for_PV_Info].Serial_Number[i-6] = 0xFF;
					for(i=30;i<54;i++)
						pv_info_table[Bus_ID_for_PV_Info].Firmware_Version[i-30] = 0xFF;
					for(i=54;i<78;i++)
						pv_info_table[Bus_ID_for_PV_Info].Hardware_Version[i-54] = 0xFF;
					for(i=78;i<102;i++)
						pv_info_table[Bus_ID_for_PV_Info].Device_Specification[i-78] = 0xFF;
					for(i=102;i<110;i++)
						pv_info_table[Bus_ID_for_PV_Info].Manufacture_Date[i-102] = 0xFF;*/
					/*rs485_write(Request_PV_Info_Buffer,8);
					num = Rs4852Array(118);*/
				}
			}while(error<3);
		}
	}
	return 0;
}

void Response_PV_Info(uint8* Buffer_Array)
{
	int i;
	for(i=0;i<6;i++)
		pv_info_table[Bus_ID_for_PV_Info].MAC[i] = Buffer_Array[6+i];
	for(i=6;i<30;i++)
		pv_info_table[Bus_ID_for_PV_Info].Serial_Number[i-6] = Buffer_Array[6+i];
	for(i=30;i<54;i++)
		pv_info_table[Bus_ID_for_PV_Info].Firmware_Version[i-30] = Buffer_Array[6+i];
	for(i=54;i<78;i++)
		pv_info_table[Bus_ID_for_PV_Info].Hardware_Version[i-54] = Buffer_Array[6+i];
	for(i=78;i<102;i++)
		pv_info_table[Bus_ID_for_PV_Info].Device_Specification[i-78] = Buffer_Array[6+i];
	for(i=102;i<110;i++)
		pv_info_table[Bus_ID_for_PV_Info].Manufacture_Date[i-102] = Buffer_Array[6+i];
}

int Check_Mac(uint8* Buffer_Array)
{
	int Index=0;
	int New_Index=0;
	//check Mac & ID
	while(member_table[Index].MAC[5]!=0)
	{
		if(member_table[Index].MAC[5]==Buffer_Array[11])
		{
			if(member_table[Index].MAC[4]==Buffer_Array[10])
			{
				if(member_table[Index].MAC[3]==Buffer_Array[9])
				{
					if(member_table[Index].MAC[2]==Buffer_Array[8])
					{
						if(member_table[Index].MAC[1]==Buffer_Array[7])
						{
							if(member_table[Index].MAC[0]==Buffer_Array[6])
							{
								return Index;
							}
							else
								Index++;
						}
						else
							Index++;
					}
					else
						Index++;
				}
				else
					Index++;
			}
			else
				Index++;
		}
		else
			Index++;
	}
	//check Vailid    //// if we got the Mac form SJB is consistance with the mak in the member table , reuse it
					  //// if new Mac from SJB is totaly new we put the new SJB from 0 in member table
	if(member_table[Index].MAC[5]==Buffer_Array[11] && member_table[Index].MAC[4]==Buffer_Array[10] && member_table[Index].MAC[3]==Buffer_Array[9]
      && member_table[Index].MAC[2]==Buffer_Array[8] && member_table[Index].MAC[1]==Buffer_Array[7] && member_table[Index].MAC[0]==Buffer_Array[6])
	{
		Bus_ID = Index;
		return Index;//assign Bus ID
	}
	else
	{
		while(member_table[New_Index].Valid>0x00)
		{
			New_Index++;
		}
		Bus_ID = Index;
		return New_Index;//assign Bus ID
	}
	/*
	Bus_ID = Index;
	return Index;//assign Bus ID*/
}

void Check_JB_Number()
{
	int i;
	JB_Count = 0;
	JB_Number = 0;
	for(i=0;i<Number_of_JB;i++)
	{
		if(member_table[i].Valid>0)
			JB_Number++;
		if(member_table[i].MAC[0]!=0 || member_table[i].MAC[1]!=0 || member_table[i].MAC[2]!=0 || member_table[i].MAC[3]!=0
				|| member_table[i].MAC[4]!=0 || member_table[i].MAC[5]!=0)
			JB_Count++;
		if(JB_Count>Number_of_JB) JB_Count=Number_of_JB;
	}

}

/*void RESET_MCU (unsigned int DelayTime, unsigned char SpecialOperation)
{
	// Assert Chip Reset after Delay
	UARTprintf("\n ::: Warning After %d mSec  MCU will Reset :::\n", DelayTime);
	//vTaskDelay ( (portTickType) DelayTime);

	//
	// Dumping Data for Debugging

	//
	// Reset MCU
	SysCtlReset();
}*/
