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
#ifndef __SH1106_H__
#define __SH1106_H__

#if SANREMO
#include "sanremo.h"
#endif

#if USTICA
#include "ustica.h"
#endif

#define DATABUF_SIZE_I2COLED	260
#define char611v_CharSize	(7*2)

#define ScanModeSet			0x20
#define SCAN_In_Horizontal 	0x2
#define SCAN_In_vertical 	0x1
#define SCAN_In_Page		0x0

#define ColumnAddSet		0x21
#define PageAddSet			0x22

#define SH1106_I2COLED_Wcmd(val)		SH1106_I2COLED_Write_I(val);
#define SH1106_I2COLED_Wdata(val)		SH1106_I2COLED_Write_D(val);

#define SH1106_Cmd_noPara(cmd)	SH1106_I2COLED_Wcmd(cmd);
#define SH1106_Cmd_1Para(cmd,para1)	\
			{ SH1106_I2COLED_Wcmd(cmd); \
			  SH1106_I2COLED_Wcmd(para1); \
			}
#define SH1106_Cmd_2Para(cmd,para1,para2)	\
			{ SH1106_I2COLED_Wcmd(cmd); \
			  SH1106_I2COLED_Wcmd(para1); \
			  SH1106_I2COLED_Wcmd(para2); \
			}

#define I2COLED_Entire_DisplayON1	0xA4
#define I2COLED_Entire_DisplayON2	0xA5

#define I2COLED_Display_ON			0xAF
#define I2COLED_Display_OFF			0xAE

#if NEZHA_MIFI_V4R1
#define sh1106_Battery_Charging_Page_Start	0x00
#define sh1106_Battery_Charging_Page_Num	0x04
#define sh1106_Battery_Charging_Coloum		0x20
#define sh1106_Battery_Charging_Width		0x40
#define sh1106_Battery_Charging_Bytes_Num	(sh1106_Battery_Charging_Page_Num * sh1106_Battery_Charging_Width)

#define sh1106_LOGO0_Page_Start			0x00
#define sh1106_LOGO0_Page_Num				0x04
#define sh1106_LOGO0_Coloum				0x20
#define sh1106_LOGO0_Width					0x40
#define sh1106_LOGO0_Bytes_Num				(sh1106_LOGO0_Page_Num * sh1106_LOGO0_Width)

#define sh1106_Battery_status0_Page_Start	0x00
#define sh1106_Battery_status0_Page_Num	0x04
#define sh1106_Battery_status0_Coloum		0x40
#define sh1106_Battery_status0_Width		0x20
#define sh1106_Battery_status0_Bytes_Num	(sh1106_Battery_status0_Page_Num * sh1106_Battery_status0_Width)

#define sh1106_External_Power_status0_Page_Start	0x00
#define sh1106_External_Power_status0_Page_Num		0x04
#define sh1106_External_Power_status0_Coloum		0x40
#define sh1106_External_Power_status0_Width		0x10
#define sh1106_External_Power_status0_Bytes_Num	(sh1106_External_Power_status0_Page_Num * sh1106_External_Power_status0_Width)

#define sh1106_USB_status0_Page_Start	0x00
#define sh1106_USB_status0_Page_Num	0x03
#define sh1106_USB_status0_Coloum		0x30
#define sh1106_USB_status0_Width		0x30
#define sh1106_USB_status0_Bytes_Num	(sh1106_USB_status0_Page_Num * sh1106_USB_status0_Width)

#define sh1106_Download_Page_Start	0x00
#define sh1106_Download_Page_Num	0x04
#define sh1106_Download_Coloum		0x30
#define sh1106_Download_Width		0x20
#define sh1106_Download_Bytes_Num	(sh1106_Download_Page_Num * sh1106_Download_Width)
#else
#define sh1106_Battery_Charging_Page_Start	0x02
#define sh1106_Battery_Charging_Page_Num	0x04
#define sh1106_Battery_Charging_Coloum		0x20
#define sh1106_Battery_Charging_Width		0x40
#define sh1106_Battery_Charging_Bytes_Num	(sh1106_Battery_Charging_Page_Num * sh1106_Battery_Charging_Width)

#define sh1106_LOGO0_Page_Start			0x02
#define sh1106_LOGO0_Page_Num				0x04
#define sh1106_LOGO0_Coloum				0x20
#define sh1106_LOGO0_Width					0x40
#define sh1106_LOGO0_Bytes_Num				(sh1106_LOGO0_Page_Num * sh1106_LOGO0_Width)

#define sh1106_Battery_status0_Page_Start	0x02
#define sh1106_Battery_status0_Page_Num	0x04
#define sh1106_Battery_status0_Coloum		0x40
#define sh1106_Battery_status0_Width		0x20
#define sh1106_Battery_status0_Bytes_Num	(sh1106_Battery_status0_Page_Num * sh1106_Battery_status0_Width)

#define sh1106_External_Power_status0_Page_Start	0x02
#define sh1106_External_Power_status0_Page_Num		0x04
#define sh1106_External_Power_status0_Coloum		0x40
#define sh1106_External_Power_status0_Width		0x10
#define sh1106_External_Power_status0_Bytes_Num	(sh1106_External_Power_status0_Page_Num * sh1106_External_Power_status0_Width)

#define sh1106_USB_status0_Page_Start	0x02
#define sh1106_USB_status0_Page_Num	0x03
#define sh1106_USB_status0_Coloum		0x30
#define sh1106_USB_status0_Width		0x30
#define sh1106_USB_status0_Bytes_Num	(sh1106_USB_status0_Page_Num * sh1106_USB_status0_Width)

#define sh1106_Download_Page_Start	0x02
#define sh1106_Download_Page_Num	0x04
#define sh1106_Download_Coloum		0x30
#define sh1106_Download_Width		0x20
#define sh1106_Download_Bytes_Num	(sh1106_Download_Page_Num * sh1106_Download_Width)
#endif

/******************************************************/
#define BatCharging_Full()
#define BatState_Disoff()
#define BatCharging_In_Process()
#define External_Power_Off()
#define No_Battery_Off()


/*************************************************/
void SH1106_I2COLED_Init(void);

#endif
