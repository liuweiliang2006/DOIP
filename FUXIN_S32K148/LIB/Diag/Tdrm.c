
/* Includes ******************************************************************/
#include "tdrm.h"
#include "CanTp.h"


/* --- Version --- */
/* --- Version check --- */
#if (DIAG_TDRM_VERSION != 0x0105u) || \
    (DIAG_TDRM_RELEASE_VERSION != 0x00u)
#error "Error in Tdrm.c: Source and header file are inconsistent!"
#endif

/* Defines *******************************************************************/
#define S3TimerStop()                     S3TimerSet(0)
#define S3TimerGet()                      localS3Timer
#define S3TimerDecr()                     localS3Timer--
#define S3TimerSet(a)                     localS3Timer = (a)

#define P2TimerStop()                     P2TimerSet(0)
#define P2TimerGet()                      localP2Timer
#define P2TimerDecr()                     localP2Timer--
#define P2TimerSet(a)                     localP2Timer = (a)

#define P3TimerStop()                     P3TimerSet(0)
#define P3TimerGet()                      localP3Timer
#define P3TimerDecr()                     localP3Timer--
#define P3TimerSet(a)                     localP3Timer = (a)

#define TDRM_P3_ECU_DEFAULT_CNT           ((TDRM_P3_ECU_DEFAULT_VALUE+kTdrmCallCycle-1)/kTdrmCallCycle)        

/* Parameter to StopWaitEcuResponse() function */
#define kEcuRequestImmediate              0
#define kEcuRequestDelayed                1


/* Typdefs and structures for internal use ***********************************/


/*****************************************************************************/
/* Global and local data (sorted on const, near and far definition) **********/
/*****************************************************************************/
tTdrmState       tdrmState;
static vuint16          localSendCopyDataLen;

/*----------------------*/
/*-- Global variables --*/
/*----------------------*/
vuint8 tdrmComBuffer[kTdrmBufferSize];
vuint8 tdrmTesterPresentBuffer[kTdrmBufferSize];

/*---------------------*/
/*-- Local variables --*/
/*---------------------*/
static vuint8          *localSendReqDataPtr;
static vuint16          localSendReqDataLen;
static vuint8           localSendReqActive;

static vuint8           localCurrentSession;
static vuint8           localLastServiceRequestParam[3];
static tTdrmParam       localTdrmParam;

static vuint16          localP3Timer;
static vuint16          localS3Timer;
static vuint16          localP2Timer;

V_MEMROM0 static V_MEMROM1 tServiceInformation V_MEMROM2 serviceInformation[] = TDRM_SERVICE_INFORMATION_TABLE;

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
/*#include "memmap.h"*/
/*------  End of variable definition in NEAR section ------------------------*/


/* Prototypes for internal use ***********************************************/
static vuint16 CalculateTesterCycle(vuint16 ecuTime, vuint16 factor);
static void    TdrmCalculateTesterCycles(void);
static void TdrmCalculateTesterCyclesFast(void);
static void    StartWaitEcuResponse(void);
static void    StopWaitEcuResponse(vuint8 delayRequests);
static vuint8  TdrmGetServiceOptions(vuint8 serviceId);

/*****************************************************************************/
/* Implementation                                                            */ 
/*****************************************************************************/

/******************************************************************************
* Name         :  TdrmInitPowerOn
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  Called during power-on sequence
******************************************************************************/
tTdrmReturn TdrmInitPowerOn(vuint8 fast)
{
   /* Call COM-layer for pre-initialisation */
   TdrmComInitPowerOn();

   /* Pre-init the tdrmParameter structure */
   localTdrmParam.P2EcuThreshold          = TDRM_P2_ECU_DEFAULT_VALUE;
   localTdrmParam.P2StarEcuThreshold      = TDRM_P2STAR_ECU_DEFAULT_VALUE;
   localTdrmParam.S3EcuThreshold          = TDRM_S3_ECU_DEFAULT_VALUE;

   localTdrmParam.P2TesterFactor          = TDRM_P2_FACTOR;
   localTdrmParam.P2StarTesterFactor      = TDRM_P2STAR_FACTOR;
   /*localTdrmParam.S3TesterFactor          = TDRM_S3_FACTOR;*/

   /* Calculate the tester cycle time */
   if(fast == 0)
   {
	   TdrmCalculateTesterCycles();
   }
   else
   {
	   TdrmCalculateTesterCyclesFast();
   }

   /* Pre-init further parameters */
   localTdrmParam.EcuTargetAddress        = TDRM_ECU_TARGET_ADDRESS;
   TdrmComSetTargetAddress(TDRM_ECU_TARGET_ADDRESS);

   return TdrmInit();
}


