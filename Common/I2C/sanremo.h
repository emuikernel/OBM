/******************************************************************************
 *
 *  (C)Copyright 2005 - 2012 Marvell. All Rights Reserved.
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
#ifndef _SANREMO_H_
#define _SANREMO_H_

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


/*********(0) For Battery_Charger Begin*******************************************/

#define SANREMO_ADDRESS		0x30	/*88PM8607,  (codenamed 'SanRemo') */

#define SANREMO_ID		0x00
#define SANREMO_PDOWN_STATUS	0x01
#define SANREMO_PDOWN_STATUS_B0	0x0e
#define SANREMO_OVER_TEMP_EVENT		(1 << 0)
#define SANREMO_SW_PWDOWN_EVENT		(1 << 2)
#define SANREMO_SYSEN_EVENT		(1 << 3)
#define SANREMO_WD_EVENT		(1 << 4)
#define SANREMO_LONG_ONKEY_EVENT	(1 << 5)
#define SANREMO_RTC_UNDER_VOLTAGE	(1 << 7)

#define SANREMO_STATUS_B0	0x01
#define SANREMO_STATUS	 	0x02
#define SANREMO_ONKEY_STATUS		(1 << 0)
#define SANREMO_EXTON_STATUS		(1 << 1)
#define SANREMO_CHG_STATUS		(1 << 2)
#define SANREMO_BAT_STATUS		(1 << 3)
#define SANREMO_PEN_STATUS		(1 << 4)
#define SANREMO_HEADSET_STATUS		(1 << 5)
#define SANREMO_HOOK_STATUS		(1 << 6)
#define SANREMO_MICIN_STATUS		(1 << 7)
#define SANREMO_VBUS_STATUS		(1 << 4)					/* Modified by Joshua Wang. @ 20/08/2012.		*/

#define SANREMO_INTERRUPT_STATUS1	0x03
#define SANREMO_ONKEY_INT		(1 << 0)
#define SANREMO_EXTON_INT		(1 << 1)
#define SANREMO_CHG_INT			(1 << 2)
#define SANREMO_BAT_INT			(1 << 3)
#define SANREMO_RTC_INT			(1 << 4)
#define SANREMO_CC_INT			(1 << 5)

#define SANREMO_INTERRUPT_STATUS2	0x04
#define SANREMO_VBAT_INT		(1 << 0)
#define SANREMO_VCHG_INT		(1 << 1)
#define SANREMO_VSYS_INT		(1 << 2)
#define SANREMO_TINT_INT		(1 << 3)
#define SANREMO_GPADC0_INT		(1 << 4)
#define SANREMO_GPADC1_INT		(1 << 5)
#define SANREMO_GPADC2_INT		(1 << 6)
#define SANREMO_GPADC3_INT		(1 << 7)

#define SANREMO_INTERRUPT_STATUS3	0x05
#define SANREMO_PEN_INT			(1 << 1)
#define SANREMO_HEADSET_INT		(1 << 2)
#define SANREMO_HOOK_INT		(1 << 3)
#define SANREMO_MICIN_INT		(1 << 4)
#define SANREMO_CHG_TOUT_INT	(1 << 5)
#define SANREMO_CHG_DONE_INT	(1 << 6)
#define SANREMO_CHG_FAIL_INT	(1 << 7)

#define SANREMO_INTERRUPT_ENABLE1		0x06
#define SANREMO_ONKEY_MASK		(1 << 0)
#define SANREMO_EXTON_MASK		(1 << 1)
#define SANREMO_CHG_MASK		(1 << 2)
#define SANREMO_BAT_MASK		(1 << 3)
#define SANREMO_RTC_MASK		(1 << 4)

