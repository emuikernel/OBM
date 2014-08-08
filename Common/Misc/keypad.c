
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
 
 ******************************************************************************
**
**  FILENAME:   keypad.c
**
**  PURPOSE:    keypad support: init, read
**
**  History:    Initial Creation 11/08
******************************************************************************/

#include "misc.h"
#include "KPC.h"


#define KPC_PCAS        (1 << 30)    // Force Automatic Scan
#define KPC_ASACT       (1 << 29)    // Automatic Scan on Activity bit
#define KPC_MKRN        (7 << 26)    // Number of Keypad Rows - 1 
#define KPC_MKCN        (7 << 23)    // Number of Keypad Columns - 1
#define KPC_MI          (1 << 22)    // Matrix interupt bit
#define KPC_IMKP        (1 << 21)    // Ignore Multiple Keypad Press bit
#define KPC_MS7         (1 << 20)    // Matrix Scan Line 7
#define KPC_MS6         (1 << 19)    // Matrix Scan Line 6
#define KPC_MS5         (1 << 18)    // Matrix Scan Line 5
#define KPC_MS4         (1 << 17)    // Matrix Scan Line 4
#define KPC_MS3         (1 << 16)    // Matrix Scan Line 3
#define KPC_MS2         (1 << 15)    // Matrix Scan Line 2
#define KPC_MS1         (1 << 14)    // Matrix Scan Line 1
#define KPC_MS0         (1 << 13)    // Matrix Scan Line 0
#define KPC_ME          (1 << 12)    // Matrix Keypad Enable bit
#define KPC_MIE         (1 << 11)    // Matrix Keypad Interrupt Enable bit
#define KPC_DKN         (2 <<  6)    // Number of direct keypad inputs -1
#define KPC_DI          (1 <<  5)    // Direct keypad interrupt bit
#define KPC_REE0        (1 <<  2)    // Rotary encoder 0 enable bit
#define KPC_DE          (1 <<  1)    // Direct Keypad Enable bit

#define	KPC_FNCLKSEL_32KHZ	(0<<4)		// functional clock select: 0=32KHz, 1=16KHz, 2=26MHz
#define	KPC_RST				(0<<2)		// reset: 0=active, 1=hold in reset
#define	KPC_FNCLK			(1<<1)		// kpc functional clock: 0=clock off, 1=clock on
#define	KPC_APBCLK			(1<<0)		// kpc apb clock: 0=clock off, 1=clock on

#define KPC_PC_ASPEN	\
(						\
	KPC_ASACT    |      \
	KPC_MKRN     |      \
	KPC_MKCN     |      \
	KPC_MS7      |      \
	KPC_MS6      |      \
	KPC_MS5      |      \
	KPC_MS4      |      \
	KPC_MS3      |      \
	KPC_MS2      |      \
	KPC_MS1      |      \
	KPC_MS0      |      \
	KPC_ME              \
)

void InitializeKeypad() 
{

	*(volatile unsigned long*)KPC_PC = 0xd2; // direct key number is 4, not 3, even DKIN[0] is not connected
    *(volatile unsigned long*)KPC_REC = 0x7f007f;
    *(volatile unsigned long*)KPC_KDI = 0x1;

    Delay(20 * 63);
    // force to do a scan, otherwise read will fail to detect
    *(volatile unsigned long*)KPC_PC = ( *(volatile unsigned long*)KPC_PC ) | (KPC_PCAS);
    Delay(20 * 63);
}



// return 1 if a key is available.
unsigned long ReadKeypad( unsigned int *k ) 
{
	unsigned int key, Temp, RegisterValue;
	
#if MIFI_V3R1
	*(VUINT_T *)(GPIO1_BASE + GPIO_CDR) |= BIT24;
	Temp = *(VUINT_T *)(GPIO1_BASE + GPIO_CDR);
	Temp = *(VUINT_T *)(GPIO1_BASE + GPIO_PLR);

	RegisterValue = *(VUINT_T *)(GPIO1_BASE + GPIO_PLR);
	if (((RegisterValue >> 24) & 1) == 0)
	{
		*k = KEYID_SOFTWAREUPGRADEREQUEST;
		return 1;
	}
	else
	{
		*k = 0xff;
		return 0;
	}

#else
    // steps:
    // 1. initiate a scan
    // 2. wait for the scan to complete
    // 3. examine the register for indications and details of key present

    // noted that some times the kpc_pcas fails to clear.
    // need to debug. for now, since auto scan is on, just read the auto scan result register (kpc_as)
    *(volatile unsigned long*)KPC_PC = ( *(volatile unsigned long*)KPC_PC ) | (KPC_PCAS);

    Delay(20 * 63);

    // brownstone use direct key
    key = (*(volatile unsigned int *)KPC_DK);
    key &= 0xff;
    if (key != 0xf) {
        *k = key;
        return 1;
    }
	else
	{
		*k = 0xff;
		return 0;
	}
#endif
}