/******************************************************************************
* Name         :  TdrmInit
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  (Re-)initializes the module
******************************************************************************/
tTdrmReturn TdrmInit(void)
{
   /* Call COM-layer for pre-initialisation */
   TdrmComInit();

   /* Initialize local state */
   tdrmState = kTdrmStateIdle;

   /* Set currently active session to default */
   localCurrentSession = kTdrmSessionDefault;

   /* Initialize S3-timer */
   S3TimerStop();

   /* Initialize P2 timer */
   P2TimerStop();

   /* Init Request send to inactive */
   localSendReqActive = 0;
   localSendCopyDataLen = 0;

   return kTdrmReturnOk;
}


/******************************************************************************
* Name         :  TdrmTask
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  None
* Description  :  Cyclic functions to manage the internal timers
******************************************************************************/
void TdrmTask(void)
{

   /* Perform state dependent action */
   switch(tdrmState)
   {
      /*--------------------------*/
      case kTdrmStateIdle:
      /*--------------------------*/
         /* Send request active. Try to transmit */
         if (localSendReqActive > 0)
         {
            /* Try a send request */
            if (TdrmComSendMessage(localSendReqDataPtr, localSendReqDataLen)==kTdrmReturnOk)
            {
               /* Yes, wait for transmission confirmation */
               tdrmState = kTdrmStateWaitSendReqConfirm;

               /* Transmission request succeeded. Stop trying to send */
               localSendReqActive = 0;
            }
            else
            {
               /* Decrement send requests */
               localSendReqActive--;
               uart_log_printf_isr("Active -\r\n");
               /* All retries have failed. Indicate error to the application */
               if (localSendReqActive==0)
               {
                  /* Indicate this to the application */
                  ApplTdrmTransmitError(kTdrmTxErrorTransmissionRequestFailed);
               }
            }
         }

         /* Check if S3 timer is running */
         if (S3TimerGet() > 0)
         {
            /* Decrement the value */
            S3TimerDecr();

            /* Check for transition to 0 -> Timer expired */
            if (S3TimerGet() == 0)
            {

               /* Send Tester present */
               S3TimerSet(localTdrmParam.S3TesterCycle);
               uart_log_printf_isr("Present\r\n");
               TdrmSetTpChannel(1);
               if (TdrmSrvReqTesterPresent()!=kTdrmReturnOk)
               {

                  /* Transmission request failed. Try next time again. */
                  S3TimerSet(1);
               }
            }
         }
       break;

      /*--------------------------*/
      case kTdrmStateWaitSendReqConfirm:
      /*--------------------------*/
       break;

      /*--------------------------*/
      case kTdrmStateWaitEcuResponse:
      /*--------------------------*/
         /* Check if P2 timer is running */
         if (P2TimerGet() > 0)
         {
            /* Decrement the value */
            P2TimerDecr();

            /* Check for transition to 0 -> Timer expired */
            if (P2TimerGet() == 0)
            {
               /* ----------------   Service request timeout! ---------------- */
               /* Send error code to application and stop waiting for response */

               /* TBD: Indicate this to the application */
               ApplTdrmReceiveError( kTdrmRxErrServiceResponseTimeout );
               /* Reset waiting for an ECU response */
               StopWaitEcuResponse(kEcuRequestImmediate);

               /* Remove localLastServiceRequestParam to prevent reception of responses */
               localLastServiceRequestParam[kSrvIdxId] = 0x00;
            }
         }
       break;

      /*--------------------------*/
      case kTdrmStateRxInProgress:
      /*--------------------------*/
       break;

      /*--------------------------*/
      case kTdrmStateWaitRxInProgress:
      /*--------------------------*/
       break;

      /*--------------------------*/
      case kTdrmStateWaitIdle:
      /*--------------------------*/
         /* Check if P2 timer is running */
         if (P3TimerGet() > 0)
         {
            /* Decrement the value */
            P3TimerDecr();
         }
         if (P3TimerGet()==0)
         {
            /* Return to idle state after P3 has expired */
            tdrmState = kTdrmStateIdle;
         }

       break;

      /*--------------------------*/
      default:
      /*--------------------------*/
       break;
   }

   /* Call communication task */
   TdrmComTask();
}