#define SANREMO_INTERRUPT_ENABLE2		0x07
#define SANREMO_VBAT_MASK		(1 << 0)
#define SANREMO_VCHG_MASK		(1 << 1)
#define SANREMO_VSYS_MASK		(1 << 2)
#define SANREMO_TINT_MASK		(1 << 3)
#define SANREMO_GPADC0_MASK		(1 << 4)
#define SANREMO_GPADC1_MASK		(1 << 5)
#define SANREMO_GPADC2_MASK		(1 << 6)
#define SANREMO_GPADC3_MASK		(1 << 7)

#define SANREMO_INTERRUPT_ENABLE3		0x08
#define SANREMO_PEN_MASK		(1 << 1)
#define SANREMO_HEADSET_MASK		(1 << 2)
#define SANREMO_HOOK_MASK		(1 << 3)
#define SANREMO_MICIN_MASK		(1 << 4)
#define SANREMO_CHG_TIMEOUT_MASK		(1 << 5)
#define SANREMO_CHG_DONE_MASK		(1 << 6)
#define SANREMO_CHG_FAULT_MASK		(1 << 7)

#define SANREMO_RESET_OUT	0x09
#define SANREMO_SW_PDOWN		(1 << 7)
#define SANREMO_DEBOUNCE		0x0a
#define SANREMO_WAKEUP	0x0b
#define SANREMO_MISCELLANEOUS	0x0c
#define SANREMO_RESET_PMIC_REG	(1 << 6)
#define SANREMO_LDO1	0x10
#define SANREMO_LDO2	0x11
#define SANREMO_LDO3	0x12
#define SANREMO_LDO4	0x13
#define SANREMO_LDO5	0x14
#define SANREMO_LDO6	0x15
#define VLDO6_SET_SLP_2V65	(0x3 << 3)
#define VLDO6_SET_2V80	6
#define SANREMO_LDO7	0x16
#define SANREMO_LDO8	0x17
#define SANREMO_LDO9	0x18
#define SANREMO_LDO10	0x19
#define SANREMO_LDO12	0x1a
#define SANREMO_LDO14	0x1b
#define SANREMO_SLEEP_MODE1	0x1c
#define SANREMO_SLEEP_MODE2	0x1d
#define SANREMO_SLEEP_MODE3	0x1e
#define SANREMO_SLEEP_MODE4	0x1f
#define SANREMO_GO	0x20
#define SANREMO_BUCK1_GO	(1 << 0)
#define SANREMO_VBUCK1_SET_SLP	0x21
#define SANREMO_VBUCK2_SET_SLP	0x22
#define SANREMO_VBUCK3_SET_SLP	0x23
#define SANREMO_VBUCK1_SET	0x24
#define SANREMO_VBUCK2_SET	0x25
#define SANREMO_VBUCK3_SET	0x26
#define SANREMO_BUCK_CONTROLS	0x27
#define SANREMO_VIBRA_SET	0x28
#define SANREMO_VIBRA_PWM	0x29
#define SANREMO_REF_GROUP	0x2a
#define SANREMO_SUPPLIES_EN11	0x2b
#define SANREMO_SUPPLIES_EN12	0x2c
#define LDO6_EN1	BIT0
#define SANREMO_GROUP1	0x2d
#define SANREMO_GROUP2	0x2e
#define SANREMO_GROUP3	0x2f
#define SANREMO_GROUP4	0x30
#define SANREMO_GROUP5	0x31
#define SANREMO_GROUP6	0x32
#define SANREMO_SUPPLIES_EN21	0x33
#define SANREMO_SUPPLIES_EN22	0x34
#define SANREMO_LP_CONFIG1	0x35
#define SANREMO_LP_CONFIG2  0x36

