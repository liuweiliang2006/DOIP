/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : lin1.h
**     Project     : FUXIN_S32K148
**     Processor   : S32K148_176
**     Component   : lin
**     Version     : Component SDK_S32K1xx_15, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32K1xx_15
**     Compiler    : GNU C Compiler
**     Date/Time   : 2021-11-08, 11:26, # CodeGen: 0
**     Contents    :
**         LIN_DRV_Init                     - status_t LIN_DRV_Init(uint32_t instance, lin_user_config_t* linUserConfig,...
**         LIN_DRV_Deinit                   - void LIN_DRV_Deinit(uint32_t instance);
**         LIN_DRV_GetDefaultConfig         - void LIN_DRV_GetDefaultConfig(bool isMaster, lin_user_config_t * linUserConfig);
**         LIN_DRV_InstallCallback          - lin_callback_t LIN_DRV_InstallCallback(uint32_t instance, lin_callback_t...
**         LIN_DRV_SendFrameDataBlocking    - status_t LIN_DRV_SendFrameDataBlocking(uint32_t instance, uint8_t* txBuff,...
**         LIN_DRV_SendFrameData            - status_t LIN_DRV_SendFrameData(uint32_t instance, uint8_t* txBuff, uint32_t...
**         LIN_DRV_GetTransmitStatus        - status_t LIN_DRV_GetTransmitStatus(uint32_t instance, uint32_t* bytesRemaining);
**         LIN_DRV_AbortTransferData        - status_t LIN_DRV_AbortTransferData(uint32_t instance);
**         LIN_DRV_ReceiveFrameDataBlocking - status_t LIN_DRV_ReceiveFrameDataBlocking(uint32_t instance, uint8* rxBuff,...
**         LIN_DRV_ReceiveFrameData         - status_t LIN_DRV_ReceiveFrameData(uint32_t instance, uint8_t* rxBuff,...
**         LIN_DRV_GetReceiveStatus         - status_t LIN_DRV_GetReceiveStatus(uint32_t instance, uint32_t* bytesRemaining);
**         LIN_DRV_GoToSleepMode            - status_t LIN_DRV_GoToSleepMode(uint32_t instance);
**         LIN_DRV_GotoIdleState            - status_t LIN_DRV_GotoIdleState(uint32_t instance);
**         LIN_DRV_SendWakeupSignal         - status_t LIN_DRV_SendWakeupSignal(uint32_t instance);
**         LIN_DRV_GetCurrentNodeState      - lin_node_state_t LIN_DRV_GetCurrentNodeState(uint32_t instance);
**         LIN_DRV_TimeoutService           - void LIN_DRV_TimeoutService(uint32_t instance);
**         LIN_DRV_SetTimeoutCounter        - void LIN_DRV_SetTimeoutCounter(uint32_t instance, uint32_t timeoutValue);
**         LIN_DRV_MasterSendHeader         - status_t LIN_DRV_MasterSendHeader(uint32_t instance, uint8_t id);
**         LIN_DRV_EnableIRQ                - status_t LIN_DRV_EnableIRQ(uint32_t instance);
**         LIN_DRV_DisableIRQ               - status_t LIN_DRV_DisableIRQ(uint32_t instance);
**         LIN_DRV_IRQHandler               - void LIN_DRV_IRQHandler(uint32_t instance);
**         LIN_DRV_ProcessParity            - uint8_t LIN_DRV_ProcessParity(uint8_t PID, uint8_t typeAction);
**         LIN_DRV_MakeChecksumByte         - uint8_t LIN_DRV_MakeChecksumByte(uint8_t* buffer, uint8_t sizeBuffer, uint8_t...
**         LIN_DRV_AutoBaudCapture          - status_t LIN_DRV_AutoBaudCapture(uint32_t instance);
**
**     Copyright 1997 - 2015 Freescale Semiconductor, Inc. 
**     Copyright 2016-2017 NXP 
**     All Rights Reserved.
**     
**     THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
**     IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
**     OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
**     IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
**     INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
**     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
**     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
**     STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
**     IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
**     THE POSSIBILITY OF SUCH DAMAGE.
** ###################################################################*/
/*!
** @file lin1.h
** @version 01.00
*/         
/*!
**  @addtogroup lin1_module lin1 module documentation
**  @{
*/         

/*!
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, global macro not referenced
 * There are global macros defined to be used by the integrator and another one used as include guard.
 */

#ifndef LIN1_H
#define LIN1_H
/* MODULE lin1. */

#include <stdint.h>
#include "lin_driver.h"


/*! @brief Device instance number */
#define INST_LIN1 (2U)

/*! Driver state structure */
extern lin_state_t lin1_State;

/*! @brief Configuration declaration */
extern lin_user_config_t lin1_InitConfig0;

#endif
/* ifndef LIN1_H */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the Freescale S32K series of microcontrollers.
**
** ###################################################################
*/