/******************************************************************************
* Name         :  TdrmServiceRequest
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  Request to transmit a general service request
******************************************************************************/
vuint8 get_value_active(void)
{
	return localSendReqActive;
}
tTdrmReturn TdrmServiceRequest(vuint8 *dataBuffer, vuint16 datalength)
{
   vsint8 i;
   tTdrmReturn retValue;


   /* Check, if transmission is allowed at the moment */
   if (localSendReqActive==0)
   {
      /* Early state locking */
      /* Prevents nested calls while this function is running */
      localSendReqActive = kTdrmSendRetryCounter;
      //uart_log_printf_isr("trReturnOk\r\n");

      /* Copy the first bytes of the transmit buffer to evaluate */
      /* the request and adapt timing/conditions in confirmation */
      for (  i  = (vsint8)((datalength < sizeof(localLastServiceRequestParam)) ? (vsint8)(datalength) : (vsint8)((sizeof(localLastServiceRequestParam))-1))
           ; i >= 0 
           ; i--  )
      {
         /* Copy the byte to internal temporary buffer */
         localLastServiceRequestParam[i] = dataBuffer[i];
      }

      /* Store parameter in local variables */
      localSendCopyDataLen = 0;
      localSendReqDataLen = datalength;
      localSendReqDataPtr = dataBuffer;

      /* Set returnvalue as successful */
      retValue = kTdrmReturnOk;
   }
   else
   {
	   uart_log_printf_isr("ReturnBusy\r\n");
      /* TDRM module not idle, reject request */
      retValue = kTdrmReturnBusy;
   }

   return retValue;
}


/******************************************************************************
* Name         :  TdrmSrvReqTesterPresent
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  Request to transmit a service function
******************************************************************************/
tTdrmReturn TdrmSrvReqTesterPresent(void)
{
   static const vuint8 kDiagFuncTesterPresent[kSrvLenTesterPresent] = 
                    { kSrvIdTesterPresent, kSrvSubParamTesterPresent };
   //TdrmSetTpChannel(2);
   /* Initiate transmission */
   return TdrmServiceRequest((vuint8 *)kDiagFuncTesterPresent, kSrvLenTesterPresent);
}

/******************************************************************************
* Name         :  TdrmSetEcuAddress
* Called by    :  Application
* Preconditions:  None
* Parameters   :  ecuTargetAddress
* Return code  :  tTdrmReturn
* Description  :  Sets ECU address if NormalFixedAddressing is used.
******************************************************************************/
tTdrmReturn TdrmSetEcuAddress(vuint8 ecuTargetAddress)
{
   if (tdrmState!=kTdrmStateIdle)
   {
      return kTdrmReturnBusy;
   }
   localTdrmParam.EcuTargetAddress = ecuTargetAddress;
   TdrmComSetTargetAddress(ecuTargetAddress);

   return kTdrmReturnOk;
}

# if defined TP_ENABLE_SINGLE_CHANNEL_TP
# else
/******************************************************************************
* Name         :  TdrmSetTpChannel
* Called by    :  Application
* Preconditions:  None
* Parameters   :  tpChannel
* Return code  :  tTdrmReturn
* Description  :  Sets the Tp channel used by TDRM
******************************************************************************/
tTdrmReturn TdrmSetTpChannel(vuint8 tpChannel)
{
   if (tdrmState!=kTdrmStateIdle)
   {
      return kTdrmReturnBusy;
   }
   TdrmComSetTpChannel(tpChannel);
   
   return kTdrmReturnOk;
}
# endif /* TP_ENABLE_SINGLE_CHANNEL_TP */

