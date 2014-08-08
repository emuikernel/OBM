/******************************************************************************
 *
 *  (C)Copyright 2013 Marvell. All Rights Reserved.
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

 #ifndef __RT9455_H__
 #define __RT9455_H__

#include "Typedef.h"
#include "timer.h"
#include "USB1.h"

/************************************************************************************************/
/* 						The address of Intersil RT9455 Charger IC.									*/
/************************************************************************************************/
#define RT9455_CONTRL1   		0x00   
/* The bit field defines in CONTRL0 Register.*/
#define TMR_RST						0x80					/* Reset t32 timer.						*/
#define EN_STAT						0x40                    /* Enables STAT=Low When in charing.	*/
#define STAT_CHARG_READY				0x00
#define STAT_CHARG_INPROCESS			0x10
#define STAT_CHARG_DONE					0x20
#define BOOST_ENABLE					0x04					/* IC in Boost Mode.					*/

#define RT9455_CONTRL2   		0x01   
/* The bit field defines in CONTRL1 Register.*/
#define IINLIM_NOLIM					0xC0					/* Input I = No limit.					*/
#define IINLIM_1A						0x80					/* Input I = 1	  A.					*/
#define IINLIM_500mA					0x40					/* Input I = 500 mA					*/
#define IINLIM_100mA					0x00					/* Input I = 100 mA.					*/
#define RT_TE							0x08					/* Enalbe charger current termination.		*/
#define IAICR_INT						0x04					/* IAICR setting bit.					*/
#define RT_HZ_MODE						0x02					/* High-Impedence Mode.				*/
#define OPA_MODE						0x01					/* Boost Mode.						*/ 

#define RT9455_CONTRL3			0x02
#define OREG_3V54           			0x02 << 2				/* Charger Output = 3.54v.				*/
#define OREG_4V18						0x22 << 2
#define OREG_4V20						0x23 << 2
#define OREG_4V22						0x24 << 2
#define OREG_4V35						0x2a << 2
#define OREG_4V44						0x2f << 2
#define OTG_PL 							0x02					/* OTG pin = 1.						*/
#define OTG_EN							0x01					/* Enable OTG pin.						*/

#define RT9455_DEVICE_ID			0x03

#define RT9455_IBAT			0x04

#define RT9455_CONTRL5		0x05
#define EN_TMR				(1 << 7)

#define RT9455_CONTRL6			0x06
#define ICHRG_500mA				(0 << 4)
#define ICHRG_650mA				(1 << 4)
#define ICHRG_800mA				(2 << 4)
#define ICHRG_950mA				(3 << 4)
#define ICHRG_1100mA			(4 << 4)
#define ICHRG_1250mA			(5 << 4)

#define RT9455_CONTRL7			0x07
#define CHG_EN					BIT4
#define VMREG_4V20				0x00
#define VMREG_4V36				0x08
#define VMREG_4V44				0x0C

/*****************************************************************************/
void RT9455Write(UINT8_T reg_addr, UINT8_T value);
UINT8_T RT9455Read(UINT8_T reg_addr);
void EnableRT9455Charger(void);
void DisableRT9455Charger(void);
void resetTimer(void);
void ConfigRT9455Charger(UINT8_T bat_connected_status);
void Charger_Voltage_Set(UINT8_T voltage);


 #endif
 
