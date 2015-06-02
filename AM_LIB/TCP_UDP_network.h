/*
 * TCP_UDP_network.h
 *
 *  Created on: 2014/3/24
 *      Author: SHIN
 */


 /* NDK Header files */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/ndk/inc/socket.h>
#include <xdc/runtime/System.h>
//#define server_ip "59.120.25.27"
//#define server_ip "140.118.170.189"
#define server_port 9966
#define Mac4 0x81
#define Mac5 0x11

//*****************************************************************************
//
// AM Join Data Protocol Packet - Number of Byte
//
//*  Created on: 2014/7/7
// *      Author: didi Lin
//*****************************************************************************
#define AMJoinPKLength 				221
#define AMJoinResponsePKLength 		8
#define AMJoinResultPKPos			6
#define AMJoinCommand 				0x03
#define CommandPKTailCode 			0x62


#define Port_H						0x26
#define Port_L						0xEE    // port 9966  => 26EE

#define Join_Success 				0x02
#define Join_Deny					0x03 	 // MAC Doesn't Exist
#define Join_Failed 				0x04	 // Retry on Next Period

////////////////////////////////////////////////////////////////////add by alex wang
#define AM_regi_Server_Cmd_NUM		18
#define AM_regi_Server_Cmd_FWV		01
#define AM_regi_Server_Cmd_Serip	02
extern UInt8 macAddress[6];

//*****************************************************************************
//
//				 JB Join Data Protocol Packet - Number of Byte
//
//				 Author: didi Lin
//				 Created on: 2014/9/5
//*****************************************************************************
#define JBJoinPKLength 				124
#define JBJoinResponsePKLength 		14
#define JBJoinCommand 				0x08
#define JBJoinResultPKPos			6

//*****************************************************************************
//
//				 Periodic Data Transmission
//
//				 Author: dong Dong
//				 Created on: 2014/12/8
//*****************************************************************************
#define PVPeriodicPKLength			489
#define PVPeriodicResponsePKLength	33
#define PVPeriodicCommand 			0x07
#define PVPeriodicCommandPKTailCode	0x61
#define PVPeriodicUpdatePeriod1		0x00
#define PVPeriodicUpdatePeriod2		0x14

#define PVData_Success		0x00
#define PVData_NoResponse 	0x01
#define PVData_Error 		0x02
//*****************************************************************************
//
//				 Synchronization Update RTC
//
//				 Author: dong Dong
//				 Created on: 2014/12/31
//*****************************************************************************
#define UpdateRTCPKLength			8
#define UpdateRTCCommand			0x05
#define UpdateRTCPKTailCode			0x62

#define UpdateRTCResponsePKL		13
#define UpdateRTCResultLength		6

#define UpdateRTCResponsePKCom		0x06
#define UpdateRTCRespPKTaiCode		0x62
#define UpdateRTCResponsePos		4

//*****************************************************************************
//
//				 TCP Periodic Link
//
//				 Author: Alex wang
//				 Created on: 2015/04/23
// Note  be carefull consistence with server socket
//*****************************************************************************
#define TCPeriodicLINKPKLength		64
#define TCPeriodicLINKResponsePKL	32//////fix length for this command
#define TCPeriodicLINKCommand		0x0a
#define TCPeriodicLINKPKTailCode	0x61
#define LINKResponsePKCommandPos	4
#define TCPeriodicLINKResPKTailCode	0x61



#define TcpLinkAM_Send_Arrary_CmdStart		8
#define TcpLinkAM_Cmd_BasicLen				9

#define AM_EnLinkCmd_ResetAM		1
#define AM_EnLinkCmd_UpdateRTC  	2
#define AM_EnLinkCmd_UpdatePeriod   4

#define TcpLinkAM_Cmd_None		0
#define TcpLinkAMUpdatePeriod	0x01
#define TcpLinkAMUpdateTCPP		0x02
#define TcpLinkAMResetAM		0x03
#define TcpLinkAMResetJB		0x04
#define TcpLinkAMCHGAMTarget	0x05
#define TcpLinkAMUpdateRTC		0x06
#define TcpLinkAMUpdateFW		0x07
#define TcpLinkAM_Cmd_End		0xff


////// TcpLinkAMUpdateFW ///////////
#define TCL_UpFwCmd_Offset_Len_LL			1
#define TCL_UpFwCmd_Offset_Len_LH			2
#define TCL_UpFwCmd_Offset_Len_HL			3
#define TCL_UpFwCmd_Offset_Len_HH			4
#define RevFwSegment						16*1024
////// TcpLinkAMUpdateFW ///////////

#define AM_NumInfo				2


#define CommandUpdateRTCPKL		7
#define CommandUpdatePeriodPKL	3
#define CommandUpdateTCPPPKL	3
#define CommandResetAMPKL		5
#define CommandResetJBPKL		7
#define CommandCHGAMTargetPKL	7

#define Link_Reset_AM 0x00
#define Link_Completed 0x01
#define Link_Error 0x04


//*****************************************************************************
//
//				 Define Task Event & Flag state
//
//				 Author: didi Lin
//				 Created on: 2014/8/26
//*****************************************************************************
#define AM_Join 		0
#define JB_Join 		1
#define Socket_fail 	2
#define PV_Periodic		3
#define Update_RTC		4
#define TCPPeriodicLink	5
#define Environment		6
#define Nothing			20


//*****************************************************************************
//
// AM Header code - Number of Byte
//
//*****************************************************************************
#define AMJoinHeader 0x58
#define AMPVHeader 0x69

#define EnvStartAdd 0x80000
#define ContiRetCount	10

typedef struct ENVCONFIG {
	uint8_t Mac[6];
	uint8_t SeriesNumber[8];
	uint8_t Pollingtime[2];
	uint8_t IP[4];
	uint8_t EthMac[6];
} ENVCONFIG;

extern SOCKET server_sock;
extern struct sockaddr_in server_sin;
extern IPN server_IPAddr;
extern struct timeval server_timeout;


extern int network_initial();
extern int TCP_transmission(char *pbuf, int size);
extern int UDP_transmission();
extern int AM_JoinRequest();
extern int JB_JoinRequest(int Table_Idex);
extern int PV_Periodic_Trans(int Head,int Tail);
extern int GenerateAMJoinCommandPK();
extern int GenerateJBJoinCommandPK();
extern int GeneratePVPeriodicCommandPK();
extern unsigned char * Update_RTC_Request();
extern int GenerateUpdateRTCCommandPK(unsigned char *);
extern int TCP_Periodic_Link();
extern int GenerateTCPLINKCommandPK(unsigned char *,int AM_EnOption);

int Get_EvnString(void);
int SaveMac2Evn(char*Start);
int SaveSNEvn(char*Start);
int SavePtime2Evn(char*Start);
int SaveETHMACtime2Evn(char*Start);
int SaveIP2Evn(char*Start);
uint8_t* hex_decode(char *in, size_t len, uint8_t *out);
void Rewrite_Environment(void);
