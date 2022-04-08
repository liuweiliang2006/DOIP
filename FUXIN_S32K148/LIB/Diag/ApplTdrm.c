// ApplTdrm.c : Defines the entry point for the console application.
//
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "tdrm.h"
#include <string.h>

static const vuint8 testBuf[] = { 0x10, 0x02 };

/* Static data buffer used for tranmission of service requests */
vuint8 serviceData[1030];

/* Vars for RequestUpload */
vuint8 seqCnt;
vuint8 retryCnt;
vuint16 uploadTotalLen;
vuint16 uploadTransferDataLen;
vuint32 uploadSourceAddress;
vuint16 uploadRemainingLen;
vuint8 *uploadCurrentPtr;
vuint8 *uploadDstDataPtr;

vuint8 uploadLocked;


/* Test counter */
vuint16 testCnt;

/* testStates */
vuint8 testStep;


tTdrmReturn TdrmSrvReqDiagSessionControl(vuint8 session_type);
tTdrmReturn TdrmSrvReqReadDataByIdentifier(vuint16 identifier);
tTdrmReturn TdrmSrvRequestUpload(vuint8 *dstDataPtr, vuint16 uploadLen, vuint32 sourceAddress);
tTdrmReturn TdrmSrvReqDTCIdentifier(vuint16 identifier);
tTdrmReturn TdrmSrvReqClearDTCIdentifier(vuint32 identifier);
tTdrmReturn TdrmSrvReqSecurityAccessReqSeed_L1(void);
tTdrmReturn TdrmSrvReqSecurityAccessSendKey_L2(vuint8 *keydata, vuint8 keylen);
tTdrmReturn TdrmSrvReqSecurityAccessReqSeed_L3(void);
tTdrmReturn TdrmSrvReqSecurityAccessSendKey_L4(vuint8 *keydata, vuint8 keylen);


static vuint32 GetInteger( vuint8 count, const vuint8* buffer );

/* 10ms cycle call from BRS for test purposes */
vuint8 tdrmCount;
vuint8 seedFlag;

