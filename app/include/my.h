/*
 * my_wifi.h
 *
 *  Created on: 2017Äê3ÔÂ22ÈÕ
 *      Author: yuanlifu
 */

#ifndef APP_INCLUDE_MY_H_
#define APP_INCLUDE_MY_H_
#include "user_interface.h"


#define my_recvTaskPrio        0
#define my_recvTaskQueueLen    64

#define my_procTaskPrio        1
#define my_procTaskQueueLen    1

void my_init(void);


typedef enum{
	my_statIdle,
	my_statRecving,
	my_statProcess,
	my_statIpSending,
	my_statIpSended,
	my_statIpTraning
}my_stateType;



#endif /* APP_INCLUDE_MY_H_ */
