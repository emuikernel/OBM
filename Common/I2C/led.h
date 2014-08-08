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
#ifndef __LED_H__
#define __LED_H__

#include "GPIO.h"
#include "Typedef.h"
#include "timer.h"

#if SANREMO
#include "sanremo.h"
#endif

#if USTICA
#include "ustica.h"
#endif


#define HIGH_L	GPIO_PSR
#define LOW_L	GPIO_PCR

#if MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
// update LED 4G to LED power
#define R_LED_PWR(level_reg)	reg_write((GPIO3_BASE+level_reg),BIT30);		
#define G_LED_PWR(level_reg)	reg_write((GPIO2_BASE+level_reg),BIT6);
#define B_LED_PWR(level_reg)	reg_write((GPIO2_BASE+level_reg),BIT5);

#define R_LED_4G(level_reg)		NULL // reg_write((GPIO3_BASE+level_reg),BIT31);		
#define G_LED_4G(level_reg)		NULL // reg_write((GPIO0_BASE+level_reg),BIT3);
#define B_LED_4G(level_reg)		NULL // reg_write((GPIO0_BASE+level_reg),BIT13);

#define R_LED_W(level_reg)		reg_write((GPIO0_BASE+level_reg),BIT28);		
#define G_LED_W(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT1);
#define B_LED_W(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT2);

#define R_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT3);		
#define G_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT4);
#define B_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT17);
#elif MIFI_V3R0
#define R_LED_4G(level_reg)		reg_write((GPIO3_BASE+level_reg),BIT30);		
#define G_LED_4G(level_reg)		reg_write((GPIO0_BASE+level_reg),BIT2);
#define B_LED_4G(level_reg)		reg_write((GPIO0_BASE+level_reg),BIT0);

#define R_LED_PWR(level_reg)	reg_write((GPIO3_BASE+level_reg),BIT31);		
#define G_LED_PWR(level_reg)	reg_write((GPIO0_BASE+level_reg),BIT3);
#define B_LED_PWR(level_reg)	reg_write((GPIO0_BASE+level_reg),BIT13);

#define R_LED_W(level_reg)		reg_write((GPIO0_BASE+level_reg),BIT28);		
#define G_LED_W(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT1);
#define B_LED_W(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT2);

#define R_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT3);		
#define G_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT4);
#define B_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT17);
#else
#define R_LED_4G(level_reg)		reg_write((GPIO0_BASE+level_reg),BIT14);		
#define G_LED_4G(level_reg)		reg_write((GPIO0_BASE+level_reg),BIT15);
#define B_LED_4G(level_reg)		reg_write((GPIO0_BASE+level_reg),BIT16);

#define R_LED_PWR(level_reg)	reg_write((GPIO0_BASE+level_reg),BIT25);		
#define G_LED_PWR(level_reg)	reg_write((GPIO0_BASE+level_reg),BIT26);
#define B_LED_PWR(level_reg)	reg_write((GPIO0_BASE+level_reg),BIT27);

#define R_LED_W(level_reg)		reg_write((GPIO0_BASE+level_reg),BIT28);		
#define G_LED_W(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT1);
#define B_LED_W(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT2);

#define R_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT3);		
#define G_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT4);
#define B_LED_3G(level_reg)		reg_write((GPIO1_BASE+level_reg),BIT17);
#endif

#if NAND_CODE

#define LED_4G_Green()			R_LED_4G(LOW_L); G_LED_4G(HIGH_L);B_LED_4G(LOW_L);	//strong Signal
#define LED_4G_Yellow()			R_LED_4G(HIGH_L);G_LED_4G(HIGH_L);B_LED_4G(LOW_L);	//weak Signal
#define LED_4G_Blue()			R_LED_4G(HIGH_L);G_LED_4G(LOW_L); B_LED_4G(LOW_L);	//no Signal or No Service
#define LED_4G_off()			R_LED_4G(LOW_L); G_LED_4G(LOW_L); B_LED_4G(LOW_L);	//no Signal or No Service
#define LED_4G_Red()			R_LED_4G(LOW_L); G_LED_4G(LOW_L); B_LED_4G(HIGH_L);

#define LED_3G_Green()			R_LED_3G(LOW_L); G_LED_3G(HIGH_L);B_LED_3G(LOW_L);	//strong Signal
#define LED_3G_Yellow()			R_LED_3G(HIGH_L);G_LED_3G(HIGH_L);B_LED_3G(LOW_L);	//weak Signal
#define LED_3G_Blue()			R_LED_3G(HIGH_L);G_LED_3G(LOW_L); B_LED_3G(LOW_L);	//no Signal or No Service
#define LED_3G_off()			R_LED_3G(LOW_L); G_LED_3G(LOW_L); B_LED_3G(LOW_L);	//off
#define LED_3G_Red()			R_LED_3G(LOW_L); G_LED_3G(LOW_L); B_LED_3G(HIGH_L);

#define LED_Net_Red()			R_LED_W(LOW_L); G_LED_W(LOW_L); B_LED_W(HIGH_L);	//Network connected
#define LED_Net_Blue()			R_LED_W(HIGH_L);G_LED_W(LOW_L); B_LED_W(LOW_L);																				//Blue flickering, data transfering
#define LED_Net_Green()			R_LED_W(LOW_L); G_LED_W(HIGH_L);B_LED_W(LOW_L);		//Standby
#define LED_Net_off()			R_LED_W(LOW_L); G_LED_W(LOW_L); B_LED_W(LOW_L);		//off
#define NetMode_Connected()			LED_Net_Blue();
#define NetModet_Standby()			LED_Net_Green();
#define NetMode_Off()				LED_Net_off();