unsigned int keyLen;
extern QueueHandle_t fota_ecu_req_Queue;//
unsigned char ecu_seed[5];
extern void uart_printf_data_isr(unsigned char *data,unsigned char len,char *format);
unsigned int fota_seedToKey(unsigned char *seed, unsigned int length, unsigned char *key, unsigned int *retLen,unsigned int seedMask)
{
    unsigned int i;
    union
    {
        unsigned char byte[4];
        unsigned int wort;
    } seedlokal;
    const unsigned int mask = seedMask;//0xEBCAFE17;
/* The array for SEED starts with [1], the array for KEY starts with [0] */
/* seed contains the SEED from the ECU */
/* length contains the number of bytes of seed */
/* key contains the KEY to send to ECU */
/* retLen contains the number of bytes to send to ECU as key */
	if (seed[1] == 0 && seed[2] == 0)
	*retLen = 0;
	else
	{
		seedlokal.wort = ((unsigned int)seed[1] << 24) + ((unsigned int)seed[2] << 16) +((unsigned int)seed[3] << 8) + (unsigned int)seed[4];
		for (i=0; i<35; i++)
		{
			if (seedlokal.wort & 0x80000000)
			{
				seedlokal.wort = seedlokal.wort << 1;
				seedlokal.wort = seedlokal.wort ^ mask;
			}
			else
			{
				seedlokal.wort = seedlokal.wort << 1;
			}
		}
		for (i=0; i<4; i++)
		{
			key[3-i] = seedlokal.byte[i];
		}
		*retLen = length - 1;
	}
	return 1;

}
vuint8 recevie_updata[256];
extern QueueHandle_t updateDataQueue;
void uart_printf_data_all(uint8_t *data,uint8_t len,char *format);
void ApplTdrmTest()
{

   if ((testCnt--) == 0)
   {
	   tdrmCount ++;
	   if(tdrmCount > 20)
	   {
		   tdrmCount =20;
	   }
	   if(tdrmCount == 2)
	   {
			TdrmSetTpChannel(1);
			TdrmSrvReqDiagSessionControl(1);
			uart_log_printf_isr("apptdrm2\r\n");
	   }
	   if(tdrmCount == 3)
	   {
			TdrmSetTpChannel(1);
			TdrmSrvReqReadDataByIdentifier(0xF192);
			uart_log_printf_isr("apptdrm3\r\n");
	   }
	   if(tdrmCount == 4)
	   {
			TdrmSetTpChannel(1);
			TdrmSrvReqDTCIdentifier(0x0a55);
			uart_log_printf_isr("apptdrm4\r\n");
	   }
	   if(tdrmCount == 5)
	   {
		   TdrmSetTpChannel(1);
	       TdrmSrvReqClearDTCIdentifier(0xffffff);
	       uart_log_printf_isr("apptdrm5\r\n");
	   }
	   if(tdrmCount == 6)
	   {
			TdrmSetTpChannel(1);
			TdrmSrvReqDiagSessionControl(3);
			uart_log_printf_isr("apptdrm6\r\n");
	   }
	   if(tdrmCount == 7)
	   {
			TdrmSetTpChannel(1);
			TdrmSrvReqDTCIdentifier(0x01FF);
			uart_log_printf_isr("apptdrm7\r\n");
			//TdrmSetTpChannel(1);
			//TdrmSrvReqDiagSessionControl(2);
			//uart_log_printf_isr("apptdrm7\r\n");
			//TdrmSrvReqSecurityAccessReqSeed_L1();
			//seedFlag = 1;
	   }
	   if(tdrmCount == 8)
	   {
			TdrmSetTpChannel(1);
			TdrmSrvReqDTCIdentifier(0x02FF);
			uart_log_printf_isr("apptdrm8\r\n");

	   }
	   if(tdrmCount == 9)
	   {
		   	   ;
	   }
	   if(tdrmCount == 11)
	   {
		   TdrmSetTpChannel(1);
		   TdrmSrvReqSecurityAccessReqSeed_L3();
		   uart_log_printf_isr("ReqSeed_3\r\n");
		  // xQueueReceive( fota_ecu_req_Queue, &ulReceivedValue, portMAX_DELAY );
		  // uart_log_printf_isr("ReqKey_3\r\n");
		//   fota_seedToKey(ecu_seed,5,ecu_key,&keyLen,0xE75BF4E7);
		 //  uart_printf_data_isr(ecu_key,4,",key_3");
		  // TdrmSrvReqSecurityAccessSendKey_L4(ecu_key,4);
	   }
	   if(tdrmCount == 12)
	   {
		   TdrmSetTpChannel(2);
		   TdrmSrvReqDtcSettingOnFunc();
		   uart_log_printf_isr("ReqDtcSetting\r\n");
	   }
	   if(tdrmCount == 13)
	   {
		   TdrmSetTpChannel(2);
		   TdrmSrvReqEnRxAndDisTxFunc();
		   uart_log_printf_isr("ReqEnRxAndDisTx\r\n");
	   }
	   if(tdrmCount == 14)
	   {
			TdrmSetTpChannel(1);
			TdrmSrvReqDiagSessionControl(1);
			uart_log_printf_isr("apptdrm13\r\n");
	   }
	   if(tdrmCount > 14)
	   {
		   //xQueueReceive( updateDataQueue, recevie_updata, portMAX_DELAY );
		   //uart_printf_data_all(updateDataQueue,10,",ota qf3");
	   }


      testCnt = 500;   // Wait for another 10 sec. */
   }
}
void receive_fota(void)
{
	int i;
	for(i=0;i<16;i++)
	{
		recevie_updata[i]=0;
	}
	  // if(xQueueReceive( updateDataQueue, recevie_updata, portMAX_DELAY ))
	  //  uart_printf_data_all(&recevie_updata[10],10,",otaD");
}


/******************************************************************************
* Name         :  GetInteger - 14229 specific
* Called by    :  ----
* Preconditions:  None
* Parameters   :  - Number of bytes to extract from array
*                 - Pointer to source data array
* Return code  :  Extracted integer number
* Description  :  This function extracts 'count' bytes from 'buffer' and
*                 converts it into an integer
******************************************************************************/
static vuint32 GetInteger( vuint8 count, const vuint8* buffer )
{
   vuint32 num = 0;
   vuint8  index = 0;

   while (count > 0)
   {
      num <<= 8;
      num |= (vuint32)buffer[index];
      index++;
      count--;
   }

   return num;
}

