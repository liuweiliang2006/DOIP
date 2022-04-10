/*
 * doipTester.c
 *
 *  Created on: 2022Äê4ÔÂ8ÈÕ
 *      Author: Administrator
 */



#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "transport.h"

#include "lwip/opt.h"

#include "lwip/sys.h"
#include "lwip/api.h"

#include "lwip/sockets.h"
#include "lwip/def.h"
#include "doipTester.h"


typedef struct sockaddr_s
{
    char ip[20];
    uint16_t port;
} sockaddr_t;

//diagnostic_message_t *gdoip_msg = 0;
//static int gsock = -1;
static sockaddr_t local_addr = {
    .ip = "127.0.0.1",
    .port = 7808,
};
static sockaddr_t remote_addr = {
    .ip = "127.0.0.1",
    .port = 13400,
};

void doip_tester_task( void *pvParameters )
{
//    unsigned long ulReceivedValue;
//    uint16_t len;
    /* Casting pvParameters to void because it is unused */
    (void)pvParameters;
    TickType_t xNextWakeTime;

    /* Casting pvParameters to void because it is unused */
    (void)pvParameters;
    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();
    struct uds_tp *tp;
    struct sockaddr_in in_server;
    inet_pton(AF_INET, remote_addr.ip, &in_server.sin_addr.s_addr);
    tp = doipc_open(0xbeef, ntohl(in_server.sin_addr.s_addr));
//    tp = doipc_open(0xbeef, htonl(in_server.sin_addr));
    while(1)
    {
    	Printf("doip loop\r\n");
    	vTaskDelayUntil(&xNextWakeTime, (100/portTICK_RATE_MS));

//    	SoAd_MainFunction();
//    	DoIp_MainFunction();
//    	TCP_test_func();
//    	UDP_test_func();

    }
}



void doip_tester_task_init(void)
{

	  IP_SET_TYPE_VAL(dstaddr, IPADDR_TYPE_V4);
//	  ip4addr_aton(SOCK_TARGET_HOST4, ip_2_ip4(&dstaddr));
//	  SoAd_Init();
//	  DoIp_wait_time();
//	  ip4addr_aton(SOCK_TARGET_HOST4, ip_2_ip4(&dstaddr));
//    xTaskCreate( MQTT_test_task, "MQTT_test", 12*configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	  sys_thread_new("doiptester", doip_tester_task, NULL, 400, DEFAULT_THREAD_PRIO);
//    sys_thread_new("MQTT_send", MQTT_send, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
