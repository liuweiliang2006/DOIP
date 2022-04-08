/*
 * v_def.h
 *
 *  Created on: 2021Äê9ÔÂ24ÈÕ
 *      Author: Administrator
 */

#ifndef DIAG_V_DEF_H_
#define DIAG_V_DEF_H_

#ifndef  V_DEF_H
#define  V_DEF_H

/***************************************************************************/
/* Version                  (abcd: Main version=ab, Sub Version=cd )       */
/***************************************************************************/

/* ##V_CFG_MANAGEMENT ##CQProject : Common_Vdef CQComponent : Implementation */
#define COMMON_VDEF_VERSION            0x0354
#define COMMON_VDEF_RELEASE_VERSION    0x00

/* compatibility for IL versions < 3.52 */
#define V_DEF_VERSION                  COMMON_VDEF_VERSION

/***************************************************************************/
/***************************************************************************/
/****  Hardware independent settings  **************************************/
/***************************************************************************/
/***************************************************************************/
/*--- standard memory qualifier definition --------------------------------*/

/* 8-Bit qualifier */
#if !defined( vuint8 ) /* ASR compatibility */
typedef unsigned char  vuint8;
#endif
#define canuint8 vuint8

#if !defined( vsint8 ) /* ASR compatibility */
typedef signed char    vsint8;
#endif
#define cansint8 vsint8

/* 16-Bit qualifier */
#if !defined( vuint16 ) /* ASR compatibility */
typedef unsigned short vuint16;
#endif
#define canuint16 vuint16

#if !defined( vsint16 ) /* ASR compatibility */
typedef signed short   vsint16;
#endif
#define cansint16 vsint16

/* 32-Bit qualifier */
#if !defined( vuint32 ) /* ASR compatibility */
typedef unsigned long  vuint32;
#endif
#define canuint32 vuint32

#if !defined( vsint32 ) /* ASR compatibility */
typedef signed long    vsint32;
#endif
#define cansint32 vsint32

typedef unsigned char *TxDataPtr;              /* ptr to transmission data buffer */
typedef unsigned char *RxDataPtr;              /* ptr to receiving data buffer    */

#ifndef MEMORY_HUGE
#  define MEMORY_HUGE               /* no entry                         */
#endif

/* *********************** default defines - used to store permanent data **************************************** */
#ifndef V_MEMROM0
# define V_MEMROM0                  /* addition qualifier data access in ROM  */
#endif

#ifndef V_MEMROM1_NEAR
# define V_MEMROM1_NEAR             /* fast data access in ROM */
#endif

#ifndef V_MEMROM1
# define V_MEMROM1                 /* fast data access in ROM */
#endif

#ifndef V_MEMROM1_FAR
# define V_MEMROM1_FAR             /* slow addressing mode in ROM */
#endif

#ifndef MEMORY_ROM_NEAR
# ifndef V_MEMROM2_NEAR
#  define V_MEMROM2_NEAR   const    /* fast data access in ROM */
# endif
  /* compatibility for modules which use old definition of memory qualifer */
# define MEMORY_ROM_NEAR   V_MEMROM2_NEAR
#else
# define V_MEMROM2_NEAR    MEMORY_ROM_NEAR
#endif

#ifndef MEMORY_ROM
# ifndef V_MEMROM2
#  define V_MEMROM2        const    /* fast data access in ROM */
# endif
  /* compatibility for modules which use old definition of memory qualifer */
# define MEMORY_ROM        V_MEMROM2
#else
# define V_MEMROM2         MEMORY_ROM
#endif

#ifndef MEMORY_ROM_FAR
# ifndef V_MEMROM2_FAR
#  define V_MEMROM2_FAR    const    /* slow addressing mode in ROM */
# endif
  /* compatibility for modules which use old definition of memory qualifer */
# define MEMORY_ROM_FAR    V_MEMROM2_FAR
#else
# define V_MEMROM2_FAR     MEMORY_ROM_FAR
#endif

#ifndef V_MEMROM3
# define V_MEMROM3
#endif

/* *********************** default defines - used to store volatile data **************************************** */
#ifndef V_MEMRAM0
# define V_MEMRAM0                  /* addition qualifier data access in RAM  */
#endif

#ifndef V_MEMRAM1_NEAR
# define V_MEMRAM1_NEAR             /* fast data access in RAM */
#endif

#ifndef V_MEMRAM1
# define V_MEMRAM1                 /* fast data access in RAM */
#endif

#ifndef V_MEMRAM1_FAR
# define V_MEMRAM1_FAR             /* slow addressing mode in RAM */
#endif

#ifndef MEMORY_NEAR
# ifndef V_MEMRAM2_NEAR
#  define V_MEMRAM2_NEAR           /* fast data access in RAM */
# endif
  /* compatibility for modules which use old definition of memory qualifer */
# define MEMORY_NEAR   V_MEMRAM2_NEAR
#else
# define V_MEMRAM2_NEAR    MEMORY_NEAR
#endif

#ifndef MEMORY_NORMAL
# ifndef V_MEMRAM2
#  define V_MEMRAM2                 /* fast data access in RAM */
# endif
  /* compatibility for modules which use old definition of memory qualifer */
# define MEMORY_NORMAL   V_MEMRAM2
#else
# define V_MEMRAM2           MEMORY_NORMAL
#endif

#ifndef MEMORY_FAR
# ifndef V_MEMRAM2_FAR
#  define V_MEMRAM2_FAR            /* slow addressing mode in RAM */
# endif
  /* compatibility for modules which use old definition of memory qualifer */
# define MEMORY_FAR    V_MEMRAM2_FAR
#else
# define V_MEMRAM2_FAR     MEMORY_FAR
#endif

#ifndef V_MEMRAM3_NEAR
# define V_MEMRAM3_NEAR             /* fast data access in RAM */
#endif

#ifndef V_MEMRAM3
# define V_MEMRAM3                 /* fast data access in RAM */
#endif

#ifndef V_MEMRAM3_FAR
# define V_MEMRAM3_FAR             /* slow addressing mode in RAM */
#endif

#define kTpSuccess            0  /*Everythings fine*/
#define kTpFailed             1  /*Error*/
#define kTpBusy               3  /*tpTransmit while tp is running*/
#define kTpTxBufferUnderrun   4  /*Not enough data to send*/

#if !defined(V_NULL)
# define V_NULL ((void*)0)
#endif

#endif /* V_DEF_H */

#endif /* DIAG_V_DEF_H_ */
