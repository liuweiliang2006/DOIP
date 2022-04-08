/*
 * doip_client.c
 *
 *  Created on: 2022Äê4ÔÂ7ÈÕ
 *      Author: Administrator
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "cJSON.h"
#include "mqtt_app.h"
#include "MQTTPacket.h"
#include "transport.h"

#include "lwip/opt.h"

#include "lwip/sys.h"
#include "lwip/api.h"

#include "lwip/sockets.h"
#include "doip_client.h"
#include "doip_pdu.h"
#include "ComStack_Types.h"
#include "doip_feature.h"

#define BUFFER_SIZE 1500

#define SOCK_TARGET_HOST4  "192.168.0.3"
#define LOCAL_PORT  13400

typedef struct sockaddr_s
{
    char ip[20];
    uint16_t port;
} sockaddr_t;

diagnostic_message_t *gdoip_msg = 0;
static int gsock = -1;

static sockaddr_t local_addr = {
    .ip = "192.168.0.2",
    .port = 7808,
};

static sockaddr_t remote_addr = {
    .ip = "192.168.0.3",
    .port = 13400,
};

static uint16_t src_addr = 0x0E82;
static uint16_t dst_addr = 0x0102;

static uint8_t recv_buf[BUFFER_SIZE];

static bool doip_sa_set = false;
static bool remote_unlink = false;
static struct sockaddr_in remote_address = {0};
static doip_recv_cb gRecvCb = NULL;
//static pthread_t tid_rx;



int doiphdr_create(void)
{
    if (NULL == gdoip_msg)
    {
        gdoip_msg = (diagnostic_message_t *)malloc(65535);
        if (!gdoip_msg)
        {
            printf("malloc failed!\n");
            return -1;
        }
    }

    if (gdoip_msg)
    {
        gdoip_msg->hdr.ProtocolVersion = 0x02;
        gdoip_msg->hdr.InverseProtocolVersion = 0xFD;
        gdoip_msg->hdr.PayloadType = 0x8001;
        gdoip_msg->hdr.PayloadLength = 4;
        gdoip_msg->sa[1] = htons(src_addr) >> 8;
        gdoip_msg->sa[0] = (uint8_t)htons(src_addr);
        gdoip_msg->ta[1] = htons(dst_addr) >> 8;
        gdoip_msg->ta[0] = (uint8_t)htons(dst_addr);

        doip_sa_set = true;
    }

    return 0;
}

int doip_routing_active()
{
    routing_activation_request_t rout_req;

    if (remote_unlink)
    {
        return -1;
    }

    // for first init is interrupt exit ( connect fail for no server ), so must be set the sa address info;
    if (!doip_sa_set)
    {
        doiphdr_create();
    }

    rout_req.hdr.ProtocolVersion = 0x02;
    rout_req.hdr.InverseProtocolVersion = 0xFD;
    rout_req.hdr.PayloadType = htons(0x0005);
    rout_req.hdr.PayloadLength = htonl(0x0b);
    rout_req.ActivationType = 0xE0;
    memset(&rout_req.iso_rsv, 0, sizeof(rout_req.iso_rsv));
    memset(&rout_req.oem_specific, 0, sizeof(rout_req.oem_specific));
    rout_req.SourceAddress[1] = gdoip_msg->sa[1];
    rout_req.SourceAddress[0] = gdoip_msg->sa[0];

    int ret = send(gsock, (const uint8_t *)&rout_req, sizeof(rout_req), 0);
    if (ret < 0)
    {
//        printf("send failed, errno : %d\n", get_error);
        return -1;
    }
    else
    {
//        printf("send sucess, send %d bytes!\n", ret);
        return ret;
    }
}

static void recv_loop(void)
{
    int rc = -1;
    struct sockaddr_in SenderAddr;
    unsigned int SenderAddrSize = sizeof(SenderAddr);
    int remain_len;
    int total_len;
    int offset = 0;
    PduLengthType len;
    BufReq_ReturnType result;
    PduInfoType pduInfo;

    for (;;)
    {
    	int recvLen = recv(gsock, &recv_buf[offset], BUFFER_SIZE, 0);
    	if (recvLen > 0)
    	{
            if (recvLen <= 12)
            {
                printf("message too small!\n");
                offset += recvLen;
                continue;
            }
            offset += recvLen;
            diagnostic_message_t *msg = (diagnostic_message_t *)recv_buf;
            total_len = msg->hdr.PayloadLength + 8;
            if (offset < total_len)
            {
                continue;
            }
            if (msg->hdr.PayloadType != 0x8001)
            {
//                printf("[debug], hdr.PayloadType != 0x8001\n");
                offset -= total_len;
                continue;
            }
            int diagnosticMessageLength = ntohl(msg->hdr.PayloadLength) - 4;
            if (SoAd_BufferGet(BUFFER_SIZE, &pduInfo.SduDataPtr)) {

            	result = Tdrm_StartOfReception(diagnosticMessageLength, len);

            	if (result == BUFREQ_OK && len > 0)
            	{
            		pduInfo.SduLength = diagnosticMessageLength;
            		memcpy(pduInfo.SduDataPtr,msg->userdata,diagnosticMessageLength);

    				if(len < diagnosticMessageLength)
    				{
    					PduInfoType pduInfoChunk;
    					PduLengthType lenToSend = diagnosticMessageLength;
    					/* We need to copy in smaller parts */
    					pduInfoChunk.SduDataPtr = pduInfo.SduDataPtr;

    					while(lenToSend > 0)
    					{
    						if(lenToSend >= len){
    							pduInfoChunk.SduLength = len;
    							Tdrm_CopyRxData( &pduInfoChunk);
    							lenToSend -= len;
    						}else{
    							pduInfoChunk.SduLength = lenToSend;
    							Tdrm_CopyRxData( &pduInfoChunk);
    							lenToSend = 0;
    						}
    					}
    				}else{
    					Tdrm_CopyRxData( &pduInfo);
    				}
    				Tdrm_RxIndication(0, NTFRSLT_OK);
            	}
            }

            if (gRecvCb)
            {
//                Log(false, (const uint8_t *)recv_buf, recvLen);
//                gRecvCb(msg->userdata, data_len);
            }
            offset -= total_len;
    	}
        else if (recvLen == 0)
        {
//            printf("[error], recv len = 0\n");
            continue;
        }
        else
        {
//            printf("[error], recv len < 0, errno : %d\n", get_error);
            //printf("reconnect condition, gsock: %d, remote_unlink = %d\n", gsock, remote_unlink);
            if (remote_unlink)
            {
                if (0 == gsock)
                {
                    gsock = socket(AF_INET, SOCK_STREAM, 0);
                    if (gsock < 0)
                    {
//                        printf("create socket failed in reconnect, errno : %d\n", get_error);
                        return;
                    }
                }

                rc = connect(gsock, (struct sockaddr *)&remote_address, sizeof(remote_address));
                if (0 == rc)
                {
//                    printf("reconnect... successful!\n");
                    remote_unlink = false;
                    doip_routing_active();
                }
                else
                {
                    //printf("reconnect failed, errno : %d\n", get_error);
                    // 1s reconnect
//                    usleep(1 * 1000 * 1000);
                }
            }
            else
            {
                rc = close(gsock);
                if (0 == rc)
                {
//                    printf("remote server unlink...\n");
                    gsock = 0;
                    remote_unlink = true;
                }
            }
        }
    }
}
void doip_client_rec_task( void *pvParameters )
{
    /* Casting pvParameters to void because it is unused */
    (void)pvParameters;
    TickType_t xNextWakeTime;

    /* Casting pvParameters to void because it is unused */
    (void)pvParameters;
    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();
    while(1)
    {
    	Printf("doip Client\r\n");
    	vTaskDelayUntil(&xNextWakeTime, (10/portTICK_RATE_MS));


    }
}

