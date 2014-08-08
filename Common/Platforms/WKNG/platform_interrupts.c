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
**  FILENAME:	Interrupts.c
**
**  PURPOSE: 	Contain Interrupts/Exception handlers for Bulverde B0 Boot ROM
**
******************************************************************************/

//#if (BOOTROM || TRUSTED)
// #include "dsvl.h"
//#endif

#include "predefines.h"
#include "Interrupts.h"
#include "PlatformConfig.h"
#include "ProtocolManager.h"
#include "USB1.h"
#include "resume.h"

static UINT8_T UdcDmaUsageFlag   = FALSE;  // Used to enable/disable DMA usage
static UINT8_T PortInterrupt	 = FALSE;

UINT8_T GetPortInterruptFlag(void){ return PortInterrupt;}

void SetPortInterruptFlag(void){ PortInterrupt = TRUE;}

void ClearPortInterruptFlag(void)
{
	PortInterrupt = FALSE;
	return;
}

void IRQ_Glb_Ena()
{
	BU_U32	reg;
	UINT_T CPU_Mode;
	UINT_T ID_Mask;

	ID_Mask = 0x0000fff0;

 	getCPUMode( &CPU_Mode, &ID_Mask);
	if (CPU_Mode == ARM926ID)	// AP = 926
    {
    	reg = BU_REG_READ(ICU_AP_GBL_IRQ_MSK);
    	reg &= (~ICU_AP_GBL_IRQ_MSK_IRQ_MSK);
		BU_REG_WRITE( ICU_AP_GBL_IRQ_MSK, reg);
	}
	else
	{
	    reg = BU_REG_READ(ICU_CP_GBL_IRQ_MSK);
    	reg &= (~ICU_CP_GBL_IRQ_MSK_IRQ_MSK);
		BU_REG_WRITE( ICU_CP_GBL_IRQ_MSK, reg);
	}

	/*
	* AP timer2 is used to switch USB interrupt and HSIC interrupt in BootROM,
	* and OBM doesn't use it.
	* Disable AP timer2 interrupt since it will affect USB interrupt.
	*/
	DisablePeripheralIRQInterrupt(INT_AP_TMR2);
}

// They should have been routed to the proper processor already in INT_init.
void EnableInt(unsigned int int_num)
{
       BU_REG_WRITE(ICU_INT_0_TO_63_CONF+int_num*4, (BU_REG_READ(ICU_INT_0_TO_63_CONF+int_num*4) | \
                                                                 (IRQ_INT<<4 | BROM_PRIORITY | ICU_INT_0_TO_63_CONF_AP_INT)));
}

void DisableInt(BU_U32 int_num)
{
   	BU_REG_WRITE(ICU_INT_0_TO_63_CONF+int_num*4, 0 );
}

UINT_T EnablePeripheralIRQInterrupt(unsigned int InterruptID)
{
    EnableInt(InterruptID);
	return(NoError);
}

UINT_T DisablePeripheralIRQInterrupt(unsigned int InterruptID)
{
    DisableInt(InterruptID);
	return(NoError);
}

unsigned int GetIrqStatus(void)
{
	UINT_T Regval = 0;
	BU_U32	reg;
	UINT_T CPU_Mode;
	UINT_T ID_Mask;

	ID_Mask = 0x0000fff0;

 	getCPUMode( &CPU_Mode, &ID_Mask);
	if (CPU_Mode == ARM926ID)	// AP = 926
		Regval = BU_REG_READ(ICU_AP_IRQ_SEL_INT_NUM);
	else
		Regval = BU_REG_READ(ICU_CP_IRQ_SEL_INT_NUM);

    return Regval;
}

