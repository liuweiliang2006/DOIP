

#ifndef __TDRM_H__
#define __TDRM_H__

/* --- Version --- */
/* ##V_CFG_MANAGEMENT ##CQProject : Diag_Tdrm CQComponent : Implementation */
#define DIAG_TDRM_VERSION                0x0105u
#define DIAG_TDRM_RELEASE_VERSION        0x00u

/* Includes ******************************************************************/

#include "v_def.h"
#include "tdrm_cfg.h"

/*******************************************************************************
* Defines
*******************************************************************************/
#define kTpHoldChannel  0

/* ============================================== */
/* --------- Diag service definition ------------ */
/* ============================================== */

/* Handle S3 timer for additional services */
typedef enum
{
   kTdrmKeepS3Timer
  ,kTdrmStopS3Timer
} tTdrmSessionChange;

/* Configuration data for special service options */
typedef struct
{
   vuint8 serviceId;                   /* Service ID with special options */
   tTdrmSessionChange sessionChange;   /* Perform a session change to default session? */
   vuint8 suppressPosResp;             /* How is suppress positive response indicated? */
}tServiceInformation;

/* No special service options found. */
#define kTdrmNoOptionsFound						0xFFu

/* ============================================== */
/* ----  Administrative service data defines ---- */
/* ============================================== */

/* Service specific defines */
#define kTdrmSuppressPosResponseFlag            0x80u
#define kTdrmNegResponseId                      0x7Fu
#define kTdrmNegRespResponsePending             0x78u

/* Index into the diagnstic transfer buffer */
#define kSrvIdxId                         0
#define kSrvIdxSubParam                   1
#define kSrvIdxNegRespCode                2



/*******************************************************************************
* Typdefs and structures for global use
*******************************************************************************/

/* Return value from functions */
typedef enum _tTdrmReturnValue
{
   kTdrmReturnOk
  ,kTdrmReturnFailed
  ,kTdrmReturnBusy
} tTdrmReturn;

/* Internal status of TDRM */
typedef enum _tTdrmState
{
   kTdrmStateIdle                /* System in idle state */
  ,kTdrmStateIdleSessionActive   /* System session is active (just for TdrmGetStatus() when in idle mode) */
  ,kTdrmStateTxInProgress        /* Temporary state while sending data */
  ,kTdrmStateWaitSendReqConfirm  /* Request issued to TP. Waiting for confirmation */
  ,kTdrmStateWaitEcuResponse     /* Service successfully transmitted to ECU. Wait for ECU response */
  ,kTdrmStateRxInProgress        /* Temporary state while receiving data */
  ,kTdrmStateWaitRxInProgress    /* Temporary state while receiving data */
  ,kTdrmStateWaitIdle            /* Temporary state after a transmission */
                                 /* Waits P3 to prevent an ECU overrun   */
} tTdrmState;


/* TDRM timer parameter values to manage diagnostic services */
typedef struct _tTdrmParam
{
   /* Factor values used to calculate P2, P2* and S3 timeout values */
   vuint16  P2TesterFactor;
   vuint16  P2StarTesterFactor;
   /*vuint16  S3TesterFactor;*/

   vuint16  P2EcuThreshold;
   vuint16  P2StarEcuThreshold;
   vuint16  S3EcuThreshold;

   vuint16  P2TesterCycle;
   vuint16  P2StarTesterCycle;
   vuint16  S3TesterCycle;

   vuint16  P3TesterCycle;

   vuint8   EcuTargetAddress;
} tTdrmParam;

/* Receive errors */
typedef enum _tTdrmReceiveError
{
   kTdrmRxErrTpReceptionError=0
  ,kTdrmRxErrServiceResponseInvalid
  ,kTdrmRxErrServiceResponseUnexpectedOrInvalid
  ,kTdrmRxErrServiceResponseTimeout
} tTdrmReceiveError;

/* Transmit errors */
typedef enum _tTdrmTransmitError
{
   kTdrmTxErrTpTransmissionError=0
  ,kTdrmTxErrorConfirmWithoutRequest
  ,kTdrmTxErrorTransmissionRequestFailed
} tTdrmTransmitError;