/******************************************************************************
* Name         :  TdrmSrvReqDiagSessionControl
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  Request to transmit a service function
******************************************************************************/
tTdrmReturn TdrmSrvReqDiagSessionControl(vuint8 session_type)
{
   tTdrmReturn retValue;


   /* Setup transmit buffer */
   serviceData[kSrvIdxId]         = kSrvIdSessionControl;
   serviceData[kSrvIdxSubParam]   = session_type;

   /* Initiate transmission */
   retValue = TdrmServiceRequest((vuint8 *)serviceData, kSrvLenSessionControl);

   return retValue;
}

/******************************************************************************
* Name         :  TdrmSrvReqReadDataByIdentifier
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  Request to transmit a service function
******************************************************************************/
tTdrmReturn TdrmSrvReqReadDataByIdentifier(vuint16 identifier)
{
   tTdrmReturn retValue;


   /* Setup transmit buffer */
   serviceData[kSrvIdxId]         = kSrvIdReadDataByIdentifier;
   serviceData[kSrvIdxSubParam]   = (vuint8)((identifier >> 8) & 0xFFu);  /* hi-byte */
   serviceData[kSrvIdxSubParam+1] = (vuint8)((identifier     ) & 0xFFu);  /* lo-byte */

   /* Initiate transmission */
   retValue = TdrmServiceRequest((vuint8 *)serviceData, kSrvLenReadDataByIdentifier);

   return retValue;
}

tTdrmReturn TdrmSrvReqDTCIdentifier(vuint16 identifier)
{
   tTdrmReturn retValue;


   /* Setup transmit buffer */
   serviceData[kSrvIdxId]         = 0x19;
   serviceData[kSrvIdxSubParam]   = (vuint8)((identifier>>8 ) & 0xFFu);  /* hi-byte */
   serviceData[kSrvIdxSubParam+1] = (vuint8)((identifier     ) & 0xFFu);  /* lo-byte */

   /* Initiate transmission */
   retValue = TdrmServiceRequest((vuint8 *)serviceData, 2);

   return retValue;
}

tTdrmReturn TdrmSrvReqClearDTCIdentifier(vuint32 identifier)
{
   tTdrmReturn retValue;


   /* Setup transmit buffer */
   serviceData[kSrvIdxId]         = 0x14;
   serviceData[kSrvIdxSubParam]   = (vuint8)((identifier >>16) & 0xFFu);  /* hi-byte */
   serviceData[kSrvIdxSubParam+1] = (vuint8)((identifier >>8    ) & 0xFFu);  /* lo-byte */
   serviceData[kSrvIdxSubParam+2] = (vuint8)((identifier    ) & 0xFFu);

   /* Initiate transmission */
   retValue = TdrmServiceRequest((vuint8 *)serviceData, 4);

   return retValue;
}
tTdrmReturn TdrmSrvReqSecurityAccessReqSeed_L1(void)
{
    serviceData[kSrvIdxId]         = 0x27;
    serviceData[kSrvIdxSubParam]   = 0x01;

    return TdrmServiceRequest((vuint8 *)serviceData, 0x02);
}
tTdrmReturn TdrmSrvReqSecurityAccessSendKey_L2(vuint8 *keydata, vuint8 keylen)
{
    serviceData[kSrvIdxId]         = 0x27;
    serviceData[kSrvIdxSubParam]   = 0x02;

    memcpy(&serviceData[2], keydata, keylen);

    return TdrmServiceRequest((vuint8 *)serviceData, keylen+2);
}
tTdrmReturn TdrmSrvReqSecurityAccessReqSeed_L3(void)
{
    serviceData[kSrvIdxId]         = 0x27;
    serviceData[kSrvIdxSubParam]   = 0x03;

    return TdrmServiceRequest((vuint8 *)serviceData, 0x02);
}
tTdrmReturn TdrmSrvReqSecurityAccessSendKey_L4(vuint8 *keydata, vuint8 keylen)
{
    serviceData[kSrvIdxId]         = 0x27;
    serviceData[kSrvIdxSubParam]   = 0x04;

    memcpy(&serviceData[2], keydata, keylen);

    return TdrmServiceRequest((vuint8 *)serviceData, keylen+2);
}
tTdrmReturn TdrmSrvReqDtcSettingOnFunc(void)
{
    serviceData[kSrvIdxId]         = 0x85;
    serviceData[kSrvIdxSubParam]   = 0x02;

    return TdrmServiceRequest((vuint8 *)serviceData, 2);
}
tTdrmReturn TdrmSrvReqEnRxAndDisTxFunc(void)
{
    serviceData[kSrvIdxId]         = 0x28;
    serviceData[kSrvIdxSubParam]   = 0x03;
    serviceData[2]   = 0x03;
    return TdrmServiceRequest((vuint8 *)serviceData, 3);
}

