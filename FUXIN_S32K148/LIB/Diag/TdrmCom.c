
#define __TDRMCOM_C__

/* Includes ******************************************************************/
#include "tdrm.h"
#include "CanTp_Cbk.h"
#include <stddef.h>
//#include "doip_client.h"

/* --- Version --- */
#if ( DIAG_TDRM_VERSION         != 0x0105u ) || \
    ( DIAG_TDRM_RELEASE_VERSION !=   0x00u )
# error "Error in TdrmCom.c: Tdrm and TdrmCom are inconsistent!"
#endif

/* Defines *******************************************************************/

/* Typdefs and structures for internal use ***********************************/

/*****************************************************************************/
/* Global and local data (sorted on const, near and far definition) **********/
/*****************************************************************************/

/*----------------------*/
/*-- Global variables --*/
/*----------------------*/

/*---------------------*/
/*-- Local variables --*/
/*---------------------*/
# ifdef TP_ENABLE_SINGLE_CHANNEL_TP
# else
vuint8 tdrmTpChannel;
# endif
static vuint8 tdrmComTargetAddress;

/*---------------------------------------------------------------------------*/
/*----- Begin of variable definition in NEAR section ------------------------*/
/*---------------------------------------------------------------------------*/
#define TDRM_START_SEC_NEARDATA
/*#include "memmap.h"*/
/*------------------------------------*/
/*-- Global variables in NEAR section */
/*------------------------------------*/

/*-----------------------------------*/
/*-- Local variables in NEAR section */
/*-----------------------------------*/

#define TDRM_STOP_SEC_NEARDATA
void Tdrm_TxConfirmation(vuint8 state);
/*#include "memmap.h"*/
/*------  End of variable definition in NEAR section ------------------------*/

/* Prototypes for internal use ***********************************************/

/*****************************************************************************/
/* Implementation                                                            */ 
/*****************************************************************************/

/******************************************************************************
* Name         :  TdrmComInitPowerOn
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  None
* Return code  :  None
* Description  :  Called during power-on sequence
******************************************************************************/
void TdrmComInitPowerOn(void)
{
# ifdef TP_ENABLE_SINGLE_CHANNEL_TP
# else
#  if defined (TP_ENABLE_DYNAMIC_CHANNELS)
   tdrmTpChannel = TpTxGetFreeChannel(kTdrmTpConnection);
   TpTxSetCanChannel(tdrmTpChannel, TDRM_CAN_CHANNEL);
   TpTxLockChannel(tdrmTpChannel);
#  else
   tdrmTpChannel = kTdrmTpChannel;
#  endif
# endif
}

/******************************************************************************
* Name         :  TdrmComInit
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  None
* Return code  :  None
* Description  :  Called during init sequence
******************************************************************************/
void TdrmComInit(void)
{
}

/******************************************************************************
* Name         :  TdrmComTask
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  None
* Return code  :  None
* Description  :  Called cyclically every kTdrmCycleTime
******************************************************************************/
void TdrmComTask(void)
{
}

/******************************************************************************
* Name         :  TdrmComSendMessage
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  Requested buffer size
* Return code  :  Pointer to data buffer
* Description  :  This function is called by the transport layer precopy
*                 function and returns a pointer to the diagnostic buffer
******************************************************************************/
vuint8 aavv[20];
tTdrmReturn TdrmComSendMessage( vuint8 *dataBuffer, vuint16 dataLength )
{
   vuint8      tp_rval;
   tTdrmReturn tdrm_rval;

# if defined ( TP_ENABLE_SINGLE_CHANNEL_TP )
   tp_rval = TpTransmit(dataBuffer, dataLength);
# else
   //tp_rval = TpTransmit(tdrmTpChannel,dataBuffer, dataLength);
# endif
   PduInfoType Tdrm_info_temp;
   Tdrm_info_temp.SduDataPtr = dataBuffer;
   Tdrm_info_temp.SduLength = dataLength;
   int i;
   for(i=0;i<dataLength;i++)
   {
	   aavv[i]=Tdrm_info_temp.SduDataPtr[i];
   }
//   tp_rval = CanTp_Transmit(tdrmTpChannel,&Tdrm_info_temp);
//   tp_rval = doip_client_send(Tdrm_info_temp.SduDataPtr,&Tdrm_info_temp.SduLength);

   /* Map TpTransmit return value to internal TDRM return value */
   switch (tp_rval)
   {
      case kTpSuccess:
         tdrm_rval = kTdrmReturnOk;
       break;
      case kTpBusy:
         tdrm_rval = kTdrmReturnBusy;
       break;
      case kTpFailed:
      default:
         tdrm_rval = kTdrmReturnFailed;
       break;
   }

   return tdrm_rval;
}

