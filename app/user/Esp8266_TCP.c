/*
 * Esp8266_TCP.c
 *
 *  Created on: 2015��6��27��
 *      Author: Administrator
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"
#include "driver/uart.h"
#include "Esp8266_TCP.h"
typedef enum
{
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
	uint8_t changType;
	uint8 remoteIp[4];
	int32_t remotePort;
	struct espconn *pCon;
}linkConType;

typedef struct
{
  BOOL linkEn;
  BOOL teToff;
  uint8_t linkId;
  teType teType;
  uint8_t repeaTime;
  struct espconn *pCon;
} espConnectionType;

linkConType pLink;
espConnectionType user_contype;
static struct espconn *pTcpServer;
/*
 * ������:void Wifi_AP_Init()
 * ����wifi_ap��ʼ��
 */
void WIFI_APInit()
{
	struct softap_config apConfig;
	/***************************ģʽ����************************************/
	 	 if(wifi_set_opmode(SOFTAP_MODE)){			//	����ΪAPģʽ

		 }else{

		 }
	/***************************������ͨ����************************************/
	 	  os_bzero(&apConfig, sizeof(struct softap_config));
		  wifi_softap_get_config(&apConfig);
		  apConfig.ssid_len=7;						//����ssid����
		  os_strcpy(apConfig.ssid,"ESP--yuanlief");			//����ssid����
		  os_strcpy(apConfig.password,"123456789");	//��������
		  apConfig.authmode =3;						//���ü���ģʽ
		  ETS_UART_INTR_DISABLE();					//�رմ����ж�
		  wifi_softap_set_config(&apConfig);		//����
		  ETS_UART_INTR_ENABLE();					//�򿪴���
}
/*
 *������:void TcpServer_Listen_Recv(void *arg, char *pdata, unsigned short len)
 *����:���ռ�������
 */
void TcpServer_Listen_Recv(void *arg, char *pdata, unsigned short len)
{
	uart_sendString(pdata);
}
/*
 * ������:void TcpServer_Listen_Recb(void *arg, sint8 errType)
 * ����:���Ӽ�������
 */
void TcpServer_Listen_recon_cb(void *arg, sint8 errType)
{
	struct espconn *pespconn = (struct espconn *)arg;
	  linkConType *linkTemp = (linkConType *)pespconn->reverse;

}
/*
 * ������:void Tcp_Server_Listen_discon_cb(void *arg)
 * ����:�����Ͽ�ʱ��������
 */
void Tcp_Server_Listen_discon_cb(void *arg)
{
	  struct espconn *pespconn = (struct espconn *) arg;
	  linkConType *linkTemp = (linkConType *) pespconn->reverse;
}
/*
 * ������:void Tcp_Server_Listen_sent_cb(void *arg)
 * ����:���ͳɹ���������
 */
void Tcp_Server_Listen_sent_cb(void *arg)
{
	  struct espconn *pespconn = (struct espconn *) arg;
	 linkConType *linkTemp = (linkConType *) pespconn->reverse;
}
/*
 * ������:void TcpServer_Listen_PCon(void *arg)
 * ����:�ֻ�����AP��������
 */
void TcpServer_Listen_PCon(void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;
	  pLink.teToff = FALSE;
	  pLink.linkId = 1;
	  pLink.teType = teServer;
	  pLink.repeaTime = 0;
	  pLink.pCon = pespconn;
	  pespconn->reverse = &pLink;
	  espconn_regist_recvcb(pespconn, TcpServer_Listen_Recv);							//ע����ռ�������
	  espconn_regist_reconcb(pespconn, TcpServer_Listen_recon_cb);						//ע�����Ӽ�������
	  espconn_regist_disconcb(pespconn, Tcp_Server_Listen_discon_cb);					//ע�������Ͽ�ʱ��������
	  espconn_regist_sentcb(pespconn, Tcp_Server_Listen_sent_cb);						//ע�ᷢ�ͳɹ���������

}
/*
 * ������:void WIFI_Server_MODE()
 * ����:���÷�����ģʽ
 */
void WIFI_ServerMode(void)
{
		espconn_tcp_set_max_con(1);											//����TCP���ӵ�������
	    pTcpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
	 	pTcpServer->type = ESPCONN_TCP;										//TCP����
	    pTcpServer->state = ESPCONN_NONE;									//״̬
	    pTcpServer->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	    pTcpServer->proto.tcp->local_port = 8888; 							//�Ͽ���
	    espconn_regist_connectcb(pTcpServer, TcpServer_Listen_PCon);
	    espconn_accept(pTcpServer);
	    espconn_regist_time(pTcpServer, 180, 0); 							//���ó�ʱ�Ͽ�ʱ�� ��λs


}
/*
 * ������:void WIFI_TCP_SendNews(unsigned char *dat)
 * ����:��TCP������������Ϣ
 */
void WIFI_TCP_SendNews(unsigned char *dat)
{
	espconn_sent(pLink.pCon,dat,os_strlen(dat));
}