/******************************************************************************
* Name         :  TdrmSetParam
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  Overwrite current parameter settings
******************************************************************************/
tTdrmReturn TdrmSetParam(tTdrmParam *tdrmParam)
{
   if (tdrmState!=kTdrmStateIdle)
   {
      return kTdrmReturnBusy;
   }

   localTdrmParam = *tdrmParam;

   return kTdrmReturnOk;
}


/******************************************************************************
* Name         :  TdrmGetParam
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmReturn
* Description  :  Get a new parameter settings
******************************************************************************/
tTdrmReturn TdrmGetParam(tTdrmParam *tdrmParam)
{
   *tdrmParam = localTdrmParam;

   return kTdrmReturnOk;
}


/******************************************************************************
* Name         :  TdrmGetStatus
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  tTdrmState
* Description  :  Get the current status of the Tdrm-module 
******************************************************************************/
tTdrmState TdrmGetStatus(void)
{
   tTdrmState  rval;

   /* pre-init return value */
   rval = tdrmState;

   /* Provide idle stats with the information, that a session is active (when active) */
   if ((tdrmState==kTdrmStateIdle) && (localCurrentSession==kTdrmSessionDefault))
   {
      rval = kTdrmStateIdleSessionActive;
   }

   return rval;
}

/* ======================================================================= */
/*     Callbacks from TdrmCom                                              */
/* ======================================================================= */

/******************************************************************************
* Name         :  TdrmSendConfirm
* Called by    :  TdrmCom
* Preconditions:  Message transmission has occurred before.
* Parameters   :  None
* Return code  :  None
* Description  :  The function is called when a previous transmission request
*                 has been successfully performed.
******************************************************************************/
void TdrmSendConfirm(void)
{
   vuint8 serviceIndex;

   if (tdrmState == kTdrmStateWaitSendReqConfirm)
   {
      /* Check if current service has special options. */
      serviceIndex = TdrmGetServiceOptions(localLastServiceRequestParam[kSrvIdxId]);

      /* Check for session control service and store new session */
      if (localLastServiceRequestParam[kSrvIdxId] == kSrvIdSessionControl)
      {
         if (serviceIndex != kTdrmNoOptionsFound)
         {
            /* Delete possible positive response flag from session change request */
            localCurrentSession = (localLastServiceRequestParam[kSrvIdxSubParam] & (vuint8)(~serviceInformation[serviceIndex].suppressPosResp));
         }
         else
         {
            /* No suppress positive response flag configured */
            localCurrentSession = localLastServiceRequestParam[kSrvIdxSubParam];
         }
      }

      /* Special option configuration found */ 
      if (serviceIndex != kTdrmNoOptionsFound)
      {
         /* Check if session timeouts have to be configured */
         if (serviceInformation[serviceIndex].sessionChange == kTdrmStopS3Timer)
         {
            localCurrentSession = kTdrmSessionDefault;
         }
         
         /* Subparameter does not contain "SuppressPosResponse" flag */
         if ((localLastServiceRequestParam[kSrvIdxSubParam] & serviceInformation[serviceIndex].suppressPosResp) == 0x00u)
         {
            StartWaitEcuResponse();
         }
      }
      else
      {
         /* ---  Default --- */
         StartWaitEcuResponse();
      }

      /* Indicate successful transmission */
      ApplTdrmRequestTransmitted(localLastServiceRequestParam[kSrvIdxId]);
   }
   else
   {
      /* Confirmation without awaiting it */
      ApplTdrmTransmitError(kTdrmTxErrorConfirmWithoutRequest);
   }

   /* Check if tester shall wait for response */
   if (tdrmState != kTdrmStateWaitEcuResponse)
   {
      /* If not, clear all internal states */
      StopWaitEcuResponse(kEcuRequestDelayed);
   }
}

/******************************************************************************
* Name         :  TdrmTransmissionFailed
* Called by    :  TdrmCom
* Preconditions:  Message transmission in progress
* Parameters   :  None
* Return code  :  None
* Description  :  This function is called by the TDRMCOM-Layer typically when 
*                 a previous transmission request has failed (e.g. timeout).
******************************************************************************/
void TdrmTransmissionFailed(void)
{
   /* Transmission failed. Abort wait for confirmation */
   if (tdrmState==kTdrmStateWaitSendReqConfirm)
   {
      /* Reset to idle state */
      StopWaitEcuResponse(kEcuRequestDelayed);
   }
   /* Inform application */
   ApplTdrmTransmitError(kTdrmTxErrTpTransmissionError);
}

