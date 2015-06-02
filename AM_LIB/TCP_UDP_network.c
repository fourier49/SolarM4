/*
 * TCP_UDP_network.c
 *
 *  Created on: 2014/3/24
 *      Author: SHIN
 */
/*
 * Author: didi Lin
 * Created on: 2014/8/26
 * Description: network reset, TCP_transmission doesn't work.
 *
 *
 *
 */

/* AM_LIB UART Header files */
#include "../AM_LIB/UART_PRINTF.h"
#include "../Bus_Raw_Protocol/Table.h"
#include "../Debug/configPkg/package/cfg/AM_model_pem4f.h"

#include "TCP_UDP_network.h"
#include <xdc/std.h>

#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>
#include <driverlib/flash.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static SOCKET server_sock;
static struct sockaddr_in server_sin;
static IPN server_IPAddr;
static struct timeval server_timeout;

static SOCKET server_sock2;
static struct sockaddr_in server_sin2;
static IPN server_IPAddr2;
static struct timeval server_timeout2;

extern Array_Time AM_time;
extern Array_Time Periodic_Update_time;
extern Periodic_Data TCP_Periodic_Link_time;
extern Member_Table member_table[Number_of_JB];
extern PV_Value_Table pv_value_table[Number_of_JB];
extern PV_Info_Table pv_info_table[Number_of_JB];
extern ENVCONFIG G_ENVCONFIG;
char server_ip[50]={0};
//print IP
IPN plPAddr;
char pStrBuffer[16] = {0};
char *pStrBuffer2 = NULL;
char FW_Rev_SegmentArray[RevFwSegment+32];
int network_initial()
{
	server_sock = INVALID_SOCKET;
	server_IPAddr = inet_addr(server_ip);

	// Prepare address for connect
	bzero( &server_sin, sizeof(struct sockaddr_in) );
	server_sin.sin_family = AF_INET;
	server_sin.sin_len = sizeof( server_sin );
	server_sin.sin_addr.s_addr = server_IPAddr;
	server_sin.sin_port = htons(server_port);

	// Configure our Tx and Rx timeout to be 5 seconds
	server_timeout.tv_sec = 5;
	server_timeout.tv_usec = 0;

	return 0;
}
/*int TCP_Recv(int socketID)
{
	;
}*/
int TCP_transmission(char *pbuf, int size)
{
	// Allocate the file descriptor environment for this Task
	//fdOpenSession( (HANDLE)Task_self() );

	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_sock < 0) {
	        return -1;
	    }

	setsockopt( server_sock, SOL_SOCKET, SO_SNDTIMEO, &server_timeout, sizeof( server_timeout ) );
	setsockopt( server_sock, SOL_SOCKET, SO_RCVTIMEO, &server_timeout, sizeof( server_timeout ) );

	// Connect socket
	while( connect( server_sock, (PSA) &server_sin, sizeof(server_sin) ) < 0 )
		Task_sleep(500);


	send( server_sock, pbuf, size, 0);

	fdClose(server_sock);
	//fdCloseSession( (HANDLE)Task_self() );

	return 0;
}
void EchoTcp()
{
	unsigned char *pBuf = 0 ;
	server_sock = INVALID_SOCKET;
	int I;
	int test = 11;

	server_IPAddr = inet_addr(server_ip);

	// Allocate the file descriptor environment for this Task
	//fdOpenSession( (HANDLE)Task_self() );
	//UARTprintf((const char*)"\n== Start TCP Echo Client Test ==\n");
	// Create test socket
	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( server_sock == INVALID_SOCKET )
	{
		UARTprintf((const char*)"failed socket create (%d)\n",fdError());
		goto leave;
	}

	// Prepare address for connect
		bzero( &server_sin, sizeof(struct sockaddr_in) );
		server_sin.sin_family = AF_INET;
		server_sin.sin_len = sizeof( server_sin );
		server_sin.sin_addr.s_addr = server_IPAddr;
		server_sin.sin_port = htons(server_port);

	// Configure our Tx and Rx timeout to be 2 seconds
		server_timeout.tv_sec = 5;
		server_timeout.tv_usec = 0;
		setsockopt( server_sock, SOL_SOCKET, SO_SNDTIMEO, &server_timeout, sizeof( server_timeout ) );
		setsockopt( server_sock, SOL_SOCKET, SO_RCVTIMEO, &server_timeout, sizeof( server_timeout ) );

	// Connect socket
		if( connect( server_sock, (PSA) &server_sin, sizeof(server_sin) ) < 0 )
		{
			UARTprintf((const char*)"failed connect (%d)\n",fdError());
			goto leave;
		}
	// Allocate a working buffer
		if( !(pBuf = malloc( 1025 )) )
		{
			UARTprintf((const char*)"failed temp buffer allocation\n");
			goto leave;
		}
	// Fill buffer with a test pattern
		for(I=0; I<1025; I++)
			*(pBuf+I) = (unsigned char)I;
	// Send the buffer
	if( send( server_sock, pBuf, 1025, 0 ) < 0 )
		{
			UARTprintf((const char*)"send failed (%d)\n",fdError());
			goto leave;
		}
	// Try and receive the test pattern back
	I = recv( server_sock, pBuf, 4096, MSG_WAITALL );
	if( I < 0 )
	{
		UARTprintf((const char*)"recv failed (%d)\n",fdError());
		goto leave;
	}
	// Verify reception size and pattern
	if( I != test )
	{
		UARTprintf((const char*)"received %d (not %d) bytes\n",I, test);
		goto leave;
	}
	else
	{
		pBuf[I] = '\0';
		UARTprintf((const char*)"recv: %s\n",pBuf);
	}

	/*for(I=0; I<test; I++)
	if( *(pBuf+I) != (char)I )
	{
		printf("verify failed at byte %d\n",I);
		break;
	}*/
	// If here, the test passed
	if( I==test )
		UARTprintf((const char*)"passed\n");
leave:
	if( pBuf )
		free( pBuf );
	if( server_sock != INVALID_SOCKET )
		fdClose( server_sock );
	UARTprintf((const char*)"== End TCP Echo Client Test ==\n\n");
	//Free the file descriptor environment for this Task
	//fdCloseSession( (HANDLE)Task_self() );
	//Task_exit();


}


//*  Created on: 2014/9/2
// *      Author: didi
int AM_JoinRequest()
{
	unsigned char *Join_Array = 0;
	unsigned char *Recv_Array = 0;
	int recv_num = 0;
	char Join_Result;
	server_sock = INVALID_SOCKET;

	//take IP from environment
	sprintf(server_ip,"%d.%d.%d.%d",G_ENVCONFIG.IP[0],G_ENVCONFIG.IP[1],G_ENVCONFIG.IP[2],G_ENVCONFIG.IP[3]);
	server_IPAddr = inet_addr(server_ip);

	UARTprintf((const char*)"\n Server IP= %s \n",server_ip);
	NtIfIdx2Ip(1, &plPAddr);
	NtIPN2Str( plPAddr, pStrBuffer);
	UARTprintf((const char*)"\n AM_IP= %s \n",pStrBuffer);

	// Allocate the file descriptor environment for this Task
	//fdOpenSession( (HANDLE)Task_self() );
	UARTprintf((const char*)"\n== Start AM Join Request ==\n");

	// Create test socket
	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);



