
#include "../AM_LIB/UART_PRINTF.h"
#include "../Bus_Raw_Protocol/Table.h"
#include "../Debug/configPkg/package/cfg/AM_model_pem4f.h"

#include "TCP_UDP_network.h"
#include <xdc/std.h>

#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>
#include <stdlib.h>
#include <stdio.h>

static int StartPV_Server=0;
static char *LocalIPAddr = "192.168.235.201";
static char *LocalIPMask = "255.255.255.0";
//char *GatewayIP   = "0.0.0.0";
static char *DomainName  = "demo.net";

#ifdef TelentServerPro
Void ti_ndk_config_Global_serviceReport(uint Item, uint Status,
uint Report, HANDLE h);
void mynetworkIPAddrSTATIC2DHCP(void);
void mynetworkIPAddrDHCP2STATIC(void);
Void TelentServer(UArg arg0, UArg arg1)
{

	SA_IN	sockaddr_in_Telent;
	SA_IN 	ClientAdd;
	IPN IPAddr;
	UINT8 DomainName[32];
	SOCKET server_sock;
	SOCKET RetAccept;
	int ClientSize;
	char SendBuf[256];
	//char RevBuf[256];
	int WaitDhcpCount=30;


	while(0)
	{
		TaskSleep(1000);
		StartPV_Server=1;
	}
	fdOpenSession( (HANDLE)Task_self() );

	bzero(&IPAddr,sizeof(IPAddr));

	while(1)
	{
		NtGetPublicHost(&IPAddr,sizeof(DomainName),DomainName);
		TaskSleep(1000);
		if(IPAddr) break;
		WaitDhcpCount--;
		if(WaitDhcpCount==0)
		{
			break;
		}

	}

	if(IPAddr)   /////trigger the task to start
	{
		StartPV_Server=1;
		//fdCloseSession( (HANDLE)Task_self() );
		//return ;
	}
	else
	{
		mynetworkIPAddrDHCP2STATIC();
		StartPV_Server=0;
		UARTprintf("TILENT-SERVER:Change the ip Addr to Static ip=%s\n",LocalIPAddr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////
	///////////////start to execute the telent server //////////////////////////////////////

	//mynetworkIPAddrSTATIC2DHCP();
	TaskSleep(10000);

	//NC_NetStop(0);
	//NtAddNetwork(1,0x02eba8c0,0xfceba8c0);

	NtGetPublicHost(&IPAddr,sizeof(DomainName),DomainName);


	//NtIfIdx2Ip(1,&IPAddr);

	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if( server_sock == INVALID_SOCKET )
	{
		UARTprintf((const char*)"failed socket create (%d)\n",fdError());
		while(1);
	}

		bzero( &sockaddr_in_Telent, sizeof(sockaddr_in_Telent) );
		sockaddr_in_Telent.sin_family = AF_INET;
		sockaddr_in_Telent.sin_len = sizeof( sockaddr_in_Telent );
		sockaddr_in_Telent.sin_addr.s_addr = IPAddr;
		sockaddr_in_Telent.sin_port = htons(23);

		bind(server_sock,(PSA)&sockaddr_in_Telent,sizeof(sockaddr_in_Telent));

		listen(server_sock,1);

		UARTprintf("TILENT-SERVER:Wait Client to Connect Server\n");

		while(1)
		{


			RetAccept=accept(server_sock,(PSA)&ClientAdd,&ClientSize);
			if(RetAccept>=0)
			{
				 System_printf("got client \n");
				 System_flush();
				 sprintf(SendBuf,"Welcome BIZLINK \r\n");
				 send(RetAccept,SendBuf,strlen(SendBuf)+1,0);
				 sprintf(SendBuf,"INPUT \r\n");
				 send(RetAccept,SendBuf,strlen(SendBuf)+1,0);
				 shutdown(RetAccept,SOCK_STREAM);
				 //recv(RetAccept,RevBuf,1,MSG_WAITALL);
			}
			else
			{
				shutdown(RetAccept,SOCK_STREAM);
			}
		}


	fdCloseSession( (HANDLE)Task_self() );

}

void mynetworkIPAddrDHCP2STATIC(void)
{
    HANDLE hCfg = CfgGetDefault();
    HANDLE hCfgitem;
    //char *ti_ndk_config_Global_HostName    = "tisoc";
    /* Static IP Address settings */
    //char *LocalIPAddr = "192.168.235.199";
    //char *LocalIPMask = "255.255.255.0";
    //char *GatewayIP   = "0.0.0.0";
    //char *DomainName  = "demo.net";
    if (hCfg) {

        CI_IPNET NA;
        CfgExecute(hCfg,0);

        CfgGetEntry(hCfg,CFGTAG_SERVICE,CFGITEM_SERVICE_DHCPCLIENT,1,&hCfgitem);
        CfgRemoveEntry(hCfg,hCfgitem);

        /* Setup manual IP address */
        bzero(&NA, sizeof(NA));
        NA.IPAddr  = inet_addr(LocalIPAddr);
        NA.IPMask  = inet_addr(LocalIPMask);
        strcpy(NA.Domain, DomainName);
        NA.NetType = 0;

        CfgAddEntry(hCfg, CFGTAG_IPNET, 1, 0,
                sizeof(CI_IPNET), (UINT8 *)&NA, 0);

        CfgExecute(hCfg,1);
    }
    else {
        System_printf("error: could not get handle to configuration\n");
    }
}

//////////////////////////dhcp ip & static ip switch///////////////////////////////////
void mynetworkIPAddrSTATIC2DHCP(void)
{
    HANDLE hCfg = CfgGetDefault();
    HANDLE hCfgitem;
    if (hCfg) {

    	CfgExecute(hCfg,0);

        System_printf("now adding DHCP...\n");

        // Specify DHCP Service on IF-1
        	   CfgGetEntry(hCfg,CFGTAG_SYSINFO,CFGITEM_DHCP_HOSTNAME,1,&hCfgitem);
               CfgRemoveEntry(hCfg,hCfgitem);
               CfgGetEntry(hCfg,CFGTAG_ROUTE,0,1,&hCfgitem);
               CfgRemoveEntry(hCfg,hCfgitem);
        	   CfgGetEntry(hCfg,CFGTAG_IPNET,1,1,&hCfgitem);
               CfgRemoveEntry(hCfg,hCfgitem);


               CI_SERVICE_DHCPC dhcpc;
               UINT8 DHCP_OPTIONS[] =
                       {
                       DHCPOPT_SUBNET_MASK,
                       };

               /* Specify DHCP Service on IF specified by "IfIdx" */
               bzero(&dhcpc, sizeof(dhcpc));
               dhcpc.cisargs.Mode   = 1;
               dhcpc.cisargs.IfIdx  = 1;
               dhcpc.cisargs.pCbSrv = &ti_ndk_config_Global_serviceReport;
               dhcpc.param.pOptions = DHCP_OPTIONS;
               dhcpc.param.len = 1;
               CfgAddEntry(hCfg, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, 0,
                       sizeof(dhcpc), (UINT8 *)&dhcpc, 0);

        	CfgExecute(hCfg,1);
    }
    else {
        System_printf("error: could not get handle to configuration\n");
    }
}
#endif
