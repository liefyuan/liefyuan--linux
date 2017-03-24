/*
 * Esp8266_TCP.h
 *
 *  Created on: 2015Äê6ÔÂ27ÈÕ
 *      Author: Administrator
 */

#ifndef APP_USER_ESP8266_TCP_H_
#define APP_USER_ESP8266_TCP_H_

void WIFI_APInit();
void WIFI_ServerMode(void);
void TcpServer_Listen_PCon(void *arg);
void WIFI_TCP_SendNews(unsigned char *dat);
#endif /* APP_USER_ESP8266_TCP_H_ */
