/******************************************************************************
 *
 *  (C)Copyright 2005 - 2011 Marvell. All Rights Reserved.
 *
 *  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MARVELL.
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *  This Module contains Proprietary Information of Marvell and should be
 *  treated as Confidential.
 *  The information in this file is provided for the exclusive use of the
 *  licensees of Marvell.
 *  Such users have the right to use, modify, and incorporate this code into
 *  products for purposes authorized by the license agreement provided they
 *  include this notice and the associated copyright notice with any such
 *  product.
 *  The information in this file is provided "AS IS" without warranty.
 
 ******************************************************************************/
/*************************************************************
 * Timer.h
 *
 * Contents:
 *      Definitions and functions declarations used fin the
 *      Boot ROM development
 *
 *************************************************************/
#ifndef __timer_h
#define __timer_h

#include "TMR.h"
#include "PlatformConfig.h"

#define TMR_CR0	(volatile UINT_T *)TMR1_CRN
#define TIMER_CER (volatile UINT_T *)TMR1_CER
#define APB_CU_CR (volatile UINT_T *)(APBC_TIMERS1_CLK_RST)
#define TIMER_CLEAR (*TMR_CR0 = 0)
#define TIMER_GET   (*TMR_CR0)
#define TIMER_ENABLE (*TIMER_CER |= 1)

#define Delay_us(x) Delay(x)
#define WAITINMICROSECONDS  			0
#define WAITINMILLISECONDS  			1

// Prototypes

UINT_T GetOSCR0(void);
UINT_T OSCR0IntervalInSec(UINT_T Before, UINT_T After);
UINT_T OSCR0IntervalInMilli(UINT_T Before, UINT_T After);
UINT_T OSCR0IntervalInMicro(UINT_T Before, UINT_T After);
UINT_T WaitForOperationComplete(UINT_T TimeOutValue, UINT8_T WaitGranularity, VUINT_T *pAddress, UINT_T Value, UINT_T Mask);
UINT32 InitSODTimer( void );
UINT32 GetSODTimerValue( void );
UINT_T GetElapsedTimeMilliSecFromSOD( void );
UINT_T GetElapsedTimeUSecFromSOD( void );
UINT_T GetElapsedTimeSecondsFromSOD( void );
void Delay(UINT_T uSec);
#endif // Misc_h