/******************************************************************************
* Name         :  TdrmReceiveIndication
* Called by    :  TdrmCom
* Preconditions:  Message reception from current targetEcu
* Parameters   :  dataBuffer: contains service request information
*                 dataLength: Length of this service request
* Return code  :  None
* Description  :  The function is called when a service response have been
*                 received from the current targetEcu.
******************************************************************************/
void TdrmReceiveIndication(vuint8 *dataBuffer, vuint16 dataLength)
{
   /* Check if this is a valid and pos. service request (related to the most recent request) */
   if (dataBuffer[kSrvIdxId] == (localLastServiceRequestParam[kSrvIdxId]|0x40))
   {
      /* This is a pos. response from the previous request */
      StopWaitEcuResponse(kEcuRequestImmediate);

      /* Indicate this to the application */
      ApplTdrmServicePosResponse(dataBuffer, dataLength);
     // uart_log_printf_isr("pos1\r\n");
   }
   /* neg response addresses last service request ? */
   else if ((dataBuffer[kSrvIdxId] == kTdrmNegResponseId) && (dataBuffer[kSrvIdxSubParam]==localLastServiceRequestParam[kSrvIdxId]) )
   {
      if (dataBuffer[kSrvIdxNegRespCode] == kTdrmNegRespResponsePending) 
      {
         /* This is a response Pending */
         StartWaitEcuResponse();
         P2TimerSet(localTdrmParam.P2StarTesterCycle);
         uart_log_printf_isr("pos2\r\n");
      }
      else 
      {
         /* Neg. response from ECU */
         StopWaitEcuResponse(kEcuRequestImmediate);

         /* Indicate this to the application */
         ApplTdrmServiceNegResponse(dataBuffer[kSrvIdxSubParam], dataBuffer[kSrvIdxNegRespCode]);
         uart_log_printf_isr("NegResponse\r\n");
         /* Check if sending of tester present messages has to be stopped */
         if (localLastServiceRequestParam[kSrvIdxId] == kSrvIdSessionControl)
         {
            S3TimerStop();
            uart_log_printf_isr("S3TimerStop\r\n");
         }
      }
   }
   else
   {
      /* Wrong value in first byte. Indicate this to the application */
      ApplTdrmReceiveError( kTdrmRxErrServiceResponseInvalid );

      /* Cancel reception */
      StopWaitEcuResponse(kEcuRequestImmediate);
   }
}

/******************************************************************************
* Name         :  TdrmReceptionFailed
* Called by    :  TdrmCom
* Preconditions:  Message transmission in progress
* Parameters   :  None
* Return code  :  None
* Description  :  This function is called by the TDRMCOM-Layer typically when 
*                 a reception in progress has failed (e.g. timeout).
******************************************************************************/
void TdrmReceptionFailed(void)
{
   /* Transmission failed. Abort wait for confirmation */
   if (tdrmState==kTdrmStateWaitRxInProgress)
   {
      /* Response has failed. Return to wait state and resume P2 timer */
      tdrmState = kTdrmStateWaitEcuResponse;
   }
   else if (tdrmState==kTdrmStateRxInProgress)
   {
      /* Reset to idle state */
      StopWaitEcuResponse(kEcuRequestImmediate);
   }
   /* Inform application (no action for TDRM needed) */
   ApplTdrmReceiveError( kTdrmRxErrTpReceptionError );
}

