/*
 * ESP8266_TCP_Client.c
 *
 *  Created on: 2015��6��28��
 *      Author: Administrator
 */
#include "c_types.h"
#include "osapi.h"
#include "mem.h"
#include "user_Interface.h"
#include "espconn.h"
#include "driver/uart.h"
#include "ESP8266_TCP_Client.h"



typedef enum{
teClient,
teServer
}teType;

typedef struct
{
  BOOL linkEn;
  BOOL teToff;
  uint8_t linkId;
  teType teType;
  uint8_t repeaTime;
  struct espconn *pCon;
} espConnectionType;

#define  linkConType    espConnectionType
os_timer_t checkTimer_wifistate;
os_timer_t Wifi_Init;
os_timer_t TCP_Init;
espConnectionType user_contype;
static ip_addr_t host_ip;


/*
 * ������:Inter213_Receive(void *arg, char *pdata, unsigned short len)
 * ����:���ջص�����
 */
static void ICACHE_FLASH_ATTR
Inter213_Receive(void *arg, char *pdata, unsigned short len)
{

	uart_sendString(pdata);
  return;
}
/*
 * ������:Inter213_Send_Cb(void *arg)
 * ����:���ͳɹ��Ļص�����
 */
static void ICACHE_FLASH_ATTR
Inter213_Send_Cb(void *arg)
{
	uart_sendString("\r\n Send Success \r\n");
}

/*
 * ������:Inter213_Connect_Cb(void *arg)
 * ����:���ӳɹ���Ļص�����
 */
static void ICACHE_FLASH_ATTR
Inter213_Connect_Cb(void *arg) {
  struct espconn *pespconn = (struct espconn *)arg;
  linkConType *linkTemp = (linkConType *)pespconn->reverse;

  linkTemp->linkEn = TRUE;
  linkTemp->teType = teClient;
  linkTemp->repeaTime = 0;

  uart_sendString("\r\n link Success !!\r\n");
}

/*
 * ������:Inter213_Disconnect_Cb(void *arg)
 * ����:�Ͽ����ӳɹ���Ļص�����
 */
static void ICACHE_FLASH_ATTR
Inter213_Disconnect_Cb(void *arg)
{
  struct espconn *pespconn = (struct espconn *)arg;
  linkConType *linkTemp = (linkConType *)pespconn->reverse;

  if(pespconn == NULL)  {
    return;
  }
  if(pespconn->proto.tcp != NULL)  {
    os_free(pespconn->proto.tcp);
  }
  os_free(pespconn);
  linkTemp->linkEn = FALSE;
  uart_sendString("\r\n disconnect!!! \r\n");
}
/*
 * ��������Inter213_Reconnect_Cb(void *arg, sint8 errType)
 * ����: �����ص�����
 */
static void ICACHE_FLASH_ATTR
Inter213_Reconnect_Cb(void *arg, sint8 errType) {
  struct espconn *pespconn = (struct espconn *)arg;
  linkConType *linkTemp = (linkConType *) pespconn->reverse;
  if(linkTemp->linkEn) {
    return;
  }

  if(linkTemp->teToff == TRUE) {
    linkTemp->teToff = FALSE;
    linkTemp->repeaTime = 0;
    if(pespconn->proto.tcp != NULL) {
      os_free(pespconn->proto.tcp);
    }
    os_free(pespconn);
    linkTemp->linkEn = false;
  }   else   {
    linkTemp->repeaTime++;
    if(linkTemp->repeaTime >= 1)  {
       linkTemp->repeaTime = 0;
      if(pespconn->proto.tcp != NULL)  {
        os_free(pespconn->proto.tcp);
      }
      os_free(pespconn);
      linkTemp->linkEn = false;
      return;
    }

    pespconn->proto.tcp->local_port = espconn_port();
    espconn_connect(pespconn);
  }
}
/*
 * ������:Inter213_Dns_Cb(const char *name, ip_addr_t *ipaddr, void *arg)
 * ����:dns��ѯ�ص�����
 */
LOCAL void ICACHE_FLASH_ATTR
Inter213_Dns_Cb(const char *name, ip_addr_t *ipaddr, void *arg) {
  struct espconn *pespconn = (struct espconn *) arg;
  linkConType *linkTemp = (linkConType *) pespconn->reverse;
  if(ipaddr == NULL)  {
    linkTemp->linkEn = FALSE;
    return;
  }

  if(host_ip.addr == 0 && ipaddr->addr != 0) {
    if(pespconn->type == ESPCONN_TCP)  {
      os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
      espconn_connect(pespconn);
    } else {
      os_memcpy(pespconn->proto.udp->remote_ip, &ipaddr->addr, 4);
      espconn_connect(pespconn);
    }
  }
}
/*
 * ������:void Inter213_InitTCP(char * ipAddress, int32_t port)
 * ����:����TCP����
 */