#define LED_PWR_Red()			R_LED_PWR(LOW_L); G_LED_PWR(LOW_L); B_LED_PWR(HIGH_L);	//Battery charged 6% ~ 20%
#define LED_PWR_Yellow()		R_LED_PWR(HIGH_L);G_LED_PWR(HIGH_L);B_LED_PWR(LOW_L);	//Battery charged 1% ~ 5%	
#define LED_PWR_Blue()			R_LED_PWR(HIGH_L);G_LED_PWR(LOW_L); B_LED_PWR(LOW_L);	//Battery charged >20%	
#define LED_PWR_Green()			R_LED_PWR(LOW_L); G_LED_PWR(HIGH_L);B_LED_PWR(LOW_L);	//Fully charged

#define LED_PWR_White()			R_LED_PWR(HIGH_L); G_LED_PWR(HIGH_L); 	B_LED_PWR(HIGH_L);	//White
#define LED_PWR_off()			R_LED_PWR(LOW_L); G_LED_PWR(LOW_L); 	B_LED_PWR(LOW_L);	//off

#else

#define LED_4G_Green()			R_LED_4G(LOW_L); G_LED_4G(HIGH_L);B_LED_4G(LOW_L);	//strong Signal
#define LED_4G_Yellow()			R_LED_4G(HIGH_L);G_LED_4G(HIGH_L);B_LED_4G(LOW_L);	//weak Signal
#define LED_4G_Red()			R_LED_4G(HIGH_L);G_LED_4G(LOW_L); B_LED_4G(LOW_L);	//no Signal or No Service
#define LED_4G_off()			R_LED_4G(LOW_L); G_LED_4G(LOW_L); B_LED_4G(LOW_L);	//no Signal or No Service
#define LED_4G_Blue()			R_LED_4G(LOW_L); G_LED_4G(LOW_L); B_LED_4G(HIGH_L);

#define LED_3G_Green()			R_LED_3G(LOW_L); G_LED_3G(HIGH_L);B_LED_3G(LOW_L);	//strong Signal
#define LED_3G_Yellow()			R_LED_3G(HIGH_L);G_LED_3G(HIGH_L);B_LED_3G(LOW_L);	//weak Signal
#define LED_3G_Red()			R_LED_3G(HIGH_L);G_LED_3G(LOW_L); B_LED_3G(LOW_L);	//no Signal or No Service
#define LED_3G_off()			R_LED_3G(LOW_L); G_LED_3G(LOW_L); B_LED_3G(LOW_L);	//off
#define LED_3G_Blue()			R_LED_3G(LOW_L); G_LED_3G(LOW_L); B_LED_3G(HIGH_L);

#define LED_Net_Blue()			R_LED_W(LOW_L); G_LED_W(LOW_L); B_LED_W(HIGH_L);	//Network connected
#define LED_Net_Red()			R_LED_W(HIGH_L);G_LED_W(LOW_L); B_LED_W(LOW_L);																				//Blue flickering, data transfering
#define LED_Net_Green()			R_LED_W(LOW_L); G_LED_W(HIGH_L);B_LED_W(LOW_L);		//Standby
#define LED_Net_off()			R_LED_W(LOW_L); G_LED_W(LOW_L); B_LED_W(LOW_L);		//off
#define NetMode_Connected()			LED_Net_Blue();
#define NetModet_Standby()			LED_Net_Green();
#define NetMode_Off()				LED_Net_off();


#define LED_PWR_Blue()			R_LED_PWR(LOW_L); G_LED_PWR(LOW_L); B_LED_PWR(HIGH_L);	//Battery charged 6% ~ 20%
#define LED_PWR_Yellow()		R_LED_PWR(HIGH_L);G_LED_PWR(HIGH_L);B_LED_PWR(LOW_L);	//Battery charged 1% ~ 5%	
#define LED_PWR_Red()			R_LED_PWR(HIGH_L);G_LED_PWR(LOW_L); B_LED_PWR(LOW_L);	//Battery charged >20%	
#define LED_PWR_Green()			R_LED_PWR(LOW_L); G_LED_PWR(HIGH_L);B_LED_PWR(LOW_L);	//Fully charged

#define LED_PWR_White()			R_LED_PWR(HIGH_L); G_LED_PWR(HIGH_L); 	B_LED_PWR(HIGH_L);	//White
#define LED_PWR_off()			R_LED_PWR(LOW_L); G_LED_PWR(LOW_L); 	B_LED_PWR(LOW_L);	//off

#endif

#define BatState_10()			LED_PWR_Blue();

#define BatCharging_Full()		LED_PWR_Green();
#define BatCharging_On()		LED_PWR_Red();
#define BatState_Disoff()		LED_PWR_off();

#define SoftwareUpgrade_Start()	LED_PWR_Red();LED_Net_Red();LED_3G_Red();LED_4G_Red();
#define SoftwareUpgrade_Done()	LED_PWR_Green();LED_Net_Green();LED_3G_Green();LED_4G_Green();

#define External_Power_Display()	LED_PWR_Red();
#define External_Power_Off()		LED_PWR_off();
#define BatCharging_In_Process()	LED_PWR_Red();

#define No_Battery_Display()	LED_PWR_Red();
#define No_Battery_Off()		LED_PWR_off();

#define ONKEY_Bootup()			LED_PWR_Blue();
#define USB_Connect_Display()	LED_PWR_Red();

#define Firmware_Upgrade_Start()		SoftwareUpgrade_Start();
#define Firmware_Upgrade_Done()			SoftwareUpgrade_Done();

#define PowerOnBoot()			LED_PWR_Green();LED_Net_Green();LED_3G_Green();LED_4G_Green();

#define ClosePowerOnBoot()		LED_PWR_Red();LED_Net_off();LED_3G_off();LED_4G_off();

/*************************************************************************/
void DisplayBatstate(UINT8_T batpercent);

#endif

