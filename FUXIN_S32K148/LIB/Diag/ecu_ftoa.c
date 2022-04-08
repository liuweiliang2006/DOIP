/*
 * ecu_ftoa.c
 *
 *  Created on: 2021年10月12日
 *      Author: Administrator
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "tdrm.h"
#include "Com.h"
#include <string.h>
#include "com_task.h"
#include "OsekNm.h"

extern unsigned int keyLen;
extern QueueHandle_t fota_ecu_req_Queue;
vuint8 ecu_key[4];
extern unsigned char ecu_seed[5];
extern vuint8 serviceData[1024];
vuint8 tdrm_service_request_active;
vuint8 can_task_fast;

#define kSrvLenSessionControl                   2
#define kSrvIdSessionControl                    0x10u
#define kTdrmSessionExtendedNoResp              0x83u

tTdrmReturn TdrmSrvReqClearDTCIdentifier(vuint32 identifier);
tTdrmReturn TdrmSrvReqSecurityAccessReqSeed_L1(void);
tTdrmReturn TdrmSrvReqSecurityAccessSendKey_L2(vuint8 *keydata, vuint8 keylen);
tTdrmReturn TdrmSrvReqSecurityAccessReqSeed_L3(void);
tTdrmReturn TdrmSrvReqSecurityAccessSendKey_L4(vuint8 *keydata, vuint8 keylen);
unsigned int fota_seedToKey(unsigned char *seed, unsigned int length, unsigned char *key, unsigned int *retLen,unsigned int seedMask);
extern void uart_printf_data_isr(unsigned char *data,unsigned char len,char *format);
tTdrmReturn TdrmSrvReqSessionExtendedFunc(void)
{
    vuint8 data[kSrvLenSessionControl] = {kSrvIdSessionControl,kTdrmSessionExtendedNoResp};
    tdrm_service_request_active = 1;
    //return TdrmSrvReqFunc(data, kSrvLenSessionControl);
    return TdrmServiceRequest((vuint8 *)data, kSrvLenSessionControl);
}
/* RoutineControl defines */
#define kSrvLenRoutineControl                   4
#define kSrvIdRoutineControl                    0x31
tTdrmReturn TdrmSrvReqCheckProgramPrecondition(void)
{
    serviceData[kSrvIdxId]         = kSrvIdRoutineControl;
    serviceData[kSrvIdxSubParam]   = 0x01;
    serviceData[kSrvIdxSubParam+1] = 0xFF;
    serviceData[kSrvIdxSubParam+2] = 0x02;

    return TdrmServiceRequest((vuint8 *)serviceData, kSrvLenRoutineControl);
}
vuint8 ecu_data_F15A[12] = {0x19, 0x11, 0x19, 0x1, 0x2, 0x3, 0x5, 0x8, 0x13, 0x13, 0x00, 0x01};
/* WriteDataByIdentifier */
#define kSrvLenWriteDataByIdentifier            15
#define kSrvIdWriteDataByIdentifier             0x2Eu
tTdrmReturn WriteDataByIdentifier(vuint16 identifier, vuint8 *databuf, vuint8 datalen)
{
    serviceData[kSrvIdxId]         = kSrvIdWriteDataByIdentifier;
    serviceData[kSrvIdxSubParam]   = (identifier >> 8)&0xFF;
    serviceData[kSrvIdxSubParam+1] = identifier & 0xFF;
    memcpy(&serviceData[3], databuf, datalen);

    return TdrmServiceRequest((vuint8 *)serviceData, datalen+3);
}
static tTdrmReturn TdrmSrvReqWrite0xF15AData(void)
{
    return WriteDataByIdentifier(0xF15A, ecu_data_F15A,9);
}
vuint8 cur_image;
/* RoutineControl defines */
#define kSrvLenRoutineControl                   4
#define kSrvIdRoutineControl                    0x31
/* Erase Memory */
#define kSrvLenEraseMemory                      13
static void Vuint32To4Vuint8(vuint8* buffer, vuint32 data)
{
    for(vuint8 i=0; i<4; i++)
    {
        buffer[i] = (data>>((3-i)*8)) & 0xFF;
    }
}
tTdrmReturn TdrmSrvReqEraseMemory(vuint32 MemoryAddress, vuint32 MemorySize)
{
    serviceData[kSrvIdxId]         = kSrvIdRoutineControl;
    serviceData[kSrvIdxSubParam]   = 0x01;
    serviceData[kSrvIdxSubParam+1] = 0xFF;
    serviceData[kSrvIdxSubParam+2] = 0x00;
    serviceData[kSrvIdxSubParam+3] = 0x44;
    Vuint32To4Vuint8(&serviceData[5], MemoryAddress);
    Vuint32To4Vuint8(&serviceData[9], MemorySize);

    return TdrmServiceRequest((vuint8 *)serviceData, kSrvLenEraseMemory);
}
static tTdrmReturn TdrmSrvReqUserEraseMemory(void)
{
    cur_image++;
    //写入地址和长度
    return TdrmSrvReqEraseMemory(0xC06800, 0x49800);
}
/* RequestTransferExit */
#define kSrvLenRequestTransferExit              1
#define kSrvIdRequestTransferExit               0x37
tTdrmReturn TdrmSrvReqRequestTransferExit(void)
{
    serviceData[kSrvIdxId]         = kSrvIdRequestTransferExit;

    return TdrmServiceRequest((vuint8 *)serviceData, kSrvLenRequestTransferExit);
}
tTdrmReturn TdrmSrvReqCheckProgrammingDependence(void)
{
    serviceData[kSrvIdxId]         = kSrvIdRoutineControl;
    serviceData[kSrvIdxSubParam]   = 0x01;
    serviceData[kSrvIdxSubParam+1] = 0xFF;
    serviceData[kSrvIdxSubParam+2] = 0x01;

    return TdrmServiceRequest((vuint8 *)serviceData, kSrvLenRoutineControl);
}
tTdrmReturn TdrmSrvReqCheckProgrammingIntegrated(unsigned int data)
{
    serviceData[kSrvIdxId]         = kSrvIdRoutineControl;
    serviceData[kSrvIdxSubParam]   = 0x01;
    serviceData[kSrvIdxSubParam+1] = 0xF0;
    serviceData[kSrvIdxSubParam+2] = 0x01;
    Vuint32To4Vuint8(&serviceData[4], data);

    return TdrmServiceRequest((vuint8 *)serviceData, kSrvLenRoutineControl+4);
}
/* EcuReset defines */
#define kSrvLenEcuReset                         2
#define kSrvIdEcuReset                          0x11u
#define kSrvEcuHardReset                        0x01u
tTdrmReturn TdrmSrvReqEcuHardReset(void)
{
    serviceData[kSrvIdxId]         = kSrvIdEcuReset;
    serviceData[kSrvIdxSubParam]   = kSrvEcuHardReset;

    return TdrmServiceRequest((vuint8 *)serviceData, kSrvLenEcuReset);
}
tTdrmReturn TdrmSrvReqRequestDownload(vuint32 MemoryAddress, vuint32 MemorySize)
{
    serviceData[kSrvIdxId]         = 0x34;
    serviceData[kSrvIdxSubParam]   = 0x00;
    serviceData[2] = 0x44;
    Vuint32To4Vuint8(&serviceData[3], MemoryAddress);
    Vuint32To4Vuint8(&serviceData[7], MemorySize);

    return TdrmServiceRequest((vuint8 *)serviceData, 11);
}
tTdrmReturn TdrmSrvReqTransferData(vuint8 *databuf, vuint16 datalen)
{
    serviceData[kSrvIdxId]         = 0x36;

    memcpy(&serviceData[kSrvIdxSubParam], databuf, datalen);
    return TdrmServiceRequest((vuint8 *)serviceData, datalen+1);
}

