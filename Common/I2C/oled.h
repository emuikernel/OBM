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
#ifndef __OLED_H__
#define __OLED_H__


#if SANREMO
#include "sanremo.h"
#endif
 
#if USTICA
#include "ustica.h"
#endif

#include "ssd1306.h"
#include "sh1106.h"

#define DATABUF_SIZE_I2COLED	260
#define char611v_CharSize	(7*2)

/******************************************************/
#define BatCharging_Full()
#define BatState_Disoff()
#define BatCharging_In_Process()
#define External_Power_Off()
#define No_Battery_Off()
#define PowerOnBoot()	show_logo(0);


/*************************************************/
void I2COLED_Init(void);
void Clear_Screen(void);
void Battery_Charing_Display(UINT8_T battery_stage);
UINT8_T Get_Dispaly_Percent(UINT8_T percent);
void BatCharging_On(void);
void Update_Battery_State(UINT16_T vbat_mv);
void ONKEY_Bootup(void);
void show_logo(UINT8_T choice);
void show_battery_status(UINT8_T choice);
void show_external_power(UINT8_T choice);
void show_USB_status(UINT8_T choice);
void show_Downlaod(UINT8_T choice);

#endif
