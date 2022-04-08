#ifndef __TDRM_CFG_H__
#define __TDRM_CFG_H__

/* Add necessary includes to integrate TDRM in your application here */
/*#include "fbl_inc.h"*/ 

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* Configuration options of the TDRM manager                                 */
/* ------------------------------------------------------------------------- */
/*-------------------------------------------------------------------------- */

/* Call cycle in ms of the TdrmTask()                                   */
#define kTdrmCallCycle                             10

/* Index to the TP connection (for multi-channel communication only)    */
#define kTdrmTpConnection                          TpRxHandleTDRM_0

/* Used TP channel (used for configurations with static channel configuration) */
#define kTdrmTpChannel                             0

/* Maximum retries for internal transmission requests */
#define kTdrmSendRetryCounter                      10


/* Configuration of the diagnostic request buffer length.               */
/* This is used for both transmission and reception.                    */
#define kTdrmBufferSize                            4095

/* Pre-configuration of the TDRM timeout offset values                  */
/* The values are the tolerances in % for the tester in respect to      */
/* its timer value of the ECU.                                          */
/* Example:                                                             */
/* P2(ECU)=50ms, TDRM-Factor is 20%  => P2(Tester) = 50 + 50*0.2 = 60ms */
#define TDRM_P2_FACTOR                             20
#define TDRM_P2STAR_FACTOR                         20

/* Default values of the TDRM module used for power-on initialisation   */
/* The values are seen from the ECU side. The TDRM will add a           */
/* percentage factor for internal timeout observation (see xxx_FACTOR)  */
#define TDRM_P2_ECU_DEFAULT_VALUE                  100
#define TDRM_P2STAR_ECU_DEFAULT_VALUE              5000
#define TDRM_S3_ECU_DEFAULT_VALUE                  5000

/* ECU request delay time to avoid overrun */
#define TDRM_P3_ECU_DEFAULT_VALUE                  20

/* Default ECU address */
#define TDRM_ECU_TARGET_ADDRESS                    0xEE
/* Tester source address */
#define TDRM_TESTER_SOURCE_ADDRESS                 0xF0
/* CAN channel used by TDRM */
#define TDRM_CAN_CHANNEL                           0x00

/* Configure services */
/* Start communication control */
#define kSrvIdStartCommControl                  0x81u

/* Stop communication control */
#define kSrvIdStopCommControl                   0x82u

/* Access timing parameter */
#define kSrvIdAccessTimingParameter             0x83u

/* Control DTC settings */
#define kSrvIdControlDTCSettings                0x85u

/* Link Control */
#define kSrvIdLinkControl                       0x87u

/* EcuReset defines */
#define kSrvIdEcuReset                          0x11u

/* Tester Present defines */
#define kSrvLenTesterPresent                    2
#define kSrvIdTesterPresent                     0x3Eu
#define kSrvSubParamTesterPresent               0x0u//0x80u

/* Session control defines */
#define kSrvLenSessionControl                   2
#define kSrvIdSessionControl                    0x10u

/* Session information */
#define kTdrmSessionDefault                     0x01u
#define kTdrmSessionExtended                    0x02u
#define kTdrmSessionProgramming                 0x03u


/* ReadDataByIdentifier defines */
#define kSrvLenReadDataByIdentifier             3
#define kSrvIdReadDataByIdentifier              0x22

/* RoutineControl defines */
#define kSrvLenRoutineControl                   4
#define kSrvIdRoutineControl                    0x31

/* RoutineControl types */
#define kSrvRcTypeStartRoutine                  0x01
#define kSrvRcTypeStopRoutine                   0x02
#define kSrvRcTypeRequestRoutineResults         0x03

/* Service information table */
/* This table configures how to handle the S3 timer
   and the suppress positive response feature for additional
   services.
   Example:
   EcuReset triggers a change to default session 
   (stop S3 timer) and can be sent with
   SuppressPositiveResponse if the second byte
   of the message has bit 0x80 set:
#define TDRM_SERVICE_INFORMATION_TABLE \
{                                                                          \
   {kSrvIdSessionControl, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag}, \
   {kSrvIdEcuReset, kTdrmStopS3Timer, kTdrmSuppressPosResponseFlag}        \
}
*/   
#define TDRM_SERVICE_INFORMATION_TABLE \
{                                                        \
   {kSrvIdSessionControl, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag}, \
   {kSrvIdEcuReset, kTdrmStopS3Timer, kTdrmSuppressPosResponseFlag},           \
   {kSrvIdRoutineControl, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag},           \
   {kSrvIdTesterPresent, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag},            \
   {kSrvIdStartCommControl, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag},         \
   {kSrvIdStopCommControl, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag},          \
   {kSrvIdAccessTimingParameter, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag},    \
   {kSrvIdControlDTCSettings, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag},       \
   {kSrvIdLinkControl, kTdrmKeepS3Timer, kTdrmSuppressPosResponseFlag}               \
}

#endif   /* __TDRM_CFG_H__ */