int doip_client_init(void)
{
//	int gsock = -1;
	int ret = -1;
	int opt = 1;
	struct sockaddr_in address;
	struct ifaddrs * ifAddrStruct=NULL;
	getifaddrs(&ifAddrStruct);
//	const struct timeval time = {
//			.tv_sec = 2,
//		};
	address.sin_family = AF_INET;
//	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(local_addr.port);
	inet_pton(AF_INET, local_addr.ip, &address.sin_addr);
	memset(&(address.sin_zero), 0, sizeof(address.sin_zero));

	gsock = socket(AF_INET, SOCK_STREAM, 0);
	if (gsock < 0)
	{
	    Printf("Socket error\n");
	    goto __exit;
	}

    remote_address.sin_family = AF_INET;
    inet_pton(AF_INET, remote_addr.ip, &remote_address.sin_addr);
    remote_address.sin_port = htons(remote_addr.port);
    ret = connect(gsock, (struct sockaddr *)&remote_address, sizeof(remote_address));

    if (ret < 0)
    {
//        printf("connect failed, errno : %d\n", get_error);
        remote_unlink = true;
        goto __exit;
    }

    doiphdr_create();

__exit:
	if (gsock < 0) closesocket(gsock);
	sys_thread_new("doip_client", doip_client_rec_task, NULL, 400, DEFAULT_THREAD_PRIO);
}