//	System_printf("server_sock=%d \n",server_sock);
//	System_flush();


	if( server_sock == INVALID_SOCKET )
	{
		UARTprintf((const char*)"failed socket create (%d)\n",fdError());
		goto leave;
	}

	// Prepare address for connect
	bzero( &server_sin, sizeof(struct sockaddr_in) );
	server_sin.sin_family = AF_INET;
	server_sin.sin_len = sizeof( server_sin );
	server_sin.sin_addr.s_addr = server_IPAddr;
	server_sin.sin_port = htons(server_port);

	// Configure our Tx and Rx timeout to be 2 seconds
	server_timeout.tv_sec = 5;
	server_timeout.tv_usec = 0;
	setsockopt( server_sock, SOL_SOCKET, SO_SNDTIMEO, &server_timeout, sizeof( server_timeout ) );
	setsockopt( server_sock, SOL_SOCKET, SO_RCVTIMEO, &server_timeout, sizeof( server_timeout ) );

	// Connect socket
	if( connect( server_sock, (PSA) &server_sin, sizeof(server_sin) ) < 0 )
	{
		UARTprintf((const char*)"failed connect (%d)\n",fdError());///error code ENOBUFS in serron.h
		goto leave;
	}

	NtIfIdx2Ip(1, &plPAddr);
	NtIPN2Str( plPAddr, pStrBuffer);

	//UARTprintf((const char*)"\n IP= %s \n",pStrBuffer);
	//UARTprintf((const char*)"\n tt= %d \n",tt);

	// Allocate a working buffer
	if( !(Join_Array = malloc( AMJoinPKLength )) )
	{
		UARTprintf((const char*)"failed Join_Array buffer allocation\n");
		goto leave;
	}
	if( !(Recv_Array = malloc( AMJoinResponsePKLength )) )
	{
		UARTprintf((const char*)"failed Recv_Array buffer allocation\n");
		goto leave;
	}

	// generate AM Join Package
	Join_Array[0] = AMJoinHeader;
	Join_Array[1] = 217;
	Join_Array[2] = 0x00;
	Join_Array[3] = rand()%256;
	Join_Array[4] = AMJoinCommand;
	Join_Array[5] = 0x01;
	GenerateAMJoinCommandPK(Join_Array);
	Join_Array[AMJoinPKLength-1] = CommandPKTailCode;

	// Send Join Package
	if( send( server_sock, Join_Array, AMJoinPKLength, 0 ) < 0 )
		{
			UARTprintf((const char*)"send failed (%d)\n",fdError());
			goto leave;
		}

	// receive Server's Join Response
	recv_num = recv( server_sock, Recv_Array, AMJoinResponsePKLength, MSG_WAITALL );
	if( recv_num < 0 )
	{
		UARTprintf((const char*)"recv failed (%d)\n",fdError());
		goto leave;
	}
	else
	{
		UARTprintf((const char*)"Recv: %d bytes \n",recv_num);
	}

leave:
	switch(Recv_Array[AMJoinResultPKPos])
	{	case Join_Success:
			 Join_Result = Join_Success;
		break;

		case Join_Deny:
			 Join_Result = Join_Deny;
		break;

		case Join_Failed:
			 Join_Result = Join_Failed;
		break;
		default: Join_Result = Join_Failed;
	}

	// Free Pointer & Socket
	if( Join_Array )
		free( Join_Array );
	if( Recv_Array )
		free( Recv_Array );
	if( server_sock != INVALID_SOCKET )
	{
		UARTprintf((const char*)"-=Close AM Socket=- \n");
		fdClose( server_sock );
	}
	UARTprintf((const char*)"== End AM Join Request ==\n\n");
	//Free the file descriptor environment for this Task
	//fdCloseSession( (HANDLE)Task_self() );
	return Join_Result;
}


//*  Created on: 2014/9/4 	Continuous modification
// *      Author: Ren

int JB_JoinRequest(int Table_Index)
{
	unsigned char *Join_Array = 0;
	unsigned char *Recv_Array = 0;
	int recv_num = 0;
	char Join_Result;

	server_sock = INVALID_SOCKET;

	server_IPAddr = inet_addr(server_ip);

	// Allocate the file descriptor environment for this Task
	//fdOpenSession( (HANDLE)Task_self() );
	UARTprintf((const char*)"\n== Start JB Join Request ==\n");
	// Create test socket
	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( server_sock == INVALID_SOCKET )
	{
		UARTprintf((const char*)"failed socket create (%d)\n",fdError());
		Join_Result = Join_Failed;
		goto leave;
	}
	// Prepare address for connect
	bzero( &server_sin, sizeof(struct sockaddr_in) );
	server_sin.sin_family = AF_INET;
	server_sin.sin_len = sizeof( server_sin );
	server_sin.sin_addr.s_addr = server_IPAddr;
	server_sin.sin_port = htons(server_port);

	// Configure our Tx and Rx timeout to be 2 seconds
	server_timeout.tv_sec = 5;
	server_timeout.tv_usec = 0;
	setsockopt( server_sock, SOL_SOCKET, SO_SNDTIMEO, &server_timeout, sizeof( server_timeout ) );
	setsockopt( server_sock, SOL_SOCKET, SO_RCVTIMEO, &server_timeout, sizeof( server_timeout ) );

	// Allocate a working buffer
	if( !(Join_Array = malloc( JBJoinPKLength )) )
	{
		UARTprintf((const char*)"failed Join_Array buffer allocation\n");
		Join_Result = Join_Failed;
		goto leave;
	}

	if( !(Recv_Array = malloc( JBJoinResponsePKLength )) )
	{
		UARTprintf((const char*)"failed Recv_Array buffer allocation\n");
		Join_Result = Join_Failed;
		goto leave;
	}

	// generate JB Join Package
	Join_Array[0] = AMJoinHeader;
	Join_Array[1] = 120;
	Join_Array[2] = 0x00;
	Join_Array[3] = rand()%256;
	Join_Array[4] = JBJoinCommand;
	Join_Array[5] = 0x01;
	GenerateJBJoinCommandPK(Join_Array,Table_Index);
	Join_Array[JBJoinPKLength-1] = CommandPKTailCode;

	// Connect socket
	if( connect( server_sock, (PSA) &server_sin, sizeof(server_sin) ) < 0 )
	{
		UARTprintf((const char*)"failed connect (%d)\n",fdError());
		Join_Result = Join_Failed;
		goto leave;
	}

	// Send Join Package
	//UARTprintf((const char*)"\n SendBTCP_Mac[%d] %x %x %x %x %x %x\n\n",Table_Index,Join_Array[18],Join_Array[17],Join_Array[16],Join_Array[15],Join_Array[14],Join_Array[13]);
	if( send( server_sock, Join_Array, JBJoinPKLength, 0 ) < 0 )
	{
		UARTprintf((const char*)"send failed (%d)\n",fdError());
		Join_Result = Join_Failed;
		goto leave;
	}
	//UARTprintf((const char*)"\n SendATCP_Mac[%d] %x %x %x %x %x %x\n\n",Table_Index,Join_Array[18],Join_Array[17],Join_Array[16],Join_Array[15],Join_Array[14],Join_Array[13]);

	// receive Server's Join Response
	recv_num = recv( server_sock, Recv_Array, JBJoinResponsePKLength, MSG_WAITALL );
	if( recv_num < 0 )
	{
		UARTprintf((const char*)"recv failed (%d)\n",fdError());
		Join_Result = Join_Failed;
		goto leave;
	}
	else
	{
		UARTprintf((const char*)"Recv: %d bytes \n",recv_num);
	}


	switch(Recv_Array[JBJoinResultPKPos])
	{	case Join_Success:
			 Join_Result = Join_Success;
		break;

		default: Join_Result = Join_Failed;
	}
leave:
	// Free Pointer & Socket
	if( Join_Array )
	{
		free( Join_Array );
	}
	if( Recv_Array )
	{
		free( Recv_Array );
	}
	if( server_sock != INVALID_SOCKET )
	{
		UARTprintf((const char*)"-=Close JB_Join Socket=- \n");
		fdClose( server_sock );
	}
	UARTprintf((const char*)"== End JB Join Request ==\n\n");
	//Free the file descriptor environment for this Task
	//fdCloseSession( (HANDLE)Task_self() );
	return Join_Result;
}