#define SANREMO_AUDIO_MIC_BUTTON_DETECTION_B0		0x37
#define SANREMO_AUDIO_HEADSET_DETECTION_BO		0x38
#define SANREMO_LOW_POWER_CONFIGURATION         0x39
#define	SANREMO_SILENT_RECORD	0x3e
#define	SANREMO_POWER_UP_LOG	0x3f
#define SANREMO_ONKEY_WU_LOG		(1 << 0)
#define SANREMO_CHG_WU_LOG			(1 << 1)
#define SANREMO_EXTON_WU_LOG		(1 << 2)
#define SANREMO_SYSEN_WU_LOG		(1 << 3)
#define SANREMO_BAT_WU_LOG			(1 << 6)
#define SANREMO_MISC1	0x40
#define SANREMO_MISC1_GPIO1_DIR   (1 << 3)
#define SANREMO_MISC1_GPIO1_VAL   (1 << 4)
#define SANREMO_MISC1_GPIO2_DIR   (1 << 5)
#define SANREMO_MISC1_GPIO2_VAL   (1 << 6)

#define SANREMO_MCLK	0x41
#define SANREMO_MISC2	0x42
#define SANREMO_PLL_CTRL1	0x43
#define SANREMO_PLL_FRAC1	0x44
#define SANREMO_PLL_FRAC2	0x45
#define SANREMO_PLL_FRAC3	0x46
#define SANREMO_CC_READ		0x47
#define CC_AVG_SEL_4			SHIFT3(0x4)
#define CC_AVG_SEL_MASK			SHIFT3(0x7)


/*********(1) For Battery_Charger Begin*******************************************/       
#define SANREMO_CHG_CTRL1			0x48
#define CHARGER_MODE_MASK			SHIFT0(0x3)
#define CHARGER_MODE_CHARGE_OFF		SHIFT0(0x0)
#define CHARGER_MODE_PRE_CHARGE		SHIFT0(0x1)
#define CHARGER_MODE_FAST_CHARGE	SHIFT0(0x2)
#define ITERM_SET_MASK				SHIFT2(0x3)
#define ITERM_SET_60MA				SHIFT2(0x2)
#define ITERM_SET_40MA				SHIFT2(0x1)
#define ITERM_SET_20MA				SHIFT2(0x0)

#define VFCHG_SET_MASK				SHIFT4(0xF)
#define VFCHG_SET_4P2V				SHIFT4(0x9)

#define SANREMO_CHG_CTRL2		0x49
#define ICHG_SET_MASK			SHIFT0(0x1F)
#define ICHG_SET_500MA			SHIFT0(0x9)
#define ICHG_SET_400MA			SHIFT0(0x7)
#define ICHG_SET_1000MA			SHIFT0(0x13)
#define BB_LRSW_EN			(0x1 << 5)
#define BB_PREG_OFF			(0x1 << 6)

#define SANREMO_CHG_CTRL3		0x4a
#define CHG_TIMER_SET_DISABLE	(0xF << 4)
#define CHG_TIMER_SET_MASK		SHIFT4(0xF)
#define CHG_TIMER_SET_256MIN	SHIFT4(0x8)

#define SANREMO_CHG_CTRL4	0x4b
#define IPRE_SET_MASK		SHIFT0(0xF)
#define IPRE_SET_40MA		SHIFT0(0x7)
#define VPCHG_SET_MASK		SHIFT4(0x3)
#define VPCHG_SET_3P2V		SHIFT4(0x3)
#define IFCHG_MON_EN		SHIFT6(0x1)
#define BTEMP_MON_EN		SHIFT7(0x1)

#define SANREMO_CHG_CTRL5	0x4c
#define VCHG_ON_CNT_SEL_MASK	0x3
#define VCHG_ON_CNT_SEL_16SEC	SHIFT0(0x2)

#define SANREMO_CHG_CTRL6	0x4d
#define BD_MSK_MASK		SHIFT0(0x3)
#define BD_MSK_GPDAC1		SHIFT0(0x01)
#define BC_OV_VBAT_EN		(1 << 2)
#define BC_UV_VBAT_EN		(1 << 3)

#define SANREMO_CHG_CTRL7	0x4e
#define BAT_REM_EN		(1 << 3)
#define ILIM_LONGTMREN		(1 << 6)
#define IFSM_EN			(1 << 7)
/*********(1)For Battery_Charger End*******************************************/  

