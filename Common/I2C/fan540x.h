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
#ifndef __FAN540X_H__
#define __FAN540X_H__

#include "Typedef.h"
#include "timer.h"
#include "USB1.h"

/************************************************************************************************/
/* 						The address of Intersil FAN5405 Charger IC.									*/
/************************************************************************************************/
#define FAN5405_CONTRL0   		0x00   
/* The bit field defines in CONTRL0 Register.*/
#define TMR_RST_OTG						0x80					/* Reset t32 timer.						*/
#define FAN_EN_STAT						0x40                    /* Enables STAT=Low When in charing.	*/
#define STAT_CHARG_READY				0x00
#define STAT_CHARG_INPROCESS			0x10
#define STAT_CHARG_DONE					0x20
#define BOOST_ENABLE					0x04					/* IC in Boost Mode.					*/

#define FAN5405_CONTRL1   		0x01   
/* The bit field defines in CONTRL1 Register.*/
#define IINLIM_NOLIM					0xC0					/* Input I = No limit.					*/
#define IINLIM_800mA					0x80					/* Input I = 800 mA.					*/
#define IINLIM_500mA					0x40					/* Input I = 500 mA					*/
#define IINLIM_100mA					0x00					/* Input I = 100 mA.					*/
#define VLOW_3V7						0x30					/* VLOW = 3V7.						*/
#define VLOW_3V6						0x20					/* VLOW = 3V6.						*/
#define VLOW_3V5						0x10					/* VLOW = 3V5.						*/
#define VLOW_3V4						0x30					/* VLOW = 3V4.						*/
#define FAN_TE							0x08					/* Enalbe charger current termination.		*/
#define FAN_CEn							0x04					/* Charger Disable.					*/
#define FAN_HZ_MODE						0x02					/* High-Impedence Mode.				*/
#define OPA_MODE						0x01					/* Boost Mode.						*/ 

#define FAN5405_OREG			0x02
#define OREG_3V54           			0x02 << 2				/* Charger Output = 3.54v.				*/
#define OREG_4V18						0x22 << 2
#define OREG_4V20						0x23 << 2
#define OREG_4V22						0x24 << 2
#define OTG_PL 							0x02					/* OTG pin = 1.						*/
#define OTG_EN							0x01					/* Enable OTG pin.						*/

#define FAN5405_IC_INFO			0x03

#define FAN5405_IBAT			0x04
#define FAN_RESET							0x80
#define IOCHARGE_550mA					0x00 << 4				/* Ichange = 550mA.					*/
#define IOCHARGE_650mA					0x01 << 4				/* Ichange = 650mA.					*/
#define IOCHARGE_750mA					0x02 << 4				/* Ichange = 750mA.					*/
#define IOCHARGE_850mA					0x03 << 4				/* Ichange = 850mA.					*/
#define IOCHARGE_950mA					0x04 << 4				/* Ichange = 950mA.					*/
#define IOCHARGE_1050mA					0x05 << 4				/* Ichange = 1050mA.					*/
#define IOCHARGE_1150mA					0x06 << 4				/* Ichange = 1150mA.					*/
#define IOCHARGE_1250mA					0x07 << 4				/* Ichange = 1250mA.					*/
#define ITERM_49mA						0x00					/* Iterm = 49mA.						*/
#define ITERM_97mA						0x01					/* Iterm = 49mA.						*/
#define ITERM_146mA						0x02					/* Iterm = 49mA.						*/
#define ITERM_194mA						0x03					/* Iterm = 49mA.						*/
#define ITERM_243mA						0x04					/* Iterm = 49mA.						*/
#define ITERM_291mA						0x05					/* Iterm = 49mA.						*/
#define ITERM_340mA						0x06					/* Iterm = 49mA.						*/
#define ITERM_388mA						0x07					/* Iterm = 49mA.						*/

#define FAN5405_SP_CHANGER		0x05
#define DIS_CHANGER						0x40					/* 1.8V regulator is OFF.				*/
#define IO_LEVEL						0x20					/* Voltage across Rsense = 22.1mV.		*/
#define VSP_4V213						0x00					/* Vsp = 4.213v.						*/
#define VSP_4V293						0x01					/* Vsp = 4.293v.						*/
#define VSP_4V373						0x02					/* Vsp = 4.373v.						*/
#define VSP_4V453						0x03					/* Vsp = 4.453v.						*/
#define VSP_4V533						0x04					/* Vsp = 4.533v.						*/
#define VSP_4V613						0x05					/* Vsp = 4.613v.						*/
#define VSP_4V693						0x06					/* Vsp = 4.693v.						*/
#define VSP_4V773						0x07					/* Vsp = 4.773v.						*/

#define FAN5405_SAFETY			0x06
#define VSAFE_4V20						0x00					/* Vsafe = 4.20v.						*/
#define VSAFE_4V22						0x01					/* Vsafe = 4.22v.						*/
#define VSAFE_4V24						0x02					/* Vsafe = 4.24v.						*/
#define VSAFE_4V26						0x03					/* Vsafe = 4.26v.						*/
#define VSAFE_4V28						0x04					/* Vsafe = 4.28v.						*/
#define VSAFE_4V30						0x05					/* Vsafe = 4.30v.						*/
#define VSAFE_4V32						0x06					/* Vsafe = 4.32v.						*/
#define VSAFE_4V34						0x07					/* Vsafe = 4.34v.						*/
#define VSAFE_4V36						0x08					/* Vsafe = 4.36v.						*/
#define VSAFE_4V38						0x09					/* Vsafe = 4.38v.						*/
#define VSAFE_4V40						0x0A					/* Vsafe = 4.40v.						*/
#define VSAFE_4V42						0x0B					/* Vsafe = 4.42v.						*/
#define VSAFE_4V44						0x0C					/* Vsafe = 4.44v.						*/
//#define VSAFE_4V44						0x0D					/* Vsafe = 4.44v.						*/
//#define VSAFE_4V44						0x0E					/* Vsafe = 4.44v.						*/
//#define VSAFE_4V44						0x0F						/* Vsafe = 4.44v.						*/

#define FAN5405_MONITOR 		0x10							/* Read Only Register.					*/

/*****************************************************************************/
void FAN540XWrite(UINT8_T reg_addr, UINT8_T value);
UINT8_T FAN540XRead(UINT8_T reg_addr);
void EnableFAN540XCharger(void);
void DisableFAN540XCharger(void);
void resetTimer(void);
void ConfigFAN540XCharger(UINT8_T bat_connected_status);
void Charger_Voltage_Set(UINT8_T voltage);

#endif