vuint8 fotaCount;
vuint8 fotaCountIndex;
extern vuint8 start_fotaFlag;
extern vuint8 otadataBuffer[];
vuint8 otadataBufferTEMP[197];
extern uint8_t PfalshBuffer[];
uint8_t read_Pflsh_ota(uint16_t index,uint32_t base_address);
unsigned long main_crc_ota(unsigned char *input,unsigned int len,unsigned long crc);
unsigned long main_crc_ota_data=0xffffffff;

void TransferProgramData(vuint8 index)
{
	otadataBuffer[0]= index+1;
	read_Pflsh_ota(index,0x30000);
	vTaskDelay( (1/portTICK_RATE_MS));
	main_crc_ota_data = main_crc_ota(&otadataBuffer[1],0x400,main_crc_ota_data);
	//otadataBufferTEMP[2]=0xe0;
	uart_printf_data_all(otadataBuffer,8,",Index");
	while(1)
	{
		if(kTdrmReturnOk == TdrmSrvReqTransferData(otadataBuffer,0x401))
		{
			break;
		}
		else
		{

		}
	}
}
uint32_t fotaCountIndexApp1=0;
void TransferProgramData_app1(vuint8 index)
{
	otadataBuffer[0]= index+1;
	read_Pflsh_ota(index,0x40000);
	vTaskDelay( (1/portTICK_RATE_MS));
	main_crc_ota_data = main_crc_ota(&otadataBuffer[1],0x400,main_crc_ota_data);
	//otadataBufferTEMP[2]=0xe0;
	uart_printf_data_all(otadataBuffer,8,",Index");
	while(1)
	{
		if(kTdrmReturnOk == TdrmSrvReqTransferData(otadataBuffer,0x401))
		{
			break;
		}
		else
		{

		}
	}
}
uint32_t fotaCountIndexApp2=0;
void TransferProgramData_app2(vuint8 index)
{
	otadataBuffer[0]= index+1;
	read_Pflsh_ota(index,0x50000);
	vTaskDelay( (1/portTICK_RATE_MS));
	main_crc_ota_data = main_crc_ota(&otadataBuffer[1],0x400,main_crc_ota_data);
	//otadataBufferTEMP[2]=0xe0;
	uart_printf_data_all(otadataBuffer,8,",Index");
	while(1)
	{
		if(kTdrmReturnOk == TdrmSrvReqTransferData(otadataBuffer,0x401))
		{
			break;
		}
		else
		{

		}
	}
}
void fota_process_ecu_bak(void)
{
	vuint8 ulReceivedValue;
	vuint8 i;
	 TickType_t xNextWakeTime;
	switch(fotaCount)
	{
		case 9:
			Com_TxStop();
			break;
		case 10:
			TdrmSetTpChannel(1);
			TdrmSrvReqDiagSessionControl(1);

			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota1\r\n");
			break;
		case 12:
			TdrmSetTpChannel(1);
			TdrmSrvReqDiagSessionControl(3);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota2\r\n");
			break;
		case 13:
			TdrmSetTpChannel(1);
			TdrmSrvReqCheckProgramPrecondition();
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota3\r\n");
			break;
		case 14:
			   TdrmSetTpChannel(2);
			   TdrmSrvReqDtcSettingOnFunc();
			   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			   uart_log_printf_isr("fota4\r\n");
			break;
		case 15:
			   TdrmSetTpChannel(2);
			   TdrmSrvReqEnRxAndDisTxFunc();
			   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			   uart_log_printf_isr("fota5\r\n");
			break;
		case 16:
			TdrmSetTpChannel(1);
			TdrmSrvReqDiagSessionControl(2);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota6\r\n");
			break;
		case 148:
#if 0
			   TdrmSetTpChannel(1);
			   TdrmSrvReqSecurityAccessReqSeed_L1();
			   uart_log_printf_isr("ReqSeed\r\n");
			   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			   uart_log_printf_isr("ReqKey\r\n");
			   fota_seedToKey(ecu_seed,5,ecu_key,&keyLen,0xEBCAFE17);
			   uart_printf_data_isr(ecu_key,4,",key");
			   TdrmSrvReqSecurityAccessSendKey_L2(ecu_key,4);
#endif
			   ecu_seed[0]=0; ecu_seed[1]=0; ecu_seed[2]=0; ecu_seed[3]=0; ecu_seed[4]=0;
#if 1
			   TdrmSetTpChannel(1);
			   TdrmSrvReqSecurityAccessReqSeed_L3();
			   uart_log_printf_isr("ReqSeed_3\r\n");
			   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			   uart_log_printf_isr("ReqKey_3\r\n");
			   fota_seedToKey(ecu_seed,5,ecu_key,&keyLen,0x89B34C6B);//0xE75BF4E7);
			   uart_printf_data_isr(ecu_key,4,",key_3");
			   TdrmSrvReqSecurityAccessSendKey_L4(ecu_key,4);
			   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
#endif
			break;
		case 149:
			//TdrmSetTpChannel(1);
			//TdrmSrvReqWrite0xF15AData();

			//xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			//uart_log_printf_isr("Write0xF15A\r\n");
			break;
		case 150:
			//下载驱动
			TdrmSetTpChannel(1);
			TdrmSrvReqRequestDownload(0x00E00000,0x000007f0);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			//uart_log_printf_isr("Dow\r\n");//otadataBuffer

			for(i=0;i<6;i++)
			{
				TransferProgramData(i);
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				vTaskDelayUntil(&xNextWakeTime, (10/portTICK_RATE_MS));
			}


			break;
		case 151:
			//uart_printf_data_all(otadataBuffer,10,",151");
			//TdrmSetTpChannel(1);
			//TransferProgramData();
			//xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 152:
			TdrmSetTpChannel(1);
			TdrmSrvReqUserEraseMemory();
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			//下载程序升级
			break;
		case 153:
			//下载结束
			TdrmSetTpChannel(1);
			TdrmSrvReqRequestTransferExit();
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 154:
			TdrmSetTpChannel(1);
			TdrmSrvReqCheckProgrammingIntegrated(0x0);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 155:
			TdrmSetTpChannel(1);
			TdrmSrvReqCheckProgrammingDependence();
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 156:
			TdrmSetTpChannel(1);
			TdrmSrvReqEcuHardReset();
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 157:
			vTaskDelayUntil(&xNextWakeTime, (1000/portTICK_RATE_MS));
			break;
		case 158:
			TdrmSetTpChannel(2);
			TdrmSrvReqDiagSessionControl(1);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota17\r\n");
			break;
		case 159:
			TdrmSetTpChannel(2);
			TdrmSrvReqClearDTCIdentifier(0xffffff);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota18\r\n");
			break;
		case 160:
			TdrmSetTpChannel(1);
			//();
			break;
		case 161:
			TdrmSetTpChannel(1);
			//();
			start_fotaFlag =0;
			break;
		default:
			break;
	}
	fotaCount ++;
	if(fotaCount >=250)
	{
		fotaCount =249;
	}
}
vuint8 get_value_active(void);
vuint32 getActive;
extern uint8_t temptestdata[20];
void fota_process_ecu(void)
{
	vuint8 ulReceivedValue;
	vuint32 i;
	 TickType_t xNextWakeTime;
	 Com_TxStop();
	 //CanNm_Stop(0);
	 vTaskDelay((100/portTICK_RATE_MS));



		TdrmSetTpChannel(1);
		TdrmSrvReqDiagSessionControl(1);
		xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
		uart_log_printf_isr("DiagSession1\r\n");
		vTaskDelay((100/portTICK_RATE_MS));

		TdrmSetTpChannel(1);
		TdrmSrvReqDiagSessionControl(3);
		xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
		uart_log_printf_isr("DiagSession2\r\n");
		vTaskDelay((10/portTICK_RATE_MS));

		TdrmSetTpChannel(1);
		TdrmSrvReqCheckProgramPrecondition();
		xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
		uart_log_printf_isr("ProgramPrecondition\r\n");

		   TdrmSetTpChannel(2);
		   TdrmSrvReqDtcSettingOnFunc();
		   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
		   uart_log_printf_isr("fota4\r\n");

		   TdrmSetTpChannel(2);
		   TdrmSrvReqEnRxAndDisTxFunc();
		   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
		   uart_log_printf_isr("fota5\r\n");

			TdrmSetTpChannel(1);
			TdrmSrvReqDiagSessionControl(2);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota6\r\n");

			ecu_seed[0]=0; ecu_seed[1]=0; ecu_seed[2]=0; ecu_seed[3]=0; ecu_seed[4]=0;
			vTaskDelay((1000/portTICK_RATE_MS));
			   TdrmSetTpChannel(1);
			   TdrmSrvReqSecurityAccessReqSeed_L3();
			   uart_log_printf_isr("ReqSeed_3\r\n");
			   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			   uart_log_printf_isr("ReqKey_3\r\n");
			   fota_seedToKey(ecu_seed,5,ecu_key,&keyLen,0x89B34C6B);//0xE75BF4E7);
			   uart_printf_data_isr(ecu_key,4,",key_3");
			   TdrmSrvReqSecurityAccessSendKey_L4(ecu_key,4);
			   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );

			   TdrmInitPowerOn(1);
			   vTaskDelay((10/portTICK_RATE_MS));
				//下载驱动
				TdrmSetTpChannel(1);
				TdrmSrvReqRequestDownload(0x00C50000,0x00000800);
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				//uart_log_printf_isr("Dow\r\n");//otadataBuffer
				for(i=0;i<2;i++)
				{
					TransferProgramData(i);
					xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
					//getActive = get_value_active();
					uart_log_printf_isr("Dow\r\n");
					vTaskDelay((10/portTICK_RATE_MS));
					//getActive = get_value_active();
				}

				main_crc_ota_data^= 0xFFFFFFFF;
				temptestdata[0]=main_crc_ota_data>>24;
				temptestdata[1]=main_crc_ota_data>>16;
				temptestdata[2]=main_crc_ota_data>>8;
				temptestdata[3]=main_crc_ota_data;
				uart_printf_data_all(&temptestdata[0],4,",mcu_crc");
				TdrmSetTpChannel(1);
				TdrmSrvReqRequestTransferExit();
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("TransferExit1\r\n");

				TdrmSetTpChannel(1);
				TdrmSrvReqCheckProgrammingIntegrated(main_crc_ota_data);
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("Integrated1\r\n");
				TdrmSetTpChannel(1);
				TdrmSrvReqWrite0xF15AData();
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("Write0xF15A\r\n");

				TdrmSetTpChannel(1);
				TdrmSrvReqUserEraseMemory();
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("EraseMemory\r\n");

				TdrmSetTpChannel(1);
				TdrmSrvReqRequestDownload(0x00C06800,0x8800);
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );

				main_crc_ota_data = 0xFFFFFFFF;
				for(i=0;i<34;i++)
				{
					TransferProgramData_app1(i);
					xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
					getActive++;
					//vTaskDelayUntil(&xNextWakeTime, (1/portTICK_RATE_MS));
				}

				TdrmSetTpChannel(1);
				TdrmSrvReqRequestTransferExit();
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("TransferExit2\r\n");

				TdrmSetTpChannel(1);
				TdrmSrvReqRequestDownload(0x00C10000,0x40000);
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				for(i=0;i<256;i++)
				{
					TransferProgramData_app2(i);
					xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
					//vTaskDelayUntil(&xNextWakeTime, (1/portTICK_RATE_MS));
				}
				TdrmSetTpChannel(1);
				TdrmSrvReqRequestTransferExit();
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("TransferExit3\r\n");
				main_crc_ota_data^= 0xFFFFFFFF;
				TdrmSetTpChannel(1);
				TdrmSrvReqCheckProgrammingIntegrated(main_crc_ota_data);
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("Integrated2\r\n");

				TdrmSetTpChannel(1);
				TdrmSrvReqCheckProgrammingDependence();
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("Dependence\r\n");

				TdrmInitPowerOn(0);
				vTaskDelay((10/portTICK_RATE_MS));
				TdrmSetTpChannel(1);
				TdrmSrvReqEcuHardReset();
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("HardReset\r\n");
				vTaskDelay((1000/portTICK_RATE_MS));

				TdrmSetTpChannel(1);
				TdrmSrvReqDiagSessionControl(1);
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("DiagSession1\r\n");
				vTaskDelay((10/portTICK_RATE_MS));
				TdrmSetTpChannel(1);
				TdrmSrvReqClearDTCIdentifier(0xffffff);
				xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
				uart_log_printf_isr("ClearDTC\r\n");
				vTaskDelay((0xffffffff/portTICK_RATE_MS));


				start_fotaFlag =0;
