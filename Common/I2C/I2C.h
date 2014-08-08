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
#ifndef _I2C_H_

#define _I2C_H_

#include "Typedef.h"
#include "timer.h"
#include "charger.h"

#if LED_DISPLAY
#include "led.h"
#endif

#if SANREMO
#include "sanremo.h"
#endif

#if USTICA
#include "ustica.h"
#endif

#if FAN540X
#include "fan540x.h"
#endif

#if RT9455
#include "rt9455.h"
extern UINT8_T BBU_getCI2C(UINT8_T regAddr);
extern UINT_T BBU_putCI2C(UINT8_T regAddr, UINT8_T data);
extern UINT_T BBU_CI2C_Init();
#endif

#if OLED_SUPPORT
#include "oled.h"

extern UINT8_T BBU_getCI2C(UINT8_T regAddr);
extern UINT_T BBU_putCI2C(UINT8_T regAddr, UINT8_T data);
extern UINT_T BBU_CI2C_Init();
#endif

extern UINT8_T BBU_getPI2C(UINT8_T regAddr);
extern UINT_T BBU_putPI2C(UINT8_T regAddr, UINT8_T data);
extern UINT_T  BBU_PI2C_Init();


#define OCV_NUM 100

#define USB_Valid_Voltage			4000
#define USB_Over_Voltage			6000

#define Charger_Deafult_Voltage		3540 // FAN5405 default value

#if MIFI_V3R2 || MIFI_V3R3
#define Battery_Full_Voltage		4350
#else
#define Battery_Full_Voltage		4200
#endif

#define Charger_Maximum_Voltage		4200

#define Battery_Valid_Voltage		3300
#define Battery_Low_Voltage			3450

#define Hours_6						(6 * 60 * 60)
#define Hours_2						(2 * 60 * 60)

/********************************************************************/

void battery_process_step1(void);
void battery_process_step2(void);
void battery_process_step3(void);
void battery_process_step4(void);

#if ZIMI_PB05
#define ZIMI_OBM_VER_INFO	0x312E3100   // 1.1
#endif

#endif  /* _I2C_H_ */