/******************************************************************************
* Name         :  TdrmPrepareReception
* Called by    :  TdrmCom
* Preconditions:  None
* Parameters   :  None
* Return code  :  kTdrmStateOk: Allow reception
*                 kTdrmStateFailed or kTdrmStateBusy if reception is not 
*                   accepted at the moment.
* Description  :  This function is called by the TdrmCom-layer, when a reception
*                 is going to start. 
******************************************************************************/
tTdrmState TdrmPrepareReception(void)
{
   tTdrmState   return_value;

   /* Set default return value */
   return_value = kTdrmReturnBusy;

   /* Transmission failed. Abort wait for confirmation */
   if (tdrmState==kTdrmStateWaitEcuResponse)
   {
      /* Freeze P2-timer and wait for response */
      tdrmState    = kTdrmStateWaitRxInProgress;
      return_value = kTdrmReturnOk;
   }
   else if (tdrmState==kTdrmStateIdle)
   {
      /* Wait for response only */
      tdrmState    = kTdrmStateRxInProgress;
      return_value = kTdrmReturnOk;
   }
   else
   {
      /* Unexpected/unknown service response ID from ECU */

      /* Indicate this to the application */
      ApplTdrmReceiveError( kTdrmRxErrServiceResponseUnexpectedOrInvalid );
   }
   return return_value;
}



/* ======================================================================= */
/*   Local helper functions for internal use only                          */
/* ======================================================================= */

/******************************************************************************
* Name         :  StartWaitEcuResponse
* Called by    :  TdrmSendConfirm
* Preconditions:  Service request was successfully transmitted over CAN
* Parameters   :  None
* Return code  :  None
* Description  :  Prepares internal states to wait for a diagnostic service 
*                 response from an ECU.
******************************************************************************/
static void StartWaitEcuResponse(void)
{

   /* Init P2 timer */
   P2TimerSet(localTdrmParam.P2TesterCycle);

   /* Switch to "wait for response" internal state */
   tdrmState = kTdrmStateWaitEcuResponse;

   /* No S3-timer active while waiting for a pos. response from ECU */
   S3TimerStop();
   uart_log_printf_isr("WaitEcuResp\r\n");
}

/******************************************************************************
* Name         :  StopWaitEcuResponse
* Called by    :  TdrmReceiveIndication, TdrmServiceRequest, TdrmTask, TdrmSendConfirm
* Preconditions:  None
* Parameters   :  kEcuRequestDelayed:   Next ECU request must be delayed
*                 kEcuRequestImmediate: Next ECU request can be allowed immediately.
* Return code  :  None
* Description  :  Resets internal states after waiting on a diagnostic service 
*                 response from an ECU.
******************************************************************************/
static void StopWaitEcuResponse(vuint8 delayNextRequest)
{
   if (delayNextRequest!=kEcuRequestDelayed)
   {
      tdrmState = kTdrmStateWaitIdle;
      P3TimerSet(TDRM_P3_ECU_DEFAULT_CNT);
   }
   else
   {
      tdrmState = kTdrmStateIdle;
   }
   P2TimerStop();

   /* Start S3-timer */
   if (localCurrentSession!=kTdrmSessionDefault)
   {
      /* (Re-)Start S3 timer */
      S3TimerSet(localTdrmParam.S3TesterCycle);
      //uart_log_printf_isr("RStart S3\r\n");
   }
   else
   {
      S3TimerStop();
      uart_log_printf_isr("pos6\r\n");
   }
}


/******************************************************************************
* Name         :  CalculateThreshold
* Called by    :  TdrmCalculateTesterThreshold
* Preconditions:  None
* Parameters   :  ecuTime   : The threshold time in ms of the ECU
*                 factor    : A factor in percent the tester is adding to the ECU wait time
* Return code  :  The resulting tester threshold time
* Description  :  The values ecuTime and factor are added as ecuTime + ecuTime*(factor/100).
*                 Afterwards, the value is devided by the call cycle time.
******************************************************************************/
static vuint16 CalculateTesterCycle(vuint16 ecuTime, vuint16 factor)
{
   vuint32 l;

   /* Use long var due to potential overflow in calculation */
   l = (vuint32)ecuTime + (vuint32)(ecuTime*factor)/100;
   l = (l + kTdrmCallCycle-1)/kTdrmCallCycle;

   return( (vuint16)(l & 0xFFFFu) );
}

