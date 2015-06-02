/*
 * rs485.c
 *
 *  Created on: 2014/3/14
 *      Author: SHIN
 *  How to ues: Reference to rs485.h file
 *
 */
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTTiva.h>
#include "rs485.h"
#include "../DK_TM4C129X.h"
#include "../board.h"
#include <ti/drivers/GPIO.h>

static UART_Handle uart2;
static UART_Params uartParams;

int rs485_write(const unsigned char *buffer, UInt size)
{
	int i;
	int j=0;
	int num;
	GPIO_write(Board_RS485_DE_RE_PIN, RS485_WRITE);
	while(j<size)
	{
		num = UART_write(uart2, (char *)(buffer+j), 1);
		for(i=0;i<=1000;i++);
		j++;
	}
	GPIO_write(Board_RS485_DE_RE_PIN, RS485_READ);
	return num;
}
int rs485_read(unsigned char *buffer, UInt size)
{
	int num;
	GPIO_write(Board_RS485_DE_RE_PIN, RS485_READ);
	num = UART_read(uart2, (char *)buffer, size);
	return num;
}
int rs485_init()
{
	UART_Params_init(&uartParams);
	uartParams.writeDataMode = UART_DATA_BINARY  ;
	uartParams.readDataMode = UART_DATA_BINARY  ;
	uartParams.readReturnMode = UART_RETURN_FULL;
	uartParams.readEcho = UART_ECHO_OFF;
	uartParams.baudRate = rs485_baudRate;
	uartParams.readTimeout = rs485_timeout;

	uart2 = UART_open(Board_UART2, &uartParams);

	if (uart2 == NULL ) {
		System_abort("Error opening the UART2");
		return 1;
		}

	return 0;
}
