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

#include "oled.h"
#include "ssd1306.h"
#include "sh1106.h"
#include "Typedef.h"


void I2COLED_Init(void)
{
	#if SSD1306
	SSD1306_I2COLED_Init();
	#elif SH1106
	SH1106_I2COLED_Init();
	#endif
}

void I2COLED_DisplayOff(void)
{
	#if SSD1306
	SSD1306_I2COLED_Wcmd(I2COLED_Display_OFF);
	#elif SH1106
	SH1106_I2COLED_Wcmd(I2COLED_Display_OFF);
	#endif
}


