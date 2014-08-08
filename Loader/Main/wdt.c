/******************************************************************************
 *
 *  (C)Copyright 2013 Marvell Hefei Branch. All Rights Reserved.
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
 ******************************************************************************/

#include "wdt.h"

#define WDT_GEN_INT     0
#define WDT_GEN_RESET   1

static void wdt_access(void)
{
	*(VUINT_T *)WDT_WFAR = 0xbaba;
	*(VUINT_T *)WDT_WSAR = 0xeb10;
}

static void wdt_enable(unsigned int reset_int)
{
	UINT_T temp;

    wdt_access();
    *(VUINT_T *)WDT_WMER = 0x3;

	Delay(100);
	temp = (*(VUINT_T *)WDT_WMER);
	Delay(100);
}

static void wdt_set_match(unsigned int match)
{
	UINT_T temp;

    wdt_access();
	*(VUINT_T *)WDT_WMR = 0xffff & match;

	Delay(100);
	temp = (*(VUINT_T *)WDT_WMR);
	Delay(100);
}

static void wdt_reset_counter(void)
{
    wdt_access();
	*(VUINT_T *)WDT_CER = 0x7;

	wdt_access();
    *(VUINT_T *)WDT_WCR = 0x1;
}

static void wdt_irq(void *data)
{
    data = data;
    wdt_access();
    *(VUINT_T *)WDT_WICR = 1; // clear wdt int

    wdt_access();
    *(VUINT_T *)WDT_WMER &= ~0x1; // disable wdt
}

void wdt_test(unsigned int seconds, unsigned int reset_int)
{
    // wdt 13Mhz
    *(VUINT_T *)PMUM_WDTPCR = 0x7;
    Delay(10);
    *(VUINT_T *)PMUM_WDTPCR = 0x3;
    Delay(100);
    *(VUINT_T *)PMUM_APRR |= 0x10;

    EnablePeripheralIRQInterrupt(INT_CP_TMR1);

    wdt_reset_counter();

    wdt_set_match(seconds * 256);
    wdt_enable(reset_int);

	serial_outstr("----------------------------- WDT rest --------------\n");

	while (1);
}

void do_wdt_reset(void)
{
	wdt_test(2, WDT_GEN_RESET);
}

#if ZIMI_PB05
void zimi_wdt_test(unsigned int seconds, unsigned int reset_int)
{
    // wdt 13Mhz
    *(VUINT_T *)PMUM_WDTPCR = 0x7;
    Delay(10);
    *(VUINT_T *)PMUM_WDTPCR = 0x3;
    Delay(100);
    *(VUINT_T *)PMUM_APRR |= 0x10;

    EnablePeripheralIRQInterrupt(INT_CP_TMR1);

    wdt_reset_counter();

    wdt_set_match(seconds * 256);
    wdt_enable(reset_int);
	
}

void zimi_do_wdt_reset(void)
{
	zimi_wdt_test(3, WDT_GEN_RESET);
}
#endif
