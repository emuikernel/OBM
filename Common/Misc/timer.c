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
 *
 *  Module: timer.c
 ******************************************************************************/

#include "Typedef.h"
#include "timer.h"
#include "APBC.h"

static unsigned int         misc_SOD_OSCR0;

UINT_T GetOSCR0(void)
{
	unsigned long	tmp;
    *(volatile UINT_T *) TMR1_CVWRN = 1;		// latch the value

	tmp = *(volatile UINT_T*)TMR1_CVWRN;		// discard this read - part of handling slow clock domain crossing.
	tmp = *(volatile UINT_T*)TMR1_CVWRN;		// discard this read - part of handling slow clock domain crossing.
	tmp = *(volatile UINT_T*)TMR1_CVWRN;		// discard this read - part of handling slow clock domain crossing.
	tmp = *(volatile UINT_T*)TMR1_CVWRN;		// discard this read - part of handling slow clock domain crossing.
	tmp = *(volatile UINT_T*)TMR1_CVWRN;		// discard this read - part of handling slow clock domain crossing.

    return (*(volatile UINT_T *) TMR1_CVWRN);
}

UINT_T OSCR0IntervalInSec(UINT_T Before, UINT_T After)
{
	UINT_T divider = 1;
	UINT_T temp = (After - Before);
	divider = (TIMER_FREQ(divider));
	return (temp / (divider * 1000000));
}

UINT_T OSCR0IntervalInMilli(UINT_T Before, UINT_T After)
{
	UINT_T divider = 1;
	UINT_T temp = (After - Before);
	divider = (TIMER_FREQ(divider));
	return (temp / (divider * 1000));
}

UINT_T OSCR0IntervalInMicro(UINT_T Before, UINT_T After)
{
	UINT_T divider = 1;
	UINT_T temp = (After - Before);
	divider = (TIMER_FREQ(divider));
	return (temp / divider);
}


//-----------------------------
// Start of Day Timer Functions
//
//-----------------------------
UINT32 InitSODTimer( void )
{
	APB_CU_ENABLE;				  // 0xD401_5024 = 0x33
	TIMER_ENABLE;				  // 0xD401_4084 |= 1
 	misc_SOD_OSCR0 = GetOSCR0();  // Dummy read to flush potentially bad data
	misc_SOD_OSCR0 = GetOSCR0();
 	return misc_SOD_OSCR0;
}

UINT32 GetSODTimerValue( void )
{
  return misc_SOD_OSCR0;
}

UINT_T GetElapsedTimeMilliSecFromSOD( void )
{
  UINT32 currentTime;

  currentTime = GetOSCR0();  // Dummy read to flush potentially bad data
  currentTime = GetOSCR0(); 
  
  return( OSCR0IntervalInMilli(misc_SOD_OSCR0, currentTime) );
}

UINT_T GetElapsedTimeUSecFromSOD( void )
{
  UINT32 currentTime;

  currentTime = GetOSCR0();  // Dummy read to flush potentially bad data
  currentTime = GetOSCR0(); 
  
  return( OSCR0IntervalInMicro(misc_SOD_OSCR0, currentTime) );
}

UINT_T GetElapsedTimeSecondsFromSOD( void )
{
  UINT32 currentTime;

  currentTime = GetOSCR0();  // Dummy read to flush potentially bad data
  currentTime = GetOSCR0(); 
  
  return( OSCR0IntervalInSec(misc_SOD_OSCR0, currentTime) );
}

/******************************************************************************
 * void Delay(int usec)
 *
 *   Description:  Delays usec microseconds (accurate to within ~500 usecs)
 *   Parameters:    usec - time to delay in microseconds
 *   Return: none
 ******************************************************************************/
void Delay(UINT_T uSec)
{
	UINT_T startTime, endTime;
	// Convert input usec to clock ticks using TIMER_FREQ macro
	// Clock ticks = ClockFreqInMHz * usec (e.g. 3.25MHz*usec)
	UINT_T requestedDelay = (TIMER_FREQ(uSec));
	UINT_T delay = 0;
	
	startTime = GetOSCR0();  // Dummy read to flush potentially bad data
	startTime = GetOSCR0();
	do
	{
		endTime = GetOSCR0();
		if (endTime < startTime)
		{
			endTime += (0x0 - startTime);
		}
	   	delay = endTime - startTime;
	} while (delay <= requestedDelay);

	return;
}

/******************************************************************************
 * UINT_T WaitForOperationComplete(UINT_T TimeOutMillisec )
 *
 *   Description:  Waits for a generic operation to complete
 *   Parameters:   TimeOut - time to delay in milli or micro seconds
 * 				   WaitGranularity - 0 means MicroSeconds, 1 means MilliSeconds
 *				   VUINT_T - *pAddress
 *				   UINT_T  - Value		
 *				   UINT_T  - Mask	
 *   Return: none
 ******************************************************************************/

/* Generic WaitForOperationComplete Function */
UINT_T WaitForOperationComplete(UINT_T TimeOutValue, UINT8_T WaitGranularity, VUINT_T *pAddress, UINT_T Value, UINT_T Mask)
{
  UINT32 startTime, endTime, timeWaited = 0;

  startTime = GetOSCR0();
  startTime = GetOSCR0();

  do
  {
	if ((*pAddress & Mask) == Value)
		return NoError;

	endTime = GetOSCR0();

	if (endTime < startTime)
		endTime += (0x0 - startTime);      

	if (WaitGranularity == 0)
		timeWaited = OSCR0IntervalInMicro(startTime, endTime);
	else
		timeWaited = OSCR0IntervalInMilli(startTime, endTime);
  }
  while( timeWaited < TimeOutValue );

  return TimeOutError;
}   
