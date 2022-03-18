/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : watchdog1.h
**     Project     : FUXIN_S32K148
**     Processor   : S32K148_176
**     Component   : wdog
**     Version     : Component SDK_S32K1xx_15, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32K1xx_15
**     Compiler    : GNU C Compiler
**     Date/Time   : 2022-03-17, 14:54, # CodeGen: 0
**     Contents    :
**         WDOG_DRV_Init             - status_t WDOG_DRV_Init(uint32_t instance, const wdog_user_config_t *...
**         WDOG_DRV_Deinit           - status_t WDOG_DRV_Deinit(uint32_t instance);
**         WDOG_DRV_GetConfig        - void WDOG_DRV_GetConfig(uint32_t instance, wdog_user_config_t * const config);
**         WDOG_DRV_GetDefaultConfig - void WDOG_DRV_GetDefaultConfig(wdog_user_config_t * const config);
**         WDOG_DRV_SetInt           - status_t WDOG_DRV_SetInt(uint32_t instance,bool enable);
**         WDOG_DRV_ClearIntFlag     - void WDOG_DRV_ClearIntFlag(uint32_t instance);
**         WDOG_DRV_Trigger          - void WDOG_DRV_Trigger(uint32_t instance);
**         WDOG_DRV_GetCounter       - uint16_t WDOG_DRV_GetCounter(uint32_t instance);
**         WDOG_DRV_SetWindow        - status_t WDOG_DRV_SetWindow(uint32_t instance, bool enable, uint16_t...
**         WDOG_DRV_SetMode          - status_t WDOG_DRV_SetMode(uint32_t instance, bool enable, wdog_set_mode_t...
**         WDOG_DRV_SetTimeout       - status_t WDOG_DRV_SetTimeout(uint32_t instance, uint16_t timeout);
**         WDOG_DRV_SetTestMode      - status_t WDOG_DRV_SetTestMode(uint32_t instance, wdog_test_mode_t testMode);
**         WDOG_DRV_GetTestMode      - wdog_test_mode_t WDOG_DRV_GetTestMode(uint32_t instance);
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
** @file watchdog1.h
** @version 01.00
*/         
/*!
**  @addtogroup watchdog1_module watchdog1 module documentation
**  @{
*/         
#ifndef watchdog1_H
#define watchdog1_H

/* MODULE watchdog1.
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, Global macro not referenced.
 * The global macro will be used in function call of the module.
 */

/* Include inherited beans */
#include "clockMan1.h"
#include "Cpu.h"
#include "wdog_driver.h"

/*! @brief Device instance number */
#define INST_WATCHDOG1 0U

extern wdog_user_config_t watchdog1_Config0;

#endif
/* ifndef watchdog1_H */
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