//*  Created on: 2014/12/8
// *      Author: dong
int PV_Periodic_Trans(int Head,int Tail)
{
	unsigned char *Periodic_Array = 0;
	unsigned char *Recv_Array = 0;
	int recv_num = 0;
	int try = 0;
	//char Periodic_Result;
	int Head_Num;
	server_sock2 = INVALID_SOCKET;

	server_IPAddr2 = inet_addr(server_ip);

	// Allocate the file descriptor environment for this Task
	//fdOpenSession( (HANDLE)Task_self() );


	// Create test socket
	server_sock2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	do{
		if( server_sock2 == INVALID_SOCKET )
		{
			UARTprintf((const char*)"failed socket create (%d)\n",fdError());
			try++;
			//goto leave;
		}
		else
		{
			try = 0;
			break;
		}
	}while(try<3);
	if(try==3)
		goto leave;
	// Prepare address for connect
	bzero( &server_sin2, sizeof(struct sockaddr_in) );
	server_sin2.sin_family = AF_INET;
	server_sin2.sin_len = sizeof( server_sin2 );
	server_sin2.sin_addr.s_addr = server_IPAddr2;
	server_sin2.sin_port = htons(4900);

	// Configure our Tx and Rx timeout to be 2 seconds
	server_timeout2.tv_sec = 5;
	server_timeout2.tv_usec = 0;
	setsockopt( server_sock2, SOL_SOCKET, SO_SNDTIMEO, &server_timeout2, sizeof( server_timeout2 ) );
	setsockopt( server_sock2, SOL_SOCKET, SO_RCVTIMEO, &server_timeout2, sizeof( server_timeout2 ) );

	// Allocate a working buffer
	if( !(Periodic_Array = calloc( PVPeriodicPKLength,1 )) )
	{
		UARTprintf((const char*)"failed Periodic_Array buffer allocation\n");
		goto leave;
	}
	if( !(Recv_Array = malloc( PVPeriodicResponsePKLength )) )
	{
		UARTprintf((const char*)"failed Recv_Array buffer allocation\n");
		goto leave;
	}
	// generate Periodic Data Transmission Package
	Periodic_Array[0] = AMPVHeader;
	Periodic_Array[1] = 0xE5;	// 1~2 = 485
	Periodic_Array[2] = 0x01;	// Package Length
	Periodic_Array[3] = Head/25;		// only one sequence
	Periodic_Array[4] = PVPeriodicCommand;
	Periodic_Array[5] = 0x01;
	Head_Num = GeneratePVPeriodicCommandPK(Periodic_Array,Head,Tail);
	Periodic_Array[PVPeriodicPKLength-1] = PVPeriodicCommandPKTailCode;

	UARTprintf((const char*)"\n==  PV Segment=%d Transmission ==\n",Head_Num);

	if(Periodic_Array[12]==0x00)
		goto leave;

	// Connect socket
	if( connect( server_sock2, (PSA) &server_sin2, sizeof(server_sin2) ) < 0 )
	{
		UARTprintf((const char*)"failed connect (%d)\n",fdError());
		goto leave;
	}
	do{

		// Send PVPeriodic Package
		if( send( server_sock2, Periodic_Array, PVPeriodicPKLength, 0 ) < 0 )
			{
				UARTprintf((const char*)"send failed (%d)\n",fdError());
				goto leave;
			}
	//	UARTprintf((const char*)"== Fin PV Periodic Data Transmission ==\n\n");
		// receive Server's PeriodicPK Response
		recv_num = recv( server_sock2, Recv_Array, PVPeriodicResponsePKLength, MSG_WAITALL );
		if( recv_num < 0 )
		{
			UARTprintf((const char*)"recv failed (%d)\n",fdError());
			//goto leave;
		}
		else
		{
			//UARTprintf((const char*)"Recv: %d bytes \n",recv_num);
			if (Recv_Array[0] == 'O' && Recv_Array[1] == 'K')
			{
				//Periodic_Result = PVData_Success;
				//UARTprintf((const char*)"== Fin PV Periodic Data Transmission ==\n\n");
				break;
			}
		}

		try++;
	}while(try<5);

	leave:

	if(try>=5)
	{
		//Periodic_Result = PVData_NoResponse;
		UARTprintf((const char*)"== Periodic Data Transmission Fail !!!! ==\n\n");
	}
	// Free Pointer & Socket
	if( Periodic_Array )
		free( Periodic_Array );
	if( Recv_Array )
		free( Recv_Array );
	if( server_sock2 != INVALID_SOCKET )
	{
		//UARTprintf((const char*)"-=Close Periodic Socket=- \n");
		fdClose( server_sock2 );
	}
	//UARTprintf((const char*)"== End PV Periodic Data Transmission ==\n\n");
	//Free the file descriptor environment for this Task
	//fdCloseSession( (HANDLE)Task_self() );
	//return Periodic_Result;
	return Head_Num;
}