/*******************************************************************************
* Global data (export and import)
*******************************************************************************/
extern vuint8        tdrmComBuffer[kTdrmBufferSize];
extern tTdrmState    tdrmState;


/*******************************************************************************
* Global (exported) functions
*******************************************************************************/
#ifdef  __cplusplus
extern "C" {
#endif
/* Called during power-on sequence */
tTdrmReturn TdrmInitPowerOn(vuint8 fast);
/* (Re-)initializes the module */
tTdrmReturn TdrmInit(void);
/* Cyclic functions to manage the internal timers */
void        TdrmTask(void);
/* General purpose service transmission request function */
tTdrmReturn TdrmServiceRequest(vuint8 *dataBuffer, vuint16 datalength);

/* Dedicated service transmission request functions */
tTdrmReturn TdrmSrvReqDiagSessionControl(vuint8 session_type);
/* Send a tester present request */
tTdrmReturn TdrmSrvReqTesterPresent(void);
/* Send a tester present request */
tTdrmReturn TdrmSrvReqReadDataByIdentifier(vuint16 identifier);


/* Set the ECU target address for the physical request/responses */
tTdrmReturn TdrmSetEcuAddress(vuint8 ecuTargetAddress);
/* Set Tp channel */
tTdrmReturn TdrmSetTpChannel(vuint8 tpChannel);
/* Set a new parameter settings */
tTdrmReturn TdrmSetParam(tTdrmParam *tdrmParam);
/* Get the current parameter settings */
tTdrmReturn TdrmGetParam(tTdrmParam *tdrmParam);
/* Get the current status of the Tdrm-module */
tTdrmState  TdrmGetStatus(void);

/* ============================= */
/*   Callbacks from TdrmCom      */
/* ============================= */
/* Data transmission successfully performed */
void        TdrmSendConfirm(void);
/* Transmission request failed */
void        TdrmTransmissionFailed(void);
/* Valid data reception */
void        TdrmReceiveIndication(vuint8 *dataBuffer, vuint16 dataLength);
/* Error occurred during reception */
void        TdrmReceptionFailed(void);
/* Initiation of reception */
tTdrmState  TdrmPrepareReception(void);


/* ---------------------------------------------- */
/* Functions in the TdrmCom module called by TDRM */
/* ---------------------------------------------- */
tTdrmReturn TdrmComSendMessage( vuint8 *dataBuffer, vuint16 dataLength );
void        TdrmComSetTargetAddress(vuint8 targetAddress);
void        TdrmComSetTpChannel(vuint8 tpChannel);
void        TdrmComInitPowerOn(void);
void        TdrmComInit(void);
void        TdrmComTask(void);

#ifdef  __cplusplus
}
#endif
/*---------  End definition of global exported functions -------------------- */

/*******************************************************************************
* Global (imported) functions as user callbacks
*******************************************************************************/
#ifdef  __cplusplus
extern "C" {
#endif
/* Indicate a positive response from a module */
void ApplTdrmServicePosResponse(vuint8 *dataBuffer, vuint16 dataLength);
/* Indicate a negative response from a module */
void ApplTdrmServiceNegResponse(vuint8 serviceId, vuint8 negResponse);
/* Indicate, that a request has been transmitted successfully over the CAN-bus */
void ApplTdrmRequestTransmitted(vuint8 serviceId);
/* Indicates a problem while transmitting a service request over the CAN bus */
void ApplTdrmTransmitError(vuint8 errNo);
/* Indicates a problem while receiving or awaiting a service response */
void ApplTdrmReceiveError(vuint8 errNo);

extern void uart_log_printf_isr(const char *format, ...);
tTdrmReturn TdrmSrvReqDtcSettingOnFunc(void);
tTdrmReturn TdrmSrvReqEnRxAndDisTxFunc(void);

#ifdef  __cplusplus
}
#endif
/*---------  End definition of global imported functions -------------------- */


#endif  /* __EEPMGR_H__ */

