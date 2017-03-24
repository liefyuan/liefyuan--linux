/*
 * my_wifi.c
 *
 *  Created on: 2017Äê3ÔÂ22ÈÕ
 *      Author: yuanlifu
 */
#include "../include/my.h"

#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"


os_event_t    my_recvTaskQueue[my_recvTaskQueueLen];
os_event_t    my_procTaskQueue[my_procTaskQueueLen];

my_stateType  my_state;
BOOL echoFlag = TRUE;
BOOL specialAtState = TRUE;

static void my_procTask(os_event_t *events);
static void my_recvTask(os_event_t *events);

/**
  * @brief  Uart receive task.
  * @param  events: contain the uart receive data
  * @retval None
  */
static void ICACHE_FLASH_ATTR
my_recvTask(os_event_t *events)
{
	static uint8_t myHead[2];
	static uint8_t *pCmdLine;
	uint8_t temp;

	while(READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
	{
		WRITE_PERI_REG(0X60000914, 0x73); //WTD

		if(my_state != my_statIpTraning)
		{
			temp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
			if((temp != '\n') && (echoFlag))
			 {
				uart_tx_one_char(temp); //display back
			 }
		}

	   switch(my_state)
	   {
		    case my_statIdle: //serch "AT" head
		    	myHead[0] = myHead[1];
		    	myHead[1] = temp;
				  if((os_memcmp(myHead, "AT", 2) == 0) || (os_memcmp(myHead, "at", 2) == 0))
				  {
					my_state = my_statRecving;
					myHead[1] = 0x00;
				  }
				  else if(temp == '\n') //only get enter
				  {
					  uart_sendString("\r\nERROR\r\n");
				  }
				  break;

		    case my_statRecving: //push receive data to cmd line
		    	Inter213_TCP_SendData("this is my test!",strlen("this is my test!"));
		    	system_os_post(my_procTaskPrio, 0, 0);
		    	my_state = my_statProcess;
		    	if(echoFlag)
				{
		    		uart_sendString("statRecving\r\n"); ///////////
				}
		      break;

		    case my_statProcess: //process data
		    	//Inter213_TCP_SendData("this is my test!",strlen("this is my test!"));
		    	if(temp == '\n')
			    {
		            //system_os_post(at_busyTaskPrio, 0, 1);
			        uart_sendString("\r\nbusy p...\r\n");
			    }
		      break;

		    case my_statIpSending:

		      break;

		    case my_statIpSended: //send data

		      break;

		    case my_statIpTraning:

		      break;
		    default:

		      break;
		 }
	}
	if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
	}
	else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
	}
	ETS_UART_INTR_ENABLE();
}

/**
  * @brief  Task of process command or txdata.
  * @param  events: no used
  * @retval None
  */
static void ICACHE_FLASH_ATTR
my_procTask(os_event_t *events)
{
	if(my_state == my_statProcess)
	{
		Inter213_TCP_SendData("this is my test!",strlen("this is my test!"));
		if(specialAtState)
		{
		    my_state = my_statIdle;
		}
	}
	else if(my_state == my_statIpSended)
	{
	}
	else if(my_state == my_statIpTraning)
	{
	}
}


void ICACHE_FLASH_ATTR
my_init(void){
	Esp8266_TCPClient_Time_Init();
	system_os_task(my_recvTask, my_recvTaskPrio, my_recvTaskQueue, my_recvTaskQueueLen);
	system_os_task(my_procTask, my_procTaskPrio, my_procTaskQueue, my_procTaskQueueLen);
}