int GenerateAMJoinCommandPK(unsigned char *Join_Array)
{	int i;
	char tempserver_ip[50]={0};
	for(i=0;i<6;i++)
		Join_Array[i+6] = G_ENVCONFIG.Mac[i];//macAddress[i];    // Join_Array[6]~Join_Array[11]
	for(i=12;i<AMJoinPKLength-1;i++)
		Join_Array[i] = 0xff;				// ignore port number

	Join_Array[16] = Port_L;
	Join_Array[17] = Port_H;



//// add extra parameter like AM fW version ....etc
	i=0;
	Join_Array[AM_regi_Server_Cmd_NUM+i++] = 2;
	Join_Array[AM_regi_Server_Cmd_NUM+i++] = AM_regi_Server_Cmd_FWV;
	Join_Array[AM_regi_Server_Cmd_NUM+i++] = (unsigned char )strlen((const char*)AM_FWversion)+1;
	memcpy(&Join_Array[AM_regi_Server_Cmd_NUM+i],AM_FWversion,strlen(AM_FWversion)+1);
	i=i+strlen(AM_FWversion)+1;

	Join_Array[AM_regi_Server_Cmd_NUM+i++]=AM_regi_Server_Cmd_Serip;
	sprintf(tempserver_ip,"%d.%d.%d.%d",G_ENVCONFIG.IP[0],G_ENVCONFIG.IP[1],G_ENVCONFIG.IP[2],G_ENVCONFIG.IP[3]);
	Join_Array[AM_regi_Server_Cmd_NUM+i++] =(unsigned char )strlen((const char*)tempserver_ip)+1;///server ip length
	memcpy(&Join_Array[AM_regi_Server_Cmd_NUM+i],tempserver_ip,strlen(tempserver_ip)+1);
	i=i+strlen(tempserver_ip)+1;



	return 0;
}

int GenerateJBJoinCommandPK(unsigned char *Join_Array,int Table_Index)
{	int i;
	int temp_Index=0;
	for(i=0;i<6;i++)
		Join_Array[i+6] = G_ENVCONFIG.Mac[i];//macAddress[i];    // Join_Array[6]~Join_Array[11]
	Join_Array[12] = 0x01;
	//UARTprintf((const char*)"Before_Mac[%d] %x %x %x %x %x %x\n",Table_Index,member_table[Table_Index].MAC[5],member_table[Table_Index].MAC[4],member_table[Table_Index].MAC[3],member_table[Table_Index].MAC[2],member_table[Table_Index].MAC[1],member_table[Table_Index].MAC[0]);
	if(pv_info_table[Table_Index].Serial_Number[0]==0xFF)
	{
		if(Table_Index==0)
			temp_Index=Table_Index+1;
		else
			temp_Index=Table_Index-1;
		for(i=13;i<19;i++)
			Join_Array[i] = member_table[Table_Index].MAC[i-13];
		for(i=19;i<43;i++)
			Join_Array[i] = pv_info_table[temp_Index].Serial_Number[i-19];
		for(i=43;i<67;i++)
			Join_Array[i] = pv_info_table[temp_Index].Firmware_Version[i-43];
		for(i=67;i<91;i++)
			Join_Array[i] = pv_info_table[temp_Index].Hardware_Version[i-67];
		for(i=91;i<115;i++)
			Join_Array[i] = pv_info_table[temp_Index].Device_Specification[i-91];
		for(i=115;i<123;i++)
			Join_Array[i] = pv_info_table[temp_Index].Manufacture_Date[i-115];
	}
	else
	{
		for(i=13;i<19;i++)
			Join_Array[i] = member_table[Table_Index].MAC[i-13];
		for(i=19;i<43;i++)
			Join_Array[i] = pv_info_table[Table_Index].Serial_Number[i-19];
		for(i=43;i<67;i++)
			Join_Array[i] = pv_info_table[Table_Index].Firmware_Version[i-43];
		for(i=67;i<91;i++)
			Join_Array[i] = pv_info_table[Table_Index].Hardware_Version[i-67];
		for(i=91;i<115;i++)
			Join_Array[i] = pv_info_table[Table_Index].Device_Specification[i-91];
		for(i=115;i<123;i++)
			Join_Array[i] = pv_info_table[Table_Index].Manufacture_Date[i-115];
	}
	return 0;
}

int GeneratePVPeriodicCommandPK(unsigned char *Periodic_Array,int Head,int Tail)
{
	int i,j;
	int Index;
	int Count=0;
	/*for(i=0;i<4;i++)
		Periodic_Array[i+6] = RTC_Array[3-i];    // Periodic_Array[6]~Periodic_Array[9] time stamp*/
	Periodic_Array[6] = Periodic_Update_time.mday;
	Periodic_Array[7] = Periodic_Update_time.hour;
	Periodic_Array[8] = Periodic_Update_time.min;
	Periodic_Array[9] = Periodic_Update_time.sec;
	Periodic_Array[10] = TCP_Periodic_Link_time.Updata_Period[0];//0x14;		// Update Period[1]
	Periodic_Array[11] = TCP_Periodic_Link_time.Updata_Period[1];//0x00;		// Update Period[2]
	Index = Head;
	i = 0;
	while(Index<=Tail)
	{
		if(member_table[Index].Valid>=0 && member_table[Index].state ==JB_Join2Server)
		{
			if(pv_value_table[Index].Alert_State[0]==0 || pv_value_table[Index].Alert_State[0]==1)
			{

				for(j=0;j<6;j++)
					Periodic_Array[13+i*19+j] = member_table[Index].MAC[j]; //MAC

				for(j=1;j<3;j++)
					Periodic_Array[13+i*19+j+6] = pv_value_table[Index].Voltage[j-1]; //Payload
				for(j=3;j<5;j++)
					Periodic_Array[13+i*19+j+6] = pv_value_table[Index].Current[j-3]; //Payload
				for(j=5;j<9;j++)
					Periodic_Array[13+i*19+j+6] = pv_value_table[Index].Power_Energy[j-5]; //Payload
				for(j=9;j<13;j++)
					Periodic_Array[13+i*19+j+6] = pv_value_table[Index].Alert_State[j-9]; //Payload

					Periodic_Array[13+i*19+6]=pv_value_table[Index].Diode_Temperature;	//Payload
			}

			if(pv_value_table[Index].Alert_State[0]==JB_Offline)
			{
				member_table[Index].state = JB_Free;
			}

			Count++;

			i++;

		}
		Index++;
		if(Count==25) break;


	}
	Periodic_Array[12] = Count;
	return Index;
}