/******************************************************************************
* Name         :  TdrmComSetTargetAddress
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  Set the target address of the ECU for communication
* Return code  :  None
* Description  :  Sets the target address of the ECU to calculate the 
*                 CAN-identifier for communication.
******************************************************************************/
void TdrmComSetTargetAddress(vuint8 targetAddress)
{
# if defined ( TP_ENABLE_SINGLE_CHANNEL_TP )
   TpTxSetTargetAddress(targetAddress);
# else
#  if defined (TP_ENABLE_DYNAMIC_CHANNELS)
   TpTxSetTargetAddress(tdrmTpChannel, targetAddress);
#  endif
# endif
   tdrmComTargetAddress = targetAddress;
}

# if defined ( TP_ENABLE_SINGLE_CHANNEL_TP )
#else
/******************************************************************************
* Name         :  TdrmComSetTpChannel
* Called by    :  TDRM
* Preconditions:  None
* Parameters   :  New Tp channel
* Return code  :  None
* Description  :  Sets the TP channel used by TDRM
******************************************************************************/
void TdrmComSetTpChannel(vuint8 tpChannel)
{
   tdrmTpChannel = tpChannel;
}
# endif /* TP_ENABLE_SINGLE_CHANNEL_TP */

/******************************************************************************
* Name         :  TdrmComRxGetBuffer
* Called by    :  TPMC
* Preconditions:  None
* Parameters   :  Requested buffer size
* Return code  :  Pointer to data buffer
* Description  :  This function is called by the transport layer precopy
*                 function and returns a pointer to the diagnostic buffer
*
*              !! Note that the buffer is only provided if a response
*                 from the ECU is expected. This allows a shared Rx/Tx-buffer!!
******************************************************************************/
# ifdef TP_ENABLE_SINGLE_CHANNEL_TP
vuint8* TdrmComRxGetBuffer( vuint16 dataLength )

# else
tTdrmState  TdrmPrepareReception(void);
vuint8* TdrmComRxGetBuffer(vuint8 tpChannel,  vuint16 dataLength )
# endif /* TP_ENABLE_SINGLE_CHANNEL_TP */
{
   /* Check if buffer size fits and buffer is available */
   if ((dataLength < kTdrmBufferSize) && (TdrmPrepareReception()==kTdrmReturnOk))
   {
      return tdrmComBuffer;
   }
   else
   {
      return 0;
   }
}

# if ((defined TP_USE_MULTIPLE_ECU_NR) && (TP_USE_MULTIPLE_ECU_NR == kTpOn))
/******************************************************************************
* Name         :  TdrmComCheckTargetAddress
* Called by    :  TPMC
* Preconditions:  None
* Parameters   :  target address of currently received frame
* Return code  :  None
* Description  :  This function checks 
******************************************************************************/
t_ta_type TdrmComCheckTargetAddress(vuint8 tpCurrentTargetAddress)
{
   if (TDRM_TESTER_SOURCE_ADDRESS == tpCurrentTargetAddress)
   {
      return kTpPhysical;
   }
   return kTpNone;
}
# endif /* TP_USE_MULTIPLE_ECU_NR */

/******************************************************************************
* Name         :  TdrmComReceiveMessage
* Called by    :  TPMC
* Preconditions:  None
* Parameters   :  Number of received bytes
* Return code  :  None
* Description  :  Indication function for a transport layer message
******************************************************************************/
#if defined ( TP_ENABLE_SINGLE_CHANNEL_TP ) 
void TdrmComReceiveMessage( vuint16 dataLength )
#else
void TdrmComReceiveMessage( vuint8 tpChannel, vuint16 dataLength )
#endif /* TP_ENABLE_SINGLE_CHANNEL_TP || TDRM_USE_FBLTP */
{
   /* ToDo: Check for valid targetAddress */
   TdrmReceiveIndication( tdrmComBuffer, dataLength );

# if defined ( TP_ENABLE_SINGLE_CHANNEL_TP )
   TpRxResetChannel();
# else
   TpRxResetChannel(tpChannel);
# endif /* TP_ENABLE_SINGLE_CHANNEL_TP */
}

/******************************************************************************
* Name         :  TdrmComTxConfirmation
* Called by    :  TPMC when a diagnostic message has been transmitted
*                 successfully
* Preconditions:  Service must have been received
* Parameters   :  The result type of the confirmation
* Return code  :  None
* Description  :  Indicates that a diagnostic service was sent by the TP layer
*                 or an error has occurred during transmission.
*                 According to ISO-14229, the tester present timer must be
*                 restarted.
******************************************************************************/
#if defined ( TP_ENABLE_SINGLE_CHANNEL_TP ) 
void TdrmComTxConfirmation( vuint8 state )
#else
void TdrmComTxConfirmation( vuint8 tpChannel, vuint8 state )
#endif /* TP_ENABLE_SINGLE_CHANNEL_TP || TDRM_USE_FBLTP*/
{
   TdrmSendConfirm();
}