#if 0
	switch(fotaCount)
	{
		case 9:

			break;
		case 10:

			break;
		case 12:

			break;
		case 13:

			break;
		case 14:

			break;
		case 15:

			break;
		case 16:

			break;
		case 148:
#if 0
			   TdrmSetTpChannel(1);
			   TdrmSrvReqSecurityAccessReqSeed_L1();
			   uart_log_printf_isr("ReqSeed\r\n");
			   xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			   uart_log_printf_isr("ReqKey\r\n");
			   fota_seedToKey(ecu_seed,5,ecu_key,&keyLen,0xEBCAFE17);
			   uart_printf_data_isr(ecu_key,4,",key");
			   TdrmSrvReqSecurityAccessSendKey_L2(ecu_key,4);
#endif
			   ecu_seed[0]=0; ecu_seed[1]=0; ecu_seed[2]=0; ecu_seed[3]=0; ecu_seed[4]=0;
#if 1

#endif
			break;
		case 149:
			//TdrmSetTpChannel(1);
			//TdrmSrvReqWrite0xF15AData();

			//xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			//uart_log_printf_isr("Write0xF15A\r\n");
			break;
		case 150:



			break;
		case 151:
			//uart_printf_data_all(otadataBuffer,10,",151");
			//TdrmSetTpChannel(1);
			//TransferProgramData();
			//xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 152:

			//下载程序升级
			break;
		case 153:
			//下载结束
			TdrmSetTpChannel(1);
			TdrmSrvReqRequestTransferExit();
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 154:

			break;
		case 155:
			TdrmSetTpChannel(1);
			TdrmSrvReqCheckProgrammingDependence();
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 156:
			TdrmSetTpChannel(1);
			TdrmSrvReqEcuHardReset();
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			break;
		case 157:
			vTaskDelayUntil(&xNextWakeTime, (1000/portTICK_RATE_MS));
			break;
		case 158:
			TdrmSetTpChannel(2);
			TdrmSrvReqDiagSessionControl(1);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota17\r\n");
			break;
		case 159:
			TdrmSetTpChannel(2);
			TdrmSrvReqClearDTCIdentifier(0xffffff);
			xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
			uart_log_printf_isr("fota18\r\n");
			break;
		case 160:
			TdrmSetTpChannel(1);
			//();
			break;
		case 161:
			TdrmSetTpChannel(1);
			break;
		default:
			break;
	}
#endif
}