//*  Created on: 2014/12/31
// *      Author: dong
unsigned char * Update_RTC_Request()
{
	unsigned char *RTC_Array = 0;
	unsigned char *Recv_Array = 0;
	int recv_num = 0;
	int i;
	unsigned char *RTC_Result;

	server_sock = INVALID_SOCKET;

	server_IPAddr = inet_addr(server_ip);

	// Allocate the file descriptor environment for this Task
	//fdOpenSession( (HANDLE)Task_self() );
	UARTprintf((const char*)"\n== Start Update RTC ==\n");

	// Create test socket
	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( server_sock == INVALID_SOCKET )
	{
		UARTprintf((const char*)"failed socket create (%d)\n",fdError());
		goto leave;
	}

	// Prepare address for connect
	bzero( &server_sin, sizeof(struct sockaddr_in) );
	server_sin.sin_family = AF_INET;
	server_sin.sin_len = sizeof( server_sin );
	server_sin.sin_addr.s_addr = server_IPAddr;
	server_sin.sin_port = htons(server_port);

	// Configure our Tx and Rx timeout to be 2 seconds
	server_timeout.tv_sec = 100;
	server_timeout.tv_usec = 0;
	setsockopt( server_sock, SOL_SOCKET, SO_SNDTIMEO, &server_timeout, sizeof( server_timeout ) );
	setsockopt( server_sock, SOL_SOCKET, SO_RCVTIMEO, &server_timeout, sizeof( server_timeout ) );

	// Connect socket
	if( connect( server_sock, (PSA) &server_sin, sizeof(server_sin) ) < 0 )
	{
		UARTprintf((const char*)"failed connect (%d)\n",fdError());
		goto leave;
	}
	// Allocate a working buffer
	if( !(RTC_Array = malloc( UpdateRTCPKLength )) )
	{
		UARTprintf((const char*)"failed RTC_Array buffer allocation\n");
		goto leave;
	}
	if( !(Recv_Array = malloc( UpdateRTCResponsePKL )) )
	{
		UARTprintf((const char*)"failed Recv_Array buffer allocation\n");
		goto leave;
	}
	// RTC_Result Array
	if( !(RTC_Result = malloc( UpdateRTCResultLength )) )
	{
		UARTprintf((const char*)"failed RTC_Result buffer allocation\n");
		goto leave;
	}

	// generate Update RTC Package
	RTC_Array[0] = AMJoinHeader;
	RTC_Array[1] = 0x04;	// Package Length
	RTC_Array[2] = 0x00;	// Package Length = 0x0004
	RTC_Array[3] = 0;	// Sequence Number
	RTC_Array[4] = UpdateRTCCommand;
	RTC_Array[5] = 0x01;	// # of Command Packet
	//GenerateUpdateRTCCommandPK(RTC_Array);
	RTC_Array[5] = 0x0C;
	RTC_Array[UpdateRTCPKLength-1] = UpdateRTCPKTailCode;

	// Send Update RTC Package
	if( send( server_sock, RTC_Array, UpdateRTCPKLength, 0 ) < 0 )
		{
			UARTprintf((const char*)"send failed (%d)\n",fdError());
			goto leave;
		}
	UARTprintf((const char*)"== Fin Update RTC ==\n\n");

	// receive Server's Response
	recv_num = recv( server_sock, Recv_Array, UpdateRTCResponsePKL, MSG_WAITALL );

	if( recv_num < 0 )
	{
		UARTprintf((const char*)"recv failed (%d)\n",fdError());
		goto leave;
	}
	else
	{
		UARTprintf((const char*)"Recv: %d bytes \n",recv_num);
	}

leave:

	switch(Recv_Array[UpdateRTCResponsePos])
	{	case UpdateRTCResponsePKCom:
			 // RTC_Result Setting
			 for ( i=0;i<6;i++)
			 {
				RTC_Result[i] = Recv_Array[i+6];
			 }

		break;

		default: RTC_Result = NULL;
	}

	// Free Pointer & Socket
	if( RTC_Array )
		free( RTC_Array );
	if( Recv_Array )
		free( Recv_Array );
	if( server_sock != INVALID_SOCKET )
		fdClose( server_sock );
	UARTprintf((const char*)"== End Update RTC ==\n\n");
	//Free the file descriptor environment for this Task
	//fdCloseSession( (HANDLE)Task_self() );
	return RTC_Result;
}