/******************************************************************************
* Name         :  TdrmCalculateTesterCycles
* Called by    :  Application
* Preconditions:  None
* Parameters   :  None
* Return code  :  None
* Description  :  Get the current status of the Tdrm-module 
******************************************************************************/
static void TdrmCalculateTesterCycles(void)
{
   localTdrmParam.P2TesterCycle      = CalculateTesterCycle(localTdrmParam.P2EcuThreshold,     localTdrmParam.P2TesterFactor    );
   localTdrmParam.P2StarTesterCycle  = CalculateTesterCycle(localTdrmParam.P2StarEcuThreshold, localTdrmParam.P2StarTesterFactor);

   /* S3 tester cycle shall be half of the ECU threshold cycle */
   localTdrmParam.S3TesterCycle      = (localTdrmParam.S3EcuThreshold >> 1) / kTdrmCallCycle;
   /*localTdrmParam.S3TesterCycle      = CalculateTesterCycle(localTdrmParam.S3TesterCycle,      localTdrmParam.S3TesterFactor    );*/
}
static void TdrmCalculateTesterCyclesFast(void)
{
   localTdrmParam.P2TesterCycle      = 10*CalculateTesterCycle(localTdrmParam.P2EcuThreshold,     localTdrmParam.P2TesterFactor    );
   localTdrmParam.P2StarTesterCycle  = 10*CalculateTesterCycle(localTdrmParam.P2StarEcuThreshold, localTdrmParam.P2StarTesterFactor);

   /* S3 tester cycle shall be half of the ECU threshold cycle */
   localTdrmParam.S3TesterCycle      = 10*(localTdrmParam.S3EcuThreshold >> 1) / kTdrmCallCycle;
   /*localTdrmParam.S3TesterCycle      = CalculateTesterCycle(localTdrmParam.S3TesterCycle,      localTdrmParam.S3TesterFactor    );*/
}

/******************************************************************************
* Name         :  TdrmGetServiceOptions
* Called by    :  TdrmSendConfirm
* Preconditions:  None
* Parameters   :  Service ID
* Return code  :  Service information table index
* Description  :  The function checks if special service options are configured
*			      for the current service. It returns the service index or
*                 kTdrmNoOptionsFound.
******************************************************************************/
static vuint8 TdrmGetServiceOptions(vuint8 serviceId)
{
   vuint8 serviceCount = 0x00u;
   vuint8 serviceIndex = kTdrmNoOptionsFound;
   vuint8 serviceTableSize;
   
   /* Calculate number of service table lines */
   serviceTableSize = sizeof(serviceInformation);
   if (serviceTableSize > 0)
   {
      serviceTableSize /= sizeof(serviceInformation[0]);
   }

   /* Check if service has special options */
   while ((serviceCount < serviceTableSize) && (serviceIndex == kTdrmNoOptionsFound))
   {
      /* Check if requested service is located in table */
      if (serviceId == serviceInformation[serviceCount].serviceId)
      {
         /* store service table index */
         serviceIndex = serviceCount;
      }
      else
      {
         /* not found, increment index */
         serviceCount++;
      }
   }
   
   return serviceIndex;
}

vuint8 testData[100];
BufReq_ReturnType Tdrm_CopyTxData(const PduInfoType* const PduInfoPtr)
{
    uint32 i;
    BufReq_ReturnType result = BUFREQ_OK;


    /*if ((PduInfoPtr == NULL)||(localSendReqDataPtr == NULL))
    {
        result = BUFREQ_E_NOT_OK;
    }
    else if(PduInfoPtr->SduLength > (localSendReqDataLen - localSendCopyDataLen))
    {
        result = BUFREQ_E_OVFL;
    }
    else*/
    {
        for(i = 0; i < PduInfoPtr->SduLength; i++)
        {
            PduInfoPtr->SduDataPtr[i] = localSendReqDataPtr[localSendCopyDataLen];
            testData[i] = PduInfoPtr->SduDataPtr[i];
            localSendCopyDataLen++;
        }
    }
    return result;
}
void Tdrm_TxConfirmation(vuint8 state)
{
    localSendCopyDataLen = 0;
    if (state == NTFRSLT_OK)
    {
        TdrmSendConfirm();
    }
    else
    {
        TdrmTransmissionFailed();
    }
}
/* ---------------------------------------------------------------- */
/* Static compile checks of some limitations of this implementation */