/******************************************************************************
* Name         :  TdrmSrvRequestUpload
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  Request to transmit a service function
******************************************************************************/
tTdrmReturn TdrmSrvRequestUpload(vuint8 *dstDataPtr, vuint16 uploadLen, vuint32 sourceAddress)
{
   if (uploadLocked!=0) return kTdrmReturnBusy;

   seqCnt = 1;
   uploadTotalLen = uploadLen;
   uploadRemainingLen = uploadLen;
   uploadDstDataPtr = uploadCurrentPtr = dstDataPtr;
   retryCnt = 3;
   uploadSourceAddress = sourceAddress;

   uploadLocked = 1;
   
   /* Request upload */
   serviceData[0]  = 0x35;
   serviceData[1]  = 0x00;
   serviceData[2]  = 0x24;
   serviceData[3]  = (vuint8)((uploadSourceAddress >> 24) & 0xff);
   serviceData[4]  = (vuint8)((uploadSourceAddress >> 16) & 0xff);
   serviceData[5]  = (vuint8)((uploadSourceAddress >>  8) & 0xff);
   serviceData[6]  = (vuint8)((uploadSourceAddress      ) & 0xff);
   serviceData[7]  = (vuint8)((uploadLen >>  8) & 0xff);
   serviceData[8] =  (vuint8)((uploadLen      ) & 0xff);

   if (TdrmServiceRequest(serviceData, 9)==kTdrmReturnOk)
   {
      uploadLocked = 1;
      return kTdrmReturnOk;
   }

   return kTdrmReturnFailed;
}


/*=============================================================*/
/*   TDRM user callbacks                                       */
/*=============================================================*/

/******************************************************************************
* Name         :  ApplTdrmInit
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  None
* Return code  :  None
* Description  :  Indicates, that TdrmInit has been called by the application.
******************************************************************************/
void ApplTdrmInit(void)
{
   testCnt = 0x1000;   // Wait 10 sec.
   testStep = 0;
}



/******************************************************************************
* Name         :  ApplTdrmServicePosResponse
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  dataBuffer: Pointer to a TDRM internal data buffer
*                 dataLength: Length of received data that is valid in dataBuffer.
* Return code  :  None
* Description  :  Indicate a positive response from a module
******************************************************************************/
unsigned char print_diag[128];
unsigned int print_diag_len;