int TCP_Periodic_Link()
{
	unsigned char *Periodic_Array = 0;
	unsigned char *Recv_Array = 0;
	unsigned char *JB_MAC = 0;
	int IPlen=0;
	int recv_num = 0;
	char Link_Result;
	int i,j,pos,ResponseCPK;
	UINT16 Link_Time;
	char *temp;
	int  TcplinkLen=0;

	server_sock = INVALID_SOCKET;
	//struct timeval timeout;

	server_IPAddr = inet_addr(server_ip);

	// Allocate the file descriptor environment for this Task
	//fdOpenSession( (HANDLE)Task_self() );
	UARTprintf((const char*)"\n== Start TCP Periodic Link ==\n");

	// Create test socket
	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( server_sock == INVALID_SOCKET )
	{
		UARTprintf((const char*)"failed socket create (%d)\n",fdError());
		goto leave;
	}

	// Prepare address for connect
	bzero( &server_sin, sizeof(struct sockaddr_in) );
	server_sin.sin_family = AF_INET;
	server_sin.sin_len = sizeof( server_sin );
	server_sin.sin_addr.s_addr = server_IPAddr;
	server_sin.sin_port = htons(server_port);

	// Configure our Tx and Rx timeout to be 40 seconds
	server_timeout.tv_sec = 100;
	server_timeout.tv_usec = 0;
	setsockopt( server_sock, SOL_SOCKET, SO_SNDTIMEO, &server_timeout, sizeof( server_timeout ) );
	setsockopt( server_sock, SOL_SOCKET, SO_RCVTIMEO, &server_timeout, sizeof( server_timeout ) );

	// Connect socket
	if( connect( server_sock, (PSA) &server_sin, sizeof(server_sin) ) < 0 )
	{
		UARTprintf((const char*)"failed connect (%d)\n",fdError());
		goto leave;
	}
	// Allocate a working buffer
	if( !(Periodic_Array = malloc( TCPeriodicLINKPKLength )) )
	{
		UARTprintf((const char*)"failed Periodic_Array buffer allocation\n");
		goto leave;
	}
	if( !(Recv_Array = calloc( TCPeriodicLINKResponsePKL,1 )) )
	{
		UARTprintf((const char*)"failed Recv_Array buffer allocation\n");
		goto leave;
	}


	TcplinkLen=GenerateTCPLINKCommandPK(Periodic_Array,1);
	// generate TCP Periodic Link Package
	Periodic_Array[0] = AMPVHeader;
	Periodic_Array[1] = TcplinkLen-3;	// Package Length
	Periodic_Array[2] = 0x00;	// Package Length = 0x000D
	Periodic_Array[3] = 0;		// Sequence Number
	Periodic_Array[4] = TCPeriodicLINKCommand;
	Periodic_Array[5] = 0x01;	// # of Command Packet
	Periodic_Array[TcplinkLen++] = TCPeriodicLINKPKTailCode;

	// Send TCP Periodic Link Package
	if( send( server_sock, Periodic_Array, TcplinkLen, 0 ) < 0 )
		{
			UARTprintf((const char*)"send failed (%d)\n",fdError());
			goto leave;
		}
	UARTprintf((const char*)"== Fin TCP Periodic Link ==\n\n");

	// receive Server's Response
	recv_num = recv( server_sock, Recv_Array, TCPeriodicLINKResponsePKL, MSG_WAITALL );

	if( recv_num < 0 )
	{
		UARTprintf((const char*)"Recv failed (%d)\n",fdError());
		Link_Result = Link_Error;
		goto leave;
	}
	else
	{
		UARTprintf((const char*)"Recv: %d bytes \n",recv_num);
	}

leave:

	switch(Recv_Array[LINKResponsePKCommandPos])
	{
		case 0x0b:
			 /* Array Manager Setting */
			 ResponseCPK = Recv_Array[5]; // how many command
			 //UARTprintf((const char*)"CommandHandle:%d \n",ResponseCPK);
			 pos = 6+TcpLinkAM_Send_Arrary_CmdStart; // first command type
			 for (j =0;j < ResponseCPK; j++)
			 {
				Link_Result = 0;
				switch(Recv_Array[pos])
				{
					case TcpLinkAMUpdateRTC:
						// Update - RTC
						UARTprintf((const char*)"\n== Update - RTC ==\n");
						/*for ( i=0;i<6;i++)
						{
							Recv_Array[i] = Recv_Array[i+pos+1];
						}*/
						//-----------------------------------------------------------------------------------------------------------------//
						AM_time.sec=Recv_Array[pos+6];
						AM_time.min=Recv_Array[pos+5];
						AM_time.hour=Recv_Array[pos+4];
						AM_time.mday=Recv_Array[pos+3];
						AM_time.mon=Recv_Array[pos+2] ;
						AM_time.year=Recv_Array[pos+1] ;
						//UARTprintf((const char*)"\n sec:%d min:%d hour:%d mday:%d mon:%d year:%d \n",AM_time.sec,AM_time.min,AM_time.hour,AM_time.mday,AM_time.mon,AM_time.year);
						pos = pos + CommandUpdateRTCPKL;
					break;
					case TcpLinkAMUpdatePeriod:
						// Update - Update Period
						UARTprintf((const char*)"\n== Update - Update Period ==\n");
						for ( i=0;i<1;i++)
						{
							//Update_Period_Array[i] = Recv_Array[i+pos+1];
							TCP_Periodic_Link_time.Updata_Period[i] = Recv_Array[i+pos+1];
						}

						temp = (char * ) &Link_Time;
						*temp = TCP_Periodic_Link_time.Updata_Period[0];
						*(temp+1) = TCP_Periodic_Link_time.Updata_Period[1];
						Clock_setPeriod(Periodic_Handle,Link_Time*Time_Tick);
						UARTprintf((const char*)"\n\n== Period Change : %d sec==\n\n",Link_Time);
						//
						pos = pos + CommandUpdatePeriodPKL;
					break;
					case TcpLinkAMUpdateTCPP:
						// Update - TCP Link Period
						UARTprintf((const char*)"\n== Update - TCP Link Period ==\n");
						for ( i=0;i<1;i++)
						{
							//TCP_Periodic_Array[i] = Recv_Array[i+pos+1];
							TCP_Periodic_Link_time.TCP_Link_Period[i] = Recv_Array[i+pos+1];
						}
						pos = pos + CommandUpdateTCPPPKL;
					break;
					case TcpLinkAMResetAM:
						// Reset AM
						UARTprintf((const char*)"== Reset AM ==\n\n");
						Link_Result = Link_Reset_AM;
						pos = pos + CommandResetAMPKL;
					break;
					case TcpLinkAMResetJB:
						// Reset JB
						UARTprintf((const char*)"== Reset JB ==\n\n");
						if( !(JB_MAC = calloc( 6,1 )) )
						{
							UARTprintf((const char*)"failed JB_MAC buffer allocation\n");
							break;
						}
						for ( i=0;i<6;i++)
						{
							JB_MAC[i] = Recv_Array[i+pos+1];
						}
						//Reset_JB_MAC(JB_MAC);
						if( JB_MAC )
							free( JB_MAC );
						pos = pos + CommandResetJBPKL;
					break;
					case TcpLinkAMCHGAMTarget:
						// Change AM Target

						UARTprintf((const char*)"== Change AM Target ==\n\n");
						IPlen=Recv_Array[pos+1];

						if(IPlen==strlen((char*)&Recv_Array[pos+2]));
						{
							SaveIP2Evn((char*)&Recv_Array[pos+2]);
							UARTprintf((const char*)"Rewrite_Environment ==\n\n");
							Rewrite_Environment();
						}


						UARTprintf((const char*)"== Reset AM ==\n\n");
						Link_Result = Link_Reset_AM;
						pos = pos + CommandCHGAMTargetPKL;
					break;

					case TcpLinkAMUpdateFW:
						UARTprintf((const char*)"\n==TcpLinkAMUpdateFW ==\n");
						int StartBackupFwAdd=0x90000;
						int i=0;
						int BurnAddr=0x90000;
						int Fsize;
						int Rev_SegmentSize=RevFwSegment;
						int SegmentSizeCount=0;
						int remainder;
						Fsize=Recv_Array[pos+TCL_UpFwCmd_Offset_Len_LL]+(Recv_Array[pos+TCL_UpFwCmd_Offset_Len_LH]<<8)+(Recv_Array[pos+TCL_UpFwCmd_Offset_Len_HL]<<16)+(Recv_Array[pos+TCL_UpFwCmd_Offset_Len_HH]<<24);

						UARTprintf((const char*)"Fsize : %d \n\n",Fsize);

						SegmentSizeCount=Fsize/Rev_SegmentSize;
						if(Fsize%Rev_SegmentSize) SegmentSizeCount++;

						for(i=0;i<SegmentSizeCount;i++)
						{
							if(i==(SegmentSizeCount-1)) Rev_SegmentSize=Fsize%Rev_SegmentSize;

							recv_num = recv( server_sock, FW_Rev_SegmentArray, Rev_SegmentSize, MSG_WAITALL );
							UARTprintf((const char*)"\n   recv_num=%d  ==TcpLinkAMUpdateFW ==\n",recv_num);
							if(recv_num)
							{
							BurnAddr=StartBackupFwAdd+RevFwSegment*i;
							DynFlashErase(BurnAddr);
							//memeset(FW_Rev_SegmentArray,0x55,Rev_SegmentSize);
							//for(j=0;j<Rev_SegmentSize;j++)
								//*(FW_Rev_SegmentArray+j)=0x55;
							recv_num=recv_num+3;
							remainder=(recv_num)%4;
							if(remainder) recv_num=recv_num-remainder;
							DynFlashProgram(FW_Rev_SegmentArray,BurnAddr,recv_num);
							}
						}


						//UpdateFirmwar_Go(Fsize);

						break;

					default:
						UARTprintf((const char*)"unknown command type : %X \n\n",Recv_Array[pos]);
				}


			 }
		break;

		default:
			Link_Result = Link_Error;
			UARTprintf((const char*)"Error Command Type\n");
		break;
	}

	if(Link_Result == 0)
		Link_Result = Link_Completed;
	// Free Pointer & Socket
	if( Periodic_Array )
		free( Periodic_Array );
	if( Recv_Array )
			free( Recv_Array );
	if( server_sock != INVALID_SOCKET )
			fdClose( server_sock );
	UARTprintf((const char*)"== End TCP Periodic Link ==\n\n");
	//Free the file descriptor environment for this Task

	return Link_Result;
}

