/*
 * TdrmCom.h
 *
 *  Created on: 2021Äê9ÔÂ24ÈÕ
 *      Author: Administrator
 */

#ifndef DIAG_TDRMCOM_H_
#define DIAG_TDRMCOM_H_
#include "CanTp_Cbk.h"
#include <stddef.h>

BufReq_ReturnType CanTp_User_CopyRxData(const PduIdType  RxPduId, const PduInfoType* const PduInfoPointer, PduLengthType* const RxBufferSizePtr);
BufReq_ReturnType CanTp_User_StartOfReception(const PduIdType RxPduId, const PduLengthType TpSduLength, PduLengthType* const Length);
BufReq_ReturnType CanTp_User_CopyTxData(const PduIdType TxPduId, const PduInfoType* const PduInfoPtr,
        const RetryInfoType* const RetryInfoPtr, PduLengthType* const TxDataCntPtr);
void CanTp_User_TxConfirmation(const PduIdType TxPduId, const NotifResultType Result);


#endif /* DIAG_TDRMCOM_H_ */
