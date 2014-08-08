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

#ifndef _USTICA_H_
#define _USTICA_H_

#include "predefines.h"
#include "Typedef.h"
#include "timer.h"

typedef enum
{
	PowerUP_Battery			=		0,
	PowerUP_ONKEY			=		1,
	PowerUP_Charger			=		2,
	PowerUP_SystemEnable	=		3,
	PowerUP_USB				=		4,
	PowerUP_Reset			=		5,
	PowerUP_External		=		6,
	PowerUP_JIG				=		7,
	PowerUP_Unkown			=		0xff
}PowerUPType_t;

typedef enum
{
	External_Power_NotConnect		=	0,
	External_Power_Connect			=	1,
	USB_Invalide_Connect			=	2,
	USB_Connect						=	3,
	USB_NotConnect					=	4,
	Battery_Connect					=	5,
	Battery_NotConnect				=	6
}System_Connect_Type_T;


// Ustica base registers
#define USTICA_ID			0x00
#define USTICA_STATUS		0x01
#define USTICA_ONKEY_STATUS		(1 << 0)
#define USTICA_EXTON1_STATUS	(1 << 1) // JIG box
#define USTICA_EXTON2_STATUS	(1 << 2) // usb
#define USTICA_BAT_STATUS		(1 << 3)

#define USTICA_WAKEUP_REG	0x0d
#define USTICA_SW_PDOWN			(1 << 5)
#define USTICA_RSET_PMIC_REG	(1 << 6)

#define USTICA_POWER_UP_LOG	0x10
#define USTICA_ONKEY_WAKEUP		(1 << 0)
#define USTICA_EXTON2_WAKEUP	(1 << 1)
#define USTICA_EXTON1_WAKEUP	(1 << 2)
#define USTICA_FAULT_WAKEUP		(1 << 5)
#define USTICA_BAT_WAKEUP		(1 << 6)

#define USTICA_WATCHDOG_REG	0x1d
#define USTICA_WD_DIS			(1 << 0)

// Ustica GPADC registers
#define USTICA_MEAS_ENABLE1	0x01
#define USTICA_VINLDO_MEAS_EN	(1 << 2)
#define USTICA_VBBAT_MEAS_EN	(1 << 0)

#define USTICA_MEAS_ENABLE2	0x02
#define USTICA_INT_TEMP_MEAS_EN	(1 << 0)
#define USTICA_PMODE_MEAS_EN	(1 << 1)
#define USTICA_GPADC0_MEAS_EN	(1 << 2)
#define USTICA_GPADC1_MEAS_EN	(1 << 3)

#define USTICA_GPADC_MIS_CONG2	0x06
#define USTICA_GPADC_EN			(1 << 0)

#define USTICA_GPADC_MEAS_REG2	0x08
#define USTICA_BD_GP1_EN		(1 << 6)
#define USTICA_BD_EN			(1 << 5)
#define USTICA_BD_PREBIAS		(1 << 4)

#define USTICA_GPADC_MEAS_REG3	0x0a
#define USTICA_GP_END1			(1 << 1)
#define USTICA_GP_END0			(1 << 0)

#define USTICA_GPADC_MEAS_EN_REG2            0x02
#define USTICA_GPADC_BIAS_REG1	0x0b
#define USTICA_GPADC_BIAS_REG2	0x0c

#define USTICA_BIAS_ENABLE1		0x14
#define USTICA_GP_BIAS_OUT1		(1 << 6)

#define USTICA_VBUCK1_SET_0		0x3C
#define USTICA_VBUCK1_SET_1		0x3D
#define USTICA_VBUCK1_SET_2		0x3E
#define USTICA_VBUCK1_SET_3		0x3F

#define USTICA_VSYS_MEAS1	0x44
#define USTICA_VSYS_MEAS2	0x45
#define USTICA_GPADC0_MEAS_REG1 0x54
#define USTICA_GPADC1_MEAS_REG1 0x56
#define USTICA_BUCK4_MISC1	0x81
#define USTICA_BUCK4_PWM	(1 << 6)
#define USTICA_RTC_MISC_5	0xe7
#define USTICA_FAULT_WU		(1 << 3)
#define USTICA_FAULT_WU_EN	(1 << 2)


#define TBAT_TEM_N_5		(-5)	// -5C
#define TBAT_TEM_P_0		0	// 0C
#define TBAT_TEM_P_40		40	// 40C
#define TBAT_TEM_P_60		60	// 60C	

#define VBUS_OV_TH				6000 // 6.0V
#define VBUS_EXISTED_TH			3000 // 3.0V

void UsticaWrite(UINT8_T reg_addr, UINT8_T value);
UINT8_T UsticaRead(UINT8_T reg_addr);
UINT16_T ustica_read_volt_meas_val(UINT8_T meaReg);
void ReadBatVolt(UINT16_T * vbat);
void GetBatInstantVolt(UINT16_T *vbat, System_Connect_Type_T usb_status);
System_Connect_Type_T check_USBConnect(void);
System_Connect_Type_T check_BatteryConnect(void);
PowerUPType_t check_wakeup(UINT8_T bat_state);
UINT8_T check_BootONKey(UINT_T TimeOutValue, UINT8_T WaitGranularity);
void System_poweroff(void);
void Reset_Reg(void);
void Charger_init(void);

#endif
 