int GenerateTCPLINKCommandPK(unsigned char *Periodic_Array,int AM_EnOption)
{
	int i=6;
	/*for(i=0;i<6;i++)
		Periodic_Array[i+6] = RTC_Array[i];    // Array Manager RTC Time*/

	Periodic_Array[i++]= G_ENVCONFIG.Mac[0];
	Periodic_Array[i++] = G_ENVCONFIG.Mac[1];
	Periodic_Array[i++] = G_ENVCONFIG.Mac[2];
	Periodic_Array[i++] = G_ENVCONFIG.Mac[3];
	Periodic_Array[i++] = G_ENVCONFIG.Mac[4];
	Periodic_Array[i++] = G_ENVCONFIG.Mac[5];
	Periodic_Array[i++] = AM_EnOption;

	Periodic_Array[i++] = AM_NumInfo;
	Periodic_Array[i++] = TcpLinkAMUpdateRTC;
	Periodic_Array[i++] = 6;
	Periodic_Array[i++] = AM_time.sec;
	Periodic_Array[i++] = AM_time.min;
	Periodic_Array[i++] = AM_time.hour;
	Periodic_Array[i++] = AM_time.mday;
	Periodic_Array[i++] = AM_time.mon;
	Periodic_Array[i++] = AM_time.year;

	Periodic_Array[i++] = TcpLinkAMUpdatePeriod;
	Periodic_Array[i++] = 2;
	Periodic_Array[i++] = G_ENVCONFIG.Pollingtime[0];
	Periodic_Array[i++] = G_ENVCONFIG.Pollingtime[1];
	Periodic_Array[i++] = TcpLinkAM_Cmd_End;

#if 0
	Periodic_Array[11] = AM_time.sec;
	Periodic_Array[10] = AM_time.min;
	Periodic_Array[9] = AM_time.hour;
	Periodic_Array[8] = AM_time.mday;
	Periodic_Array[7] = AM_time.mon;
	Periodic_Array[6] = AM_time.year;
	Periodic_Array[12] = TCP_Periodic_Link_time.Updata_Period[0];//0x3c;		// Update Period[1]
	Periodic_Array[13] = TCP_Periodic_Link_time.Updata_Period[1];//0x00;		// Update Period[2]
	Periodic_Array[14] = TCP_Periodic_Link_time.TCP_Link_Period[0];//0x3c;	// TCP LINK Period[1]
	Periodic_Array[15] = TCP_Periodic_Link_time.TCP_Link_Period[1];//0x00;	// TCP LINK Period[2]
#endif
	return i;
}

int Get_EvnString(void) {
	char* ptr;
	int stringCount = 0;
	char StringBuf[10][50];
	char TempBuf[100];
	int tempbufindex = 0;
	int retConut = 0;
	char* parserStart;
	int i;

	ptr = (char*) EnvStartAdd;
	//memset(StringBuf, 0, sizeof(StringBuf));

	while (retConut < ContiRetCount && tempbufindex < 40) {

		if (*ptr == 0x0d && *(ptr + 1) == 0x0a) {
			TempBuf[tempbufindex] = 0;
			strcpy(StringBuf[stringCount], TempBuf);
			stringCount += 1;
			tempbufindex = 0;
			retConut = 0;
			ptr++;
		} else {
			if (*ptr == 0xff) {
				retConut++;
			}
			TempBuf[tempbufindex++] = *ptr;

		}

		ptr++;

	}

	for (i = 0; i < stringCount; i++) {

		if ((parserStart = strstr(StringBuf[i], "MAC="))) {
			SaveMac2Evn(parserStart + strlen("MAC="));
			//UARTprintf((const char*)"\nI'm in MAC scope\n");
			//UARTprintf((const char*)"MAC = %s\n",StringBuf[i]);
		}

		if ((parserStart = strstr(StringBuf[i], "SN="))) {
			SaveSNEvn(parserStart + strlen("SN="));
		}
		if ((parserStart = strstr(StringBuf[i], "IP="))) {
			SaveIP2Evn(parserStart + strlen("IP="));
				}

		if ((parserStart = strstr(StringBuf[i], "Polling_Time="))) {
			SavePtime2Evn(parserStart + strlen("Polling_Time="));
		}

		if ((parserStart = strstr(StringBuf[i], "ETH_MAC="))) {
			SaveETHMACtime2Evn(parserStart + strlen("ETH_MAC="));
		}


	}

	return stringCount;
}

int SaveETHMACtime2Evn(char*Start) {
	uint8_t RetHex[1];

	G_ENVCONFIG.EthMac[0] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.EthMac[1] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.EthMac[2] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.EthMac[3] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.EthMac[4] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.EthMac[5] = *hex_decode(Start, 2, RetHex);

	return 0;
}
int SaveMac2Evn(char*Start) {
	uint8_t RetHex[1];

	G_ENVCONFIG.Mac[0] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.Mac[1] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.Mac[2] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.Mac[3] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.Mac[4] = *hex_decode(Start, 2, RetHex);
	Start = strstr(Start, ":") + 1;
	G_ENVCONFIG.Mac[5] = *hex_decode(Start, 2, RetHex);

	return 0;
}

int SaveSNEvn(char*Start) {
	int i = 0;
	G_ENVCONFIG.SeriesNumber[0] = *(Start + i++) - 0x30;
	G_ENVCONFIG.SeriesNumber[1] = *(Start + i++) - 0x30;
	G_ENVCONFIG.SeriesNumber[2] = *(Start + i++) - 0x30;
	G_ENVCONFIG.SeriesNumber[3] = *(Start + i++) - 0x30;
	G_ENVCONFIG.SeriesNumber[4] = *(Start + i++) - 0x30;
	G_ENVCONFIG.SeriesNumber[5] = *(Start + i++) - 0x30;
	G_ENVCONFIG.SeriesNumber[6] = *(Start + i++) - 0x30;
	G_ENVCONFIG.SeriesNumber[7] = *(Start + i++) - 0x30;

	return 0;
}


int SavePtime2Evn(char*Start)
{

	int count=0;
	char* ptr = Start;
	uint16_t Pollingtime; //= (*Start-'0')*100+((*(Start+1)-'0'))*10+*(Start+2)-'0';


	while(*ptr>='0' && *ptr<='9'){
		count++;
		ptr++;
	}

	if(count==4)
		Pollingtime = (*Start-'0')*1000+(*(Start+1)-'0')*100+(*(Start+2)-'0')*10+(*(Start+3)-'0');
	if(count==3)
		Pollingtime = (*Start-'0')*100+((*(Start+1)-'0'))*10+*(Start+2)-'0';
	if(count==2)
		Pollingtime = ((*(Start)-'0'))*10+*(Start+1)-'0';
	if(count==1)
		Pollingtime = (*Start-'0');

	G_ENVCONFIG.Pollingtime[0] = Pollingtime>>8;		//high
	G_ENVCONFIG.Pollingtime[1] = Pollingtime & 0xff;	//low

	return 0;
}

