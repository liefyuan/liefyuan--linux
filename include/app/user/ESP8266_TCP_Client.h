/*
 * ESP8266_TCP_Client.h
 *
 *  Created on: 2015��6��28��
 *      Author: Administrator
 */

#ifndef APP_USER_TCP_CLIENT_H_
#define APP_USER_TCP_CLIENT_H_

void Inter213_InitTCP(char * ipAddress, int32_t port);
void Inter213_TCP_SendData(uint8 *buf,uint16 len);
void Esp8266_TCPClient_Time_Init(void);

#endif /* APP_USER_ESP8266_TCP_CLIENT_H_ */