void IrqHandler(void)
{
    volatile unsigned int int_num, regVal;
	void *handle;

    int_num = GetIrqStatus();
    if ( int_num &  ICU_AP_IRQ_SEL_INT_NUM_INT_PENDING )
    {
 		/* mask out the pending bit to get low 5 bit INT num */
		int_num &= ~ICU_AP_IRQ_SEL_INT_NUM_INT_PENDING;
	
		switch (int_num)
		{
			// we don't use them for now
		#if 0
			case INT_MIPI_HSI:
				HSI_ISR();
				break;
			case INT_UART2_Fast:
			{
				PortInterrupt = TRUE;
				FFUARTInterruptHandler();
				break;
			}
		#endif
			case INT_USB:
			{
			#if USBCI
				PortInterrupt = TRUE;
	            PlatformCI2InterruptHandler();
			#endif
				break;
			}
			case INT_MMC:
			{
#if MMC_CODE
				SDMMC_ISR();
#endif
				break;
			}
		}
    }
}


void UndefinedHandler(void)
{
serial_outstr("UndefinedHandler\n");
	// Terminate, simply loop forever for now
	while (1){}
}

void SwiHandler(void)
{
	// Terminate, simply loop forever for now
	while (1){}
}

void PrefetchHandler(void)
{
	// Terminate, simply loop forever for now
	while (1){}
}

void AbortHandler(void)
{
serial_outstr("AbortHandler\n");
	// Terminate, simply loop forever for now
	while (1){}
}


void FiqHandler(void)
{
   while (1){}
   //return;
}


void INT_init()
{
	int i;
	UINT_T CPU_Mode;
	UINT_T ID_Mask;

	ID_Mask = 0x0000fff0;

 	getCPUMode( &CPU_Mode, &ID_Mask);
	if (CPU_Mode == ARM926ID)	// AP = 926
	{

	   	/* disable AP global IRQ+FIQ */

			BU_REG_WRITE(ICU_AP_GBL_IRQ_MSK, ICU_AP_GBL_IRQ_MSK_IRQ_MSK | ICU_AP_GBL_IRQ_MSK_FIQ_MSK);

		/* clean up 32 interrupt config register */
		for ( i=0; i<INT_NUMS; i++ )
		{
                         BU_REG_WRITE(ICU_INT_0_TO_63_CONF + i * 4, (BU_REG_READ(ICU_INT_0_TO_63_CONF + i * 4) | ICU_INT_0_TO_63_CONF_AP_INT)); 
		}
	}
	else
	{
	   	/* disable CP global IRQ+FIQ */

			BU_REG_WRITE(ICU_CP_GBL_IRQ_MSK, ICU_CP_GBL_IRQ_MSK_IRQ_MSK | ICU_CP_GBL_IRQ_MSK_FIQ_MSK);

		/* clean up 32 interrupt config register */
		for ( i=0; i<INT_NUMS; i++ )
		{
			BU_REG_WRITE(ICU_INT_0_TO_63_CONF+i*4, (BU_REG_READ(ICU_INT_0_TO_63_CONF+i*4) | ( ICU_INT_0_TO_63_CONF_CP_INT )));
		}
	}
}

#if 0
#if RVCT
__asm void getCPUMode(UINT_T *CPU_mode, UINT_T *ID_mask )
{
	mrc	p15, 0, r3, c0, c0, 0
	ldr r2, [r1];
	and r2, r3, r2
	str	r2, [r0]
}

#else
#if LINUX_BUILD
void getCPUMode(UINT_T *CPU_mode, UINT_T *ID_mask )
/* get CPU mode */
{
	asm (
		"mrc	p15, #0, r3, c0, c0, #0\n\t"
		"ldr r2, [r1]\n\t"
		"and r2, r3, r2\n\t"
		"str	r2, [r0]"
	);
}

#else
void getCPUMode(UINT_T *CPU_mode, UINT_T *ID_mask )
/* get CPU mode */
{
	__asm {
		mrc	p15, 0, r3, c0, c0, 0
		ldr r2, [r1];
		and r2, r3, r2
		str	r2, [r0]
	}
}
#endif
#endif
#endif