void  Inter213_InitTCP(char * ipAddress, int32_t port) //ipAddress:ip��ַ     port:�˿ں�
{
	char ipTemp[128];
	uint32_t ip = 0;
	user_contype.pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
	user_contype.pCon->state = ESPCONN_NONE;
	user_contype.linkId = 0;
	ip = ipaddr_addr(ipAddress);
	user_contype.pCon->type = ESPCONN_TCP;

	user_contype.pCon->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	user_contype.pCon->proto.tcp->local_port = espconn_port();
	user_contype.pCon->proto.tcp->remote_port = port;
	os_memcpy(user_contype.pCon->proto.tcp->remote_ip, &ip, 4);
	user_contype.pCon->reverse = &user_contype;

	espconn_regist_recvcb(user_contype.pCon, Inter213_Receive);////////
	espconn_regist_sentcb(user_contype.pCon, Inter213_Send_Cb);///////
	espconn_regist_connectcb(user_contype.pCon, Inter213_Connect_Cb);
	espconn_regist_disconcb(user_contype.pCon, Inter213_Disconnect_Cb);
	espconn_regist_reconcb(user_contype.pCon, Inter213_Reconnect_Cb);

	if((ip == 0xffffffff) && (os_memcmp(ipTemp,"255.255.255.255",16) != 0))
	{
      espconn_gethostbyname(user_contype.pCon, ipAddress, &host_ip, Inter213_Dns_Cb);
	}else{
	  espconn_connect(user_contype.pCon);
	}
}
/*
 * ������:void Inter213Net_TCP_SendData(uint8 *buf,uint16 len)
 * ����:���ݷ���
 */
void Inter213_TCP_SendData(uint8 *buf,uint16 len) //buf:����ָ��    len:����
{
	  espconn_sent(user_contype.pCon,buf,len);
}
/*
 * ������:Init_InterNet(void)
 * ����:·��������
 */
void Inter213Init(void)
{
	 struct station_config stationConf;
	 os_bzero(&stationConf, sizeof(struct station_config));
	  wifi_set_opmode(STATION_MODE);
	  //wifi_station_get_config(&stationConf);
	  os_strcpy(stationConf.ssid,"MERCURY_2784");			//ssid
	  os_strcpy(stationConf.password,"18163977851"); //password
	  wifi_station_disconnect();
	  ETS_UART_INTR_DISABLE();
	  wifi_station_set_config(&stationConf);
	  ETS_UART_INTR_ENABLE();
	  wifi_station_connect();
}
/*
 * ������:Check_WifiState(void)
 * ����: wifi��ѯ״̬ ���ӻ���δ����
 */
void Check_WifiState(void)
{
	 uint8 getState;
	 getState = wifi_station_get_connect_status();
	 if(getState == STATION_GOT_IP)//���״̬��ȷ��֤���Ѿ�����
	  {
		 uart_sendString("GET IP");
		 os_timer_disarm(&checkTimer_wifistate);
		 os_timer_arm(&TCP_Init,500,0);
	  }
}
/*
 * ������:void wifi_Conn(void)
 * ����:wifi����
 */
void wifi_Conn(void)
{
	//
	Inter213Init();
}
/*
 * ������:void TCP_Comm(void)
 * ����:TCP����
 */
void TCP_Comm(void)
{
	Inter213_InitTCP("101.200.46.138",2525);		//����TCP
}
/*
 * ������:void Esp8266_TCPClient_Time_Init(void)
 * ����:��ʼ����ʱ��
 */
void Esp8266_TCPClient_Time_Init(void) //tcp_client ��ʱ����ʼ��
{

	  os_timer_disarm(&checkTimer_wifistate);
	  os_timer_setfn(&checkTimer_wifistate,(os_timer_func_t *)Check_WifiState,NULL);
	  os_timer_arm(&checkTimer_wifistate,50,1);

	  os_timer_disarm(&Wifi_Init);
	  os_timer_setfn(&Wifi_Init,(os_timer_func_t *)wifi_Conn,NULL);
	  os_timer_arm(&Wifi_Init,50,0);

	  os_timer_disarm(&TCP_Init);
	  os_timer_setfn(&TCP_Init,(os_timer_func_t *)TCP_Comm,NULL);



}