/*********(2) For Measurement_Unit Begin*******************************************/       
#define SANREMO_GP_BIAS1	0x4f

#define SANREMO_MEAS_ENABLE1	0x50
#define SANREMO_MEAS_EN1_VBAT           (1 << 0)
#define SANREMO_MEAS_EN1_VCHG           (1 << 1)
#define SANREMO_MEAS_EN1_VSYS           (1 << 2)
#define SANREMO_MEAS_EN1_TINT           (1 << 3)
#define SANREMO_MEAS_EN1_GPADC0       	(1 << 4)
#define SANREMO_MEAS_EN1_GPADC1         (1 << 5)
#define SANREMO_MEAS_EN1_TBAT           (1 << 5)
#define SANREMO_MEAS_EN1_GPADC2        	(1 << 6)
#define SANREMO_MEAS_EN1_GPADC3         (1 << 7)

#define SANREMO_MEAS_ENABLE2	0x51

#define SANREMO_MEAS_ENABLE3	0x52
#define SANREMO_MEAS_EN3_IBAT           (1 << 0)
#define SANREMO_MEAS_EN3_IBAT_COMP		(1 << 1)
#define SANREMO_MEAS_EN3_BAT_DET_EN_B0	(1 << 1)
#define SANREMO_MEAS_EN3_COULOMB_COUNTER (1 << 2)
#define SANREMO_MEAS_EN3_PENDET         (1 << 3)
#define SANREMO_MEAS_EN3_TSIX		(1 << 4)
#define SANREMO_MEAS_EN3_TSIY           (1 << 5)
#define SANREMO_MEAS_EN3_TSIZ1        	(1 << 6)
#define SANREMO_MEAS_EN3_TSIZ2         	(1 << 7)

#define SANREMO_MEAS_OFF_TIME1	0x53
#define SANREMO_MEASOFFTIME1_DOUBLE_TSI 	(1 << 0)
#define SANREMO_MEASOFFTIME1_MEAS_OFF_TIME1  	(1 << 1)
#define SANREMO_MEASOFFTIME1_MEAS_OFF_TIME1_MASK  	(0x3F << 1)
#define SANREMO_MEASOFFTIME1_MEAS_OFF_TIME1_BIT  	(1)

#define SANREMO_MEAS_OFF_TIME2	0x54
#define SANREMO_TSI_PREBIAS_TIME	0x55
#define SANREMO_PD_PREBIAS_TIME	0x56

#define SANREMO_GPADC_MISC1	0x57
#define SANREMO_GPADC_MISC1_GPFSM_EN    (1 << 0)
#define SANREMO_GPPADC_GP_PREBIAS_TIME  (01 << 1)
#define SANREMO_GPADC_MISC1_MASK  (SANREMO_GPADC_MISC1_GPFSM_EN | SANREMO_GPPADC_GP_PREBIAS_TIME)

#define SANREMO_SW_CAL	0x58
#define SANREMO_GPADC_MISC2	0x59
#define SANREMO_GPADC0_GP_BIAS_A0	(1 << 0)
#define SANREMO_GPADC1_GP_BIAS_A1	(1 << 1)
#define SANREMO_GPADC2_GP_BIAS_A2	(1 << 2)
#define SANREMO_GPADC3_GP_BIAS_A3	(1 << 3)

#define SANREMO_GP_BIAS2	0x5a
#define GP1_BIAS_SET_MASK	SHIFT4(0xF)
#define GP1_BIAS_SET_15UA	SHIFT4(0x2)
#define GP1_BIAS_SET_16UA	SHIFT4(0x3)
#define C1_GP1_BIAS_SET_15UA	SHIFT4(0x3)