int doip_client_send(uint8_t *payload, uint32_t len)
{
	int result = 0;
    if (!payload || !gdoip_msg)
    {
        Printf("error: payload or gdoip_msg is null!");
        return -1;
    }

    uint32_t send_len = 0;
    memcpy(gdoip_msg->userdata, payload, len);
    gdoip_msg->hdr.PayloadLength += len;
    send_len = gdoip_msg->hdr.PayloadLength + 8;
    gdoip_msg->hdr.PayloadLength = htonl(gdoip_msg->hdr.PayloadLength);
    gdoip_msg->hdr.PayloadType = htons(gdoip_msg->hdr.PayloadType);

//    Log(true, (const uint8_t *)gdoip_msg, 14);

    int ret = send(gsock, (const uint8_t *)gdoip_msg, send_len, 0);
    if(ret == send_len)
    {
    	Tdrm_TxConfirmation(NTFRSLT_OK);
        Printf("send sucess, send %d bytes!\n", ret);
        gdoip_msg->hdr.PayloadLength = 4;
        gdoip_msg->hdr.PayloadType = 0x8001;
        result = 0;
    }
    else if (ret < 0)
    {
    	Tdrm_TxConfirmation(NTFRSLT_E_NOT_OK);
        Printf("send failed, errno\n");
        result = 1;
    }
    return result;

}

void doip_register_cb(doip_recv_cb cb) { gRecvCb = cb; }


static void fill_hdr(struct doip_hdr *hdr, uint16_t payload_type, uint32_t payload_length)
{
	hdr->ver = DOIP_VERSION;
	hdr->inv_ver = DOIP_VERSION ^ 0xff;
	hdr->payload_type = htons(payload_type);
	hdr->payload_length = htonl(payload_length);
}

static int check_hdr(struct doip_hdr *hdr, uint16_t payload_type, uint32_t payload_length)
{
	if (hdr->ver != (hdr->inv_ver ^ 0xFF))
		return -11;

	if (ntohs(hdr->payload_type) != payload_type)
		return -12;

	if (ntohl(hdr->payload_length) != payload_length)
		return -13;

	return 0;
}

static int do_vehicle_id_req(int fd, uint8_t *rx_buf, uint16_t *rx_len)
{
	struct timeval time = {
		.tv_sec = 3,
	};
	struct doip_hdr hdr;
	int ret;

	fill_hdr(&hdr, DOIP_PT_vehicle_id_req,0);
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time, sizeof(time));

	ret = send(fd, &hdr, sizeof(hdr), 0);
	if (ret <= 0) {
//		perror("do_vehicle_id_req: send()");
		return -errno;
	}

	while (1) {
		ret = recv(fd, rx_buf, *rx_len, 0);
		if (ret <= 0) {
//			perror("do_vehicle_id_req: recv()");
			return -errno;
		}
		if (ret < (DOIP_HDR_LEN + sizeof(struct doip_pt_vehicle_id_rsp)))
			continue;

		if (check_hdr((struct doip_hdr *)rx_buf, DOIP_PT_vehicle_id_rsp, ret - DOIP_HDR_LEN) != 0)
			continue;
		*rx_len = ret;
		break;
	}
	return 0;
}


int doipc_get_ta_by_ip(uint32_t ip, uint16_t *ta)
{
	struct sockaddr_in serv_addr;
	uint8_t rx_buf[200];
	uint16_t rx_len = (uint16_t)sizeof(rx_buf);
	struct doip_pt_vehicle_id_rsp *vid_rsp = (void*)&rx_buf[DOIP_HDR_LEN];
	int fd;
	int ret;

	if (ip == 0 || ip == 0xffffffff || !ta)
		return -1;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket()");
		return -1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DOIP_UDP_DISCOVERY_PORT);
//    serv_addr.sin_addr.s_addr = htonl(ip);
    inet_pton(AF_INET, remote_addr.ip, &serv_addr.sin_addr);

	ret = connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret == -1) {
//		perror("doipc_get_ta_by_ip: connect()");
		close(fd);
		return ret;
	}

	ret = do_vehicle_id_req(fd, rx_buf, &rx_len);
	if (ret == 0) {
		*ta = (uint16_t)vid_rsp->la[0] << 8 | vid_rsp->la[1];
	}
	close(fd);
	return ret;
}