void ApplTdrmServicePosResponse(vuint8 *dataBuffer, vuint16 dataLength)
{
   vuint32 reqUploadLen;
   print_diag_len = dataLength;
   vuint8 ulValueToSend;
   int i;
   if(print_diag_len >128)
   {
	   print_diag_len = 128;
   }
   for(i=0;i<print_diag_len;i++)
   {
	   print_diag[1+i] = dataBuffer[i];
   }
   print_diag[0] = print_diag_len;
   //uart_printf_data_all(print_diag,10,"diag");
   uart_printf_data_isr(print_diag,10,",diag");
   if(seedFlag == 1)
   {
	   seedFlag =0;
   }
   if(dataBuffer[0] != 0x7e)
	   xQueueSendToBackFromISR(fota_ecu_req_Queue, &ulValueToSend, 0);

   switch(dataBuffer[0])
   {
      case 0x67:
    	  for(i=0;i<5;i++)
    	  {
    		  ecu_seed[i]= dataBuffer[1+i];
    	  }
    	  ulValueToSend = 1;
    	  uart_log_printf_isr("fota_ecu_req\r\n");
    	  //xQueueSendToBackFromISR(fota_ecu_req_Queue, &ulValueToSend, 0);
    	  break;
      case 0x75:
          if ((dataLength > 2) && (dataLength >= 2+dataBuffer[1]))
          {
              /* Pos. response to RequestUpload */
              reqUploadLen = GetInteger(dataBuffer[1], &dataBuffer[2]);
              if (reqUploadLen > kTdrmBufferSize)
              {
                 reqUploadLen = kTdrmBufferSize;
              }

              /* Note: if reqUploadLen exceeds kTdrmBufferSize, the tester cannot receive the data !!! */
              /*       This will be eventually blocked by TP (GetFreeBuffer())                         */

              serviceData[0] = 0x36;
              serviceData[1] = seqCnt;
              TdrmServiceRequest(serviceData, 2);
          }
          else
          {
             /* Illegal response to service $35  */
             uploadLocked = 0;
          }
         break;
      case 0x180:
           if ((dataLength > 2) && ((dataLength-2) <= uploadRemainingLen) && (dataBuffer[1]==seqCnt))
           {
              memcpy(uploadCurrentPtr, &dataBuffer[2], dataLength-2);
              uploadCurrentPtr = &uploadCurrentPtr[dataLength-2];
              uploadSourceAddress += (dataLength-2);
              uploadRemainingLen  -= (dataLength-2);
              seqCnt++;
              retryCnt = 3;
           }
           else 
           {
               if (retryCnt > 0)
               {
                     retryCnt--;
                     TdrmServiceRequest(serviceData, 2);
               }
               else
               {
                  /* All retries were unsuccessful */
                  uploadLocked = 0;
               }
           }
           if ((uploadRemainingLen > 0) && (retryCnt > 0))
           {
               /* retry last upload */
               serviceData[0]  = 0x36;
               serviceData[1]  = seqCnt;

               TdrmServiceRequest(serviceData, 2);
           }
           else
           {
               serviceData[0]  = 0x37;
               TdrmServiceRequest(serviceData, 1);
           }
         break;
        
        case 0x77:
           /* Transfer finished. All data now available */
           uploadLocked = 0;
           if (dataLength==1)
           {
              /* Upload end. All data received successfully         */
              /* Indicate this now to the application (upper layer) */
           }
         break;
   }

}

/******************************************************************************
* Name         :  ApplTdrmServiceNegResponse
* Called by    :  Application
* Preconditions:  None
* Parameters   :  negResponse: The negative response code
* Return code  :  None
* Description  :  Indicate a negative response from a module ($78 is not indicated)
******************************************************************************/
void ApplTdrmServiceNegResponse(vuint8 serviceId, vuint8 negResponse)
{
	 vuint8 ulValueToSend;
   uploadLocked = 0;
   ulValueToSend =0xdd;
   //xQueueSendToBackFromISR(fota_ecu_req_Queue, &ulValueToSend, 0);
}

/******************************************************************************
* Name         :  ApplTdrmRequestTransmitted
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  The respective service ID
* Return code  :  None
* Description  :  Indicate, that a request has been transmitted successfully 
*                 over the CAN-bus (completely through TP)
******************************************************************************/
void ApplTdrmRequestTransmitted(vuint8 serviceId)
{
}

/******************************************************************************
* Name         :  ApplTdrmTransmitError
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  See enum _tTdrmTransmitError in TDRM.H
* Return code  :  tTdrmReturn
* Description  :  Indicates a transmission error over the CAN bus (through TP)
******************************************************************************/
void ApplTdrmTransmitError(vuint8 errNo)
{
   uploadLocked = 0;
}

/******************************************************************************
* Name         :  ApplTdrmReceiveError
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  See enum _tTdrmReceiveError in TDRM.H
* Return code  :  None
* Description  :  Indicates a reception error over the CAN bus (from TP)
******************************************************************************/
void ApplTdrmReceiveError(vuint8 errNo)
{
   uploadLocked = 0;
}