#define SANREMO_GP_BIAS3	//TBD
#define SANREMO_VBAT_LOW_TH	0x5b
#define SANREMO_VCHG_LOW_TH	0x5c
#define SANREMO_VSYS_LOW_TH	0x5d
#define SANREMO_TINT_LOW_TH	0x5e
#define SANREMO_TBAT_LOW_TH	0x5f
#define SANREMO_GPADC0_LOW_TH	0x5f
#define SANREMO_GPADC1_LOW_TH	0x60
#define SANREMO_GPADC2_LOW_TH	0x61
#define SANREMO_GPADC3_LOW_TH	0x62

#define SANREMO_VBAT_UPP_TH	0x63
#define SANREMO_VCHG_UPP_TH	0x64
#define SANREMO_VSYS_UPP_TH	0x65
#define SANREMO_TINT_UPP_TH	0x66
#define SANREMO_TBAT_UPP_TH	0x67

#define SANREMO_GPADC0_UPP_TH	0x67
#define SANREMO_GPADC1_UPP_TH	0x68
#define SANREMO_GPADC2_UPP_TH	0x69
#define SANREMO_GPADC3_UPP_TH	0x6a
#define SANREMO_IBAT_MEAS1	0x6b
#define SANREMO_IBAT_MEAS2	0x6c
#define SANREMO_VBAT_MEAS1	0x6d
#define SANREMO_VBAT_MEAS2	0x6e
#define SANREMO_VCHG_MEAS1	0x6f
#define SANREMO_VCHG_MEAS2	0x70
#define SANREMO_VSYS_MEAS1	0x71
#define SANREMO_VSYS_MEAS2	0x72
#define SANREMO_OVER_TEMP	(1 << 5)
#define SANREMO_TINT_MEAS1	0x73
#define SANREMO_TINT_MEAS2	0x74
#define SANREMO_GPADC0_MEAS1	0x75
#define SANREMO_GPADC0_MEAS2	0x76
#define SANREMO_TBAT_MEAS1	0x77
#define SANREMO_TBAT_MEAS2	0x78
#define SANREMO_GPADC2_MEAS1	0x79
#define SANREMO_GPADC2_MEAS2	0x7a
#define SANREMO_GPADC3_MEAS1	0x7b
#define SANREMO_GPADC3_MEAS2	0x7c
#define SANREMO_VRTC_MEAS1	(0x7d)
#define SANREMO_VRTC_MEAS2	(0x7e)
#define SANREMO_TSIX_MEAS1	0x8d
#define SANREMO_TSIX_MEAS2	0x8e
#define SANREMO_TSIY_MEAS1	0x8f
#define SANREMO_TSIY_MEAS2	0x90
#define SANREMO_TSIZ1_MEAS1	0x91
#define SANREMO_TSIZ1_MEAS2	0x92
#define SANREMO_TSIZ2_MEAS1	0x93
#define SANREMO_TSIZ2_MEAS2	0x94
#define SANREMO_CCNT1	0x95
#define SANREMO_CCNT2	0x96
#define SANREMO_VBAT_AVG	0x97
#define SANREMO_VCHG_AVG	0x98
#define SANREMO_VSYS_AVG	0x99
#define SANREMO_VBAT_MIN	0x9a
#define SANREMO_VCHG_MIN	0x9b
#define SANREMO_VSYS_MIN	0x9c
#define SANREMO_VBAT_MAX	0x9d
#define SANREMO_VCHG_MAX	0x9e
#define SANREMO_VSYS_MAX	0x9f

#define SANREMO_RTC_MISC_3	0xaf
#define SANREMO_FAULT_WU	(1 << 3)
#define SANREMO_FAULT_WU_EN	(1 << 2)

/*********(2)For Measurement_Unit End*******************************************/ 
void SanremoWrite(UINT8_T reg_addr, UINT8_T value);
UINT8_T SanremoRead(UINT8_T reg_addr);
UINT16_T sanremo_read_volt_meas_val(UINT8_T meaReg);
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