int SaveIP2Evn(char*Start)
{
	char *ptr = Start;
	int i=0;
	int j=0;
	for(j=0;j<4;j++){


	while(*Start>='0' && *Start<='9') Start++;
	//UARTprintf((const char*)"Start-ptr = %d\n",Start-ptr);
	if(Start==ptr+1){
		G_ENVCONFIG.IP[i] = *ptr-'0';
		i++;
		ptr = Start+1;
	}
	else if(Start==ptr+2){
		G_ENVCONFIG.IP[i] = (*ptr-'0')*10+(*(ptr+1)-'0');
		i++;
		ptr = Start+1;
	}
	else if(Start==ptr+3){
		G_ENVCONFIG.IP[i] = (*ptr-'0')*100+((*(ptr+1)-'0'))*10+*(ptr+2)-'0';
		i++;
		ptr = Start+1;
	}
	else
		UARTprintf((const char*)"ERROR!!!!!!!!!!!\n");

		Start++;

	//UARTprintf((const char*)"IP[i] = %d \n",G_ENVCONFIG.IP[j]);
	}

	return 0;
}


uint8_t* hex_decode(char *in, size_t len, uint8_t *out) {
	unsigned int i, t, hn, ln;

	for (t = 0, i = 0; i < len; i += 2, ++t) {

		hn = in[i] > '9' ? (in[i] | 32) - 'a' + 10 : in[i] - '0';
		ln = in[i + 1] > '9' ? (in[i + 1] | 32) - 'a' + 10 : in[i + 1] - '0';

		out[t] = (hn << 4) | ln;
	}
	return out;

}

void Rewrite_Environment(void)
{
	uint32_t ptr=0x80000;
	int EvnLen=0;
	int tempStrLen;
	char ServeripArray[20];
	char AM_MacArray[30];
	char AM_PollingArray[20];
	char AM_SerierArray[20];
	char NextLine[2]={0x0d,0x0a};
	FW_Rev_SegmentArray[0]=0;
	char Zero[1]={0};



	sprintf(ServeripArray,"IP=%d.%d.%d.%d",G_ENVCONFIG.IP[0],G_ENVCONFIG.IP[1],G_ENVCONFIG.IP[2],G_ENVCONFIG.IP[3]);
	sprintf(AM_MacArray,"MAC=%x:%x:%x:%x:%x:%x",G_ENVCONFIG.Mac[0],G_ENVCONFIG.Mac[1],G_ENVCONFIG.Mac[2],G_ENVCONFIG.Mac[3],G_ENVCONFIG.Mac[4],G_ENVCONFIG.Mac[5]);
	sprintf(AM_PollingArray,"Polling_Time=%d",G_ENVCONFIG.Pollingtime[1]);
	sprintf(AM_SerierArray,"SN=%d%d%d%d%d%d%d%d",G_ENVCONFIG.SeriesNumber[0],G_ENVCONFIG.SeriesNumber[1],G_ENVCONFIG.SeriesNumber[2],G_ENVCONFIG.SeriesNumber[3],G_ENVCONFIG.SeriesNumber[4]\
			,G_ENVCONFIG.SeriesNumber[5],G_ENVCONFIG.SeriesNumber[6],G_ENVCONFIG.SeriesNumber[7]);

	tempStrLen=strlen(AM_MacArray);
	//UARTprintf((const char*)"%d\n",tempStrLen);

	memcpy(FW_Rev_SegmentArray+EvnLen,AM_MacArray,tempStrLen);
	EvnLen=EvnLen+tempStrLen;
	memcpy(FW_Rev_SegmentArray+EvnLen,NextLine,2);
	EvnLen=EvnLen+2;

	tempStrLen=strlen(AM_PollingArray);
	memcpy(FW_Rev_SegmentArray+EvnLen,AM_PollingArray,tempStrLen);
	EvnLen=EvnLen+tempStrLen;
	memcpy(FW_Rev_SegmentArray+EvnLen,NextLine,2);
	EvnLen=EvnLen+2;

	tempStrLen=strlen(ServeripArray);
	memcpy(FW_Rev_SegmentArray+EvnLen,ServeripArray,tempStrLen);
	EvnLen=EvnLen+tempStrLen;
	memcpy(FW_Rev_SegmentArray+EvnLen,NextLine,2);
	EvnLen=EvnLen+2;

	tempStrLen=strlen(AM_SerierArray);
	memcpy(FW_Rev_SegmentArray+EvnLen,AM_SerierArray,tempStrLen);
	EvnLen=EvnLen+tempStrLen;
	memcpy(FW_Rev_SegmentArray+EvnLen,NextLine,2);
	EvnLen=EvnLen+2;

	memcpy(FW_Rev_SegmentArray+EvnLen,Zero,1);

	//UARTprintf((const char*)"%s\n",FW_Rev_SegmentArray);

#if 0
	G_ENVCONFIG.Mac[0]=G_ENVCONFIG.Mac[0];
	G_ENVCONFIG.Mac[1]=G_ENVCONFIG.Mac[1];
	G_ENVCONFIG.Mac[2]=G_ENVCONFIG.Mac[2];
	G_ENVCONFIG.Mac[3]=G_ENVCONFIG.Mac[3];
	G_ENVCONFIG.Mac[4]=G_ENVCONFIG.Mac[4];
	G_ENVCONFIG.Mac[5]=G_ENVCONFIG.Mac[5];

	//G_ENVCONFIG.IP[0]=140;
	//G_ENVCONFIG.IP[1]=118;
	//G_ENVCONFIG.IP[2]=170;
	//G_ENVCONFIG.IP[3]=189;

	G_ENVCONFIG.Pollingtime[0]=0;
	G_ENVCONFIG.Pollingtime[1]=60;

	G_ENVCONFIG.SeriesNumber[0]=0;
	G_ENVCONFIG.SeriesNumber[1]=1;
	G_ENVCONFIG.SeriesNumber[2]=2;
	G_ENVCONFIG.SeriesNumber[3]=3;
	G_ENVCONFIG.SeriesNumber[4]=4;
	G_ENVCONFIG.SeriesNumber[5]=5;
	G_ENVCONFIG.SeriesNumber[6]=6;
	G_ENVCONFIG.SeriesNumber[7]=7;
#endif
	FlashErase(ptr);
	FlashProgram((uint32_t*)FW_Rev_SegmentArray,ptr,(EvnLen+3)&(0xfffffffc));
	//UARTprintf((const char*)"\nErace Flash Finish\n");
}