/******************************************************************************
* Name         :  TdrmComTxErrorIndication
* Called by    :  TPMC
* Preconditions:  None
* Parameters   :  TPMC error number
* Return code  :  None
* Description  :  This function is called in case of a transmit error in the
*                 transport layer
******************************************************************************/
# ifdef TP_ENABLE_SINGLE_CHANNEL_TP
vuint8 TdrmComTxErrorIndication( vuint8 errNo )
# else
vuint8 TdrmComTxErrorIndication( vuint8 tpChannel, vuint8 errNo )
# endif /* TP_ENABLE_SINGLE_CHANNEL_TP */
{
   /* Inform TDRM layer */
   TdrmTransmissionFailed();

   return kTpHoldChannel;
}

/******************************************************************************
* Name         :  TdrmComRxErrorIndication
* Called by    :  TPMC if an error occurred during reception of a multi-frame
* Preconditions:  TP must been initialized
* Parameters   :  Details the error that has occurred during reception
* Return code  :  None
* Description  :  This function is called in case of a receive error in the
*                 transport layer
******************************************************************************/
#if defined ( TP_ENABLE_SINGLE_CHANNEL_TP ) 
void TdrmComRxErrorIndication( vuint8 errNo )
#else
void TdrmComRxErrorIndication( vuint8 tpChannel, vuint8 errNo )
#endif /* TP_ENABLE_SINGLE_CHANNEL_TP || TDRM_USE_FBLTP */
{
   TdrmReceptionFailed();
}

/******************************************************************************
* Name         :  ApplTdrmComFatalError
* Called by    :  TPMC
* Preconditions:  None
* Parameters   :  TPMC error number
* Return code  :  None
* Description  :  This function is called in case of a fatal error in the
*                 transport layer
******************************************************************************/
void ApplTdrmComFatalError( vuint8 errNo )
{
   ApplTdrmReceiveError( errNo );
}
vuint16          tdrmComBufLen;

void Tdrm_RxIndication(vuint8 tpChannel, vuint8 state)
{
	//TdrmReceiveIndication( tdrmComBuffer, tdrmComBufLen );
    if (state == 0x00)
    {
        TdrmReceiveIndication( tdrmComBuffer, tdrmComBufLen );
    }
    else
    {
        TdrmReceptionFailed();
    }
}
BufReq_ReturnType Tdrm_CopyRxData(const PduInfoType* const  PduInfoPointer)
{
    BufReq_ReturnType result = BUFREQ_OK;
    uint32 i;
    if(PduInfoPointer == NULL)
    {
        result = BUFREQ_E_NOT_OK;
    }
    else if((tdrmState != kTdrmStateWaitRxInProgress)&&(tdrmState != kTdrmStateRxInProgress))
    {
        result = BUFREQ_E_BUSY;
    }
    else if((kTdrmBufferSize - tdrmComBufLen) < PduInfoPointer->SduLength)
    {
        result = BUFREQ_E_OVFL;
    }
    else
    {
        for(i = (uint32)0u; i < PduInfoPointer->SduLength; i++)
        {
            tdrmComBuffer[tdrmComBufLen] = PduInfoPointer->SduDataPtr[i];
            tdrmComBufLen++;
        }
    }
    return result;
}
BufReq_ReturnType CanTp_User_CopyRxData(const PduIdType  RxPduId, const PduInfoType* const PduInfoPointer, PduLengthType* const RxBufferSizePtr)
{
    BufReq_ReturnType result = BUFREQ_OK;
    result = Tdrm_CopyRxData(PduInfoPointer);
    return result;
}

BufReq_ReturnType Tdrm_StartOfReception
(
    const PduLengthType TpSduLength,
    PduLengthType* const Length
)
{
    BufReq_ReturnType result = BUFREQ_OK;

    if (TdrmPrepareReception() != kTdrmReturnOk)
    {
        result = BUFREQ_E_NOT_OK;
    }
    else if(TpSduLength >= kTdrmBufferSize)
    {
        result = BUFREQ_E_OVFL;
    }
    else
    {
        *Length = (uint16)kTdrmBufferSize;
        tdrmComBufLen = 0;
    }

    return result;
}
BufReq_ReturnType CanTp_User_StartOfReception(const PduIdType RxPduId, const PduLengthType TpSduLength, PduLengthType* const Length)
{
    BufReq_ReturnType result = BUFREQ_OK;

    result = Tdrm_StartOfReception(TpSduLength, Length);


    return result;
}
BufReq_ReturnType CanTp_User_CopyTxData(const PduIdType TxPduId, const PduInfoType* const PduInfoPtr,
        const RetryInfoType* const RetryInfoPtr, PduLengthType* const TxDataCntPtr)
{
    BufReq_ReturnType result = BUFREQ_OK;
    result = Tdrm_CopyTxData(PduInfoPtr);

    return result;
}
void CanTp_User_TxConfirmation(const PduIdType TxPduId, const NotifResultType Result)
{

        Tdrm_TxConfirmation(Result);

}



/* ---------------------------------------------------------------- */
/* Static compile checks of some limitations of this implementation */

