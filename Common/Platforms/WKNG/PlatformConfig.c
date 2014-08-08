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
 *
 ******************************************************************************/

#include "PlatformConfig.h"
#include "platform_interrupts.h"
#include "predefines.h"
#include "USB1.h"
#include "USB2.h"
#include "PMUA.h"
#include "usbPal.h"
#include "Flash.h"
#include "timer.h"
#include "uart_regs.h"
#include "APBC.h"
#if BOOTROM
#include "bootrom.h"
#endif
#include "misc.h"
#include "sdhc2.h"
#include "CIU.h"
#include "GPIO.h"

/*
 *  Table definitions for multi function pin registers
 */

//#if UPDATE_USE_KEY
#if MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
const CS_REGISTER_PAIR_S keypad_pins[] = 
{
 		(int *) (APPS_PAD_BASE | 0x02f4), 0x1081, 0x0,	//GPIO WPS key
		0x0,0x0,0x0 // termination
};
#elif MIFI_V2R0 || MIFI_V2R1 || NEZHA_MIFI_V3R1 || NEZHA_MIFI_V4R1R1
/*****************************************************************************

  Nezha Keypad Decode lookup table 

  0x08 - Key_WiFi(left up)      0x01 - Key_TD(left down)   

  0x04 - Key_uAP(right up)

******************************************************************************/
const CS_REGISTER_PAIR_S keypad_pins[] = 
{
 		(int *) (APPS_PAD_BASE | 0x02f4), 0xa8c7, 0x0,	//Key_TD
 		(int *) (APPS_PAD_BASE | 0x02fc), 0xa8c7, 0x0,	//Key_uAP
 		(int *) (APPS_PAD_BASE | 0x0300), 0xa8c7, 0x0,	//Key_WiFi
		0x0,0x0,0x0 // termination
};
#elif NEZHA_MIFI_V4R1
const CS_REGISTER_PAIR_S keypad_pins[] = 
{
 		(int *) (APPS_PAD_BASE | 0x02f4), 0xa8c7, 0x0,	//Key_WPS
		0x0,0x0,0x0 // termination
};
#else
/*****************************************************************************

  Wukong Keypad Decode lookup table 

  0x00 - Key_WiFi(left up)      0x04 - Key_TD(right up)   

  0x02 - Key_uAP(left down)

******************************************************************************/
const CS_REGISTER_PAIR_S keypad_pins[] = 
{
 		(int *) (APPS_PAD_BASE | 0x02f8), 0xa8c7, 0x0,	//Key_WiFi
 		(int *) (APPS_PAD_BASE | 0x02fc), 0xa8c7, 0x0,	//Key_uAP
 		(int *) (APPS_PAD_BASE | 0x0300), 0xa8c7, 0x0,	//Key_TD
		0x0,0x0,0x0 // termination
};
#endif
//#endif

const CS_REGISTER_PAIR_S io_pins[] =
{
 		(int *) (APPS_PAD_BASE | 0x0040), 0x1080, 0x0,	//ND_IO0
		(int *) (APPS_PAD_BASE | 0x003C), 0x1080, 0x0,	//ND_IO1
		(int *) (APPS_PAD_BASE | 0x0038), 0x1080, 0x0,	//ND_IO2
		(int *) (APPS_PAD_BASE | 0x0034), 0x1080, 0x0,	//ND_IO3
		(int *) (APPS_PAD_BASE | 0x0030), 0x1080, 0x0,	//ND_IO4
		(int *) (APPS_PAD_BASE | 0x002C), 0x1080, 0x0,	//ND_IO5
		(int *) (APPS_PAD_BASE | 0x0028), 0x1080, 0x0,	//ND_IO6
		(int *) (APPS_PAD_BASE | 0x0024), 0x1080, 0x0,	//ND_IO7
		(int *) (APPS_PAD_BASE | 0x0020), 0x10C0, 0x0,	//ND_IO8
		(int *) (APPS_PAD_BASE | 0x001C), 0x10C0, 0x0,	//ND_IO9
		(int *) (APPS_PAD_BASE | 0x0018), 0x10C0, 0x0,	//ND_IO10
		(int *) (APPS_PAD_BASE | 0x0014), 0x10C0, 0x0,	//ND_IO11
		(int *) (APPS_PAD_BASE | 0x0010), 0x10C0, 0x0,	//ND_IO12
		(int *) (APPS_PAD_BASE | 0x000C), 0x10C0, 0x0,	//ND_IO13
		(int *) (APPS_PAD_BASE | 0x0008), 0x10C0, 0x0,	//ND_IO14
		(int *) (APPS_PAD_BASE | 0x0004), 0x10C0, 0x0,	//ND_IO15
		0x0,0x0,0x0 // termination
};

const CS_REGISTER_PAIR_S common_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x0060), 0x1081, 0x0, 	//ND_ALE_SM_nWE
		(int *) (APPS_PAD_BASE | 0x005C), 0x1080, 0x0,	//ND_CLE_SM_nOE	
		0x0,0x0,0x0 //termination
};

const CS_REGISTER_PAIR_S cs0_common_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x0074), 0x1080, 0x0, 	//SM_ADV
		(int *) (APPS_PAD_BASE | 0x0064), 0x1080, 0x0, 	//SM_SCLK
	#if !MIFI3
		(int *) (APPS_PAD_BASE | 0x004C), 0x1080, 0x0, 	//SM_nCS0
	#endif
		0x0,0x0,0x0 //termination
};

#if MIFI3
const CS_REGISTER_PAIR_S usb_charge_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x004C), 0x1081, 0x0, 	//SM_nCS0
		0x0,0x0,0x0 //termination
};	
#endif

#if XIP
const CS_REGISTER_PAIR_S cs0_reg[]=
{
        (int *) (SMC_CSDFICFGx), 0x51890009, 0x0,                   // SMC_CSDFICFG0  
        (int *) (SMC_CSADRMAPx), 0x10000F00, 0x0,                   // SMC_CSADRMAP0  
		0x0,0x0,0x0 //termination
};
#endif

#if NAND_CODE
const CS_REGISTER_PAIR_S dfc_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x0044), 0x1080, 0x0, 	//ND_nCS0_SM_nCS2
		(int *) (APPS_PAD_BASE | 0x0054), 0x1081, 0x0, 	//ND_nWE
		(int *) (APPS_PAD_BASE | 0x0058), 0x1081, 0x0,	//ND_nRE
		(int *) (APPS_PAD_BASE | 0x0068), 0x5080, 0x0, 	//ND_RDY0
		0x0,0x0,0x0 //termination
};
#endif

#if HSI
const CS_REGISTER_PAIR_S hsi_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x0044), 0xD146, 0x0, 	//ND_nCS0_SM_nCS2	HSI_RX_WAKE
		(int *) (APPS_PAD_BASE | 0x0048), 0xD146, 0x0, 	//ND_nCS1_SM_nCS3	HSI_RX_FLAG
		(int *) (APPS_PAD_BASE | 0x004C), 0xD146, 0x0, 	//SM_nCS0			HSI_RX_DATA
		(int *) (APPS_PAD_BASE | 0x0050), 0xD146, 0x0, 	//SM_nCS1			HSI_RX_READY
		(int *) (APPS_PAD_BASE | 0x0054), 0xD146, 0x0, 	//ND_nWE			HSI_TX_WAKE
		(int *) (APPS_PAD_BASE | 0x0058), 0xD146, 0x0,	//ND_nRE			HSI_TX_FLAG
		(int *) (APPS_PAD_BASE | 0x005C), 0xD146, 0x0,	//ND_CLE_SM_nOE		HSI_TX_DATA
		(int *) (APPS_PAD_BASE | 0x0060), 0xD146, 0x0, 	//ND_ALE_SM_nWE		HSI_TX_READY
		0x0,0x0,0x0 //termination
};
#endif

#if XIP
const CS_REGISTER_PAIR_S xip_pins[]=
{
  		(int *) (APPS_PAD_BASE | 0x007C), 0x1080, 0x0,	//SM_ADVMUX
  		0x0,0x0,0x0 //termination
};
#endif

#if MMC_CODE
// MMC3 controller compatible pinout
const CS_REGISTER_PAIR_S mmc3_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x005C), 0x10C1, 0x0,	//MMC3_CMD	   
		(int *) (APPS_PAD_BASE | 0x0064), 0x10C1, 0x0,	//MMC3_CLK	   
		(int *) (APPS_PAD_BASE | 0x0040), 0x10C1, 0x0,	//MMC3_DAT0
		(int *) (APPS_PAD_BASE | 0x003C), 0x10C1, 0x0,	//MMC3_DAT1
		(int *) (APPS_PAD_BASE | 0x0038), 0x10C1, 0x0,	//MMC3_DAT2
		(int *) (APPS_PAD_BASE | 0x0034), 0x10C1, 0x0,	//MMC3_DAT3
		(int *) (APPS_PAD_BASE | 0x0030), 0x10C1, 0x0,	//MMC3_DAT4
		(int *) (APPS_PAD_BASE | 0x002C), 0x10C1, 0x0,	//MMC3_DAT5
		(int *) (APPS_PAD_BASE | 0x0028), 0x10C1, 0x0,	//MMC3_DAT6
		(int *) (APPS_PAD_BASE | 0x0024), 0x10C1, 0x0,	//MMC3_DAT7
		0x0,0x0,0x0 //termination
};
#endif 

const CS_REGISTER_PAIR_S uart_pins[]=
{
  		(int *) (APPS_PAD_BASE | 0x01a8), 0x5081, 0x0,	//RX
  		(int *) (APPS_PAD_BASE | 0x01ac), 0x5081, 0x0,	//TX
  		0x0,0x0,0x0 //termination
};

#if SPI_CODE
const CS_REGISTER_PAIR_S spi_pins[]=
{
  		(int *) (APPS_PAD_BASE | 0x10), 0x1082, 0x0,	//RX
		(int *) (APPS_PAD_BASE | 0x14), 0x1082, 0x0,	//TX
  		(int *) (APPS_PAD_BASE | 0x18), 0x1082, 0x0,	//FRM (CS)
  		(int *) (APPS_PAD_BASE | 0x1C), 0x1082, 0x0,	//CLK
  		0x0,0x0,0x0 //termination
};

const CS_REGISTER_PAIR_S spi_pins_cs[]=
{
  		(int *) (APPS_PAD_BASE | 0x10), 0x0882, 0x0,	//RX
		(int *) (APPS_PAD_BASE | 0x14), 0x0882, 0x0,	//TX
  		(int *) (APPS_PAD_BASE | 0x18), 0xc8c1, 0x0,	//GPIO_25[SSP2_FRM]
  		(int *) (APPS_PAD_BASE | 0x1C), 0x1082, 0x0,	//CLK
  		(int *) (APPS_PAD_BASE | 0x140), 0xc8c7, 0x0,	//NULL[GPIO_25]
  		0x0,0x0,0x0 //termination
};
#endif

#if UPDATE_USE_GPIO
const CS_REGISTER_PAIR_S gpio_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x00d0), 0x1080, 0x0, 	//GPIO[124]
		0x0,0x0,0x0 //termination
};	
#endif

#if OLED_SUPPORT
const CS_REGISTER_PAIR_S oled_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x01B0), 0xc0c2, 0x0,	//CI2C_SCL
		(int *) (APPS_PAD_BASE | 0x01B4), 0xc0c2, 0x0,	//CI2C_SDA
		(int *) (APPS_PAD_BASE | 0x0168), 0xC880, 0x0,	//GPIO35
		0x0,0x0,0x0 //termination
};

#if NEZHA_MIFI_V4R1
const CS_REGISTER_PAIR_S oled_power_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x0140), 0xC880, 0x0,	//GPIO25
		0x0,0x0,0x0 //termination
};
#endif
#endif

#if RT8973
const CS_REGISTER_PAIR_S usb_switch_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x01B0), 0xc0c2, 0x0,	//CI2C_SCL
		(int *) (APPS_PAD_BASE | 0x01B4), 0xc0c2, 0x0,	//CI2C_SDA
		0x0,0x0,0x0 //termination
};
#endif

#if PRODUCTION_MODE_SUPPORT
const CS_REGISTER_PAIR_S production_mode_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x02f4), 0x1081, 0x0,	//TDS_MIXCTRL
		0x0,0x0,0x0 //termination
};
#endif

#if SBOOT
const CS_REGISTER_PAIR_S sboot_wps_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x02f4), 0x1081, 0x0,	//TDS_MIXCTRL
		0x0,0x0,0x0 //termination
};
#endif

#if MIFI_V3R3
const CS_REGISTER_PAIR_S battery_detect_pins[]=
{
	#if NAND_CODE
		(int *) (APPS_PAD_BASE | 0x00d0), 0x1080, 0x0,	//GPIO124
	#else
		(int *) (APPS_PAD_BASE | 0x0034), 0x1082, 0x0,	//GPIO8
	#endif
		0x0,0x0,0x0 //termination
};
#endif

#if LED_DISPLAY
const CS_REGISTER_PAIR_S led_pins[]=
{
#if MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
		(int *) (APPS_PAD_BASE | 0x02bc), 0x0881, 0x0, 	//TDS_DIO2	-> GPIO69
		(int *) (APPS_PAD_BASE | 0x02c0), 0x0881, 0x0,	//TDS_DIO3	-> GPIO70
		(int *) (APPS_PAD_BASE | 0x0080), 0x0881, 0x0,	//SM_RDY		-> GPIO3
		(int *) (APPS_PAD_BASE | 0x0064), 0x0883, 0x0, 	//SM_SCLK		-> GPIO13
		(int *) (APPS_PAD_BASE | 0x006c), 0x0880, 0x0,	//SM_nBE0		-> GPIO126
		(int *) (APPS_PAD_BASE | 0x0070), 0x0880, 0x0,	//SM_nBE1		-> GPIO127
		(int *) (APPS_PAD_BASE | 0x014c), 0x0880, 0x0,	//GPIO28
		(int *) (APPS_PAD_BASE | 0x0160), 0x0880, 0x0, 	//GPIO33
		(int *) (APPS_PAD_BASE | 0x0164), 0x0880, 0x0,	//GPIO34
		(int *) (APPS_PAD_BASE | 0x0168), 0x0880, 0x0,	//GPIO35
		(int *) (APPS_PAD_BASE | 0x016c), 0x0880, 0x0,	//GPIO36
		(int *) (APPS_PAD_BASE | 0x01a0), 0x0880, 0x0,	//GPIO49
#elif MIFI_V3R0
		(int *) (APPS_PAD_BASE | 0x0074), 0x0881, 0x0, 	//SM_ADV		-> GPIO0
		(int *) (APPS_PAD_BASE | 0x007c), 0x0881, 0x0,	//SM_ADVMUX	-> GPIO2
		(int *) (APPS_PAD_BASE | 0x0080), 0x0881, 0x0,	//SM_RDY		-> GPIO3
		(int *) (APPS_PAD_BASE | 0x0064), 0x0883, 0x0, 	//SM_SCLK		-> GPIO13
		(int *) (APPS_PAD_BASE | 0x006c), 0x0880, 0x0,	//SM_nBE0		-> GPIO126
		(int *) (APPS_PAD_BASE | 0x0070), 0x0880, 0x0,	//SM_nBE1		-> GPIO127
		(int *) (APPS_PAD_BASE | 0x014c), 0x0880, 0x0,	//GPIO28
		(int *) (APPS_PAD_BASE | 0x0160), 0x0880, 0x0, 	//GPIO33
		(int *) (APPS_PAD_BASE | 0x0164), 0x0880, 0x0,	//GPIO34
		(int *) (APPS_PAD_BASE | 0x0168), 0x0880, 0x0,	//GPIO35
		(int *) (APPS_PAD_BASE | 0x016c), 0x0880, 0x0,	//GPIO36
		(int *) (APPS_PAD_BASE | 0x01a0), 0x0880, 0x0,	//GPIO49
#else
		(int *) (APPS_PAD_BASE | 0x0098), 0x0884, 0x0, 	//MMC1_DAT2
		(int *) (APPS_PAD_BASE | 0x009c), 0x0884, 0x0,	//MMC1_DAT1
		(int *) (APPS_PAD_BASE | 0x00a0), 0x0884, 0x0,	//MMC1_DAT0
		(int *) (APPS_PAD_BASE | 0x0140), 0x0880, 0x0, 	//GPIO25
		(int *) (APPS_PAD_BASE | 0x0144), 0x0880, 0x0,	//GPIO26
		(int *) (APPS_PAD_BASE | 0x0148), 0x0880, 0x0,	//GPIO27
		(int *) (APPS_PAD_BASE | 0x014c), 0x0880, 0x0,	//GPIO28
		(int *) (APPS_PAD_BASE | 0x0160), 0x0880, 0x0, 	//GPIO33
		(int *) (APPS_PAD_BASE | 0x0164), 0x0880, 0x0,	//GPIO34
		(int *) (APPS_PAD_BASE | 0x0168), 0x0880, 0x0,	//GPIO35
		(int *) (APPS_PAD_BASE | 0x016c), 0x0880, 0x0,	//GPIO36
		(int *) (APPS_PAD_BASE | 0x01a0), 0x0880, 0x0,	//GPIO49
#endif
		0x0,0x0,0x0 //termination
};
#endif

#if I2C
#if MIFI_V3R0 || MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
const CS_REGISTER_PAIR_S charger_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x0140), 0x18C0, 0x0, 	//GPIO25
		(int *) (APPS_PAD_BASE | 0x0144), 0x18C0, 0x0,	//GPIO26
		(int *) (APPS_PAD_BASE | 0x0148), 0x18C0, 0x0,	//GPIO27
		0x0,0x0,0x0 //termination
};
#else
const CS_REGISTER_PAIR_S charger_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x0084), 0x18C1, 0x0, 	//CHG_DISABLE
		(int *) (APPS_PAD_BASE | 0x0088), 0x18C1, 0x0,	//CHG_STAT
		(int *) (APPS_PAD_BASE | 0x008c), 0x18C1, 0x0,	//CHG_OTG_EN
		0x0,0x0,0x0 //termination
};
#endif
const CS_REGISTER_PAIR_S pi2c_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x00b4), 0x00c0, 0x0, 	//PRI_TDI
		(int *) (APPS_PAD_BASE | 0x00b8), 0x00c0, 0x0,	//PRI_TMS
		(int *) (APPS_PAD_BASE | 0x00bc), 0x00c0, 0x0,	//PRI_CLK
		(int *) (APPS_PAD_BASE | 0x00c0), 0x00c0, 0x0,	//PRI_TDO
		(int *) (APPS_PAD_BASE | 0x00c8), 0xa0c1, 0x0,	//SLAVE_RESET_OUT->GPIO[122]
		(int *) (APPS_PAD_BASE | 0x0304), 0xa0c0, 0x0,	//GPIO[60]->GPIO[60]
		(int *) (APPS_PAD_BASE | 0x00cc), 0xc0c1, 0x0,	//CLK_REQ
		(int *) (APPS_PAD_BASE | 0x00d0), 0xc0c0, 0x0,	//GPIO[124]
		(int *) (APPS_PAD_BASE | 0x00d4), 0xc0c1, 0x0,	//VCXO_REQ
		(int *) (APPS_PAD_BASE | 0x00d8), 0x00c0, 0x0,	//VCXO_OUT
		0x0,0x0,0x0 //termination
};
#endif

#if ZIMI_LED_SUPPORT
const CS_REGISTER_PAIR_S zimi_led_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x0144), 0xc000, 0x0,   //xyl add:GPIO26
		(int *) (APPS_PAD_BASE | 0x0148), 0xc000, 0x0,   //xyl add:GPIO27
		(int *) (APPS_PAD_BASE | 0x014C), 0xc000, 0x0,   //xyl add:GPIO28
		(int *) (APPS_PAD_BASE | 0x0164), 0xc000, 0x0,   //xyl add:GPIO34
		(int *) (APPS_PAD_BASE | 0x016c), 0xc000, 0x0,   //xyl add:GPIO36
		0x0,0x0,0x0 //termination
};
#endif

#if ZIMI_PB05
const CS_REGISTER_PAIR_S zimi_ci2c_pins[]=
{
		(int *) (APPS_PAD_BASE | 0x01B0), 0xc0c2, 0x0,	//CI2C_SCL
		(int *) (APPS_PAD_BASE | 0x01B4), 0xc0c2, 0x0,	//CI2C_SDA
		0x0,0x0,0x0 //termination
};
#endif
//-----------------------------------------------------------------------
// ConfigRegWrite
//
//
//-----------------------------------------------------------------------
static void ConfigRegWrite( P_CS_REGISTER_PAIR_S regPtr)
{
    UINT32_T i,tmp;
	while(regPtr->registerAddr != 0x0)
    {
      *(regPtr->registerAddr) = regPtr->regValue;
	  tmp = *(regPtr->registerAddr);  // ensure write complete
      regPtr++;
    }
}

//-----------------------------------------------------------------------
// ConfigRegResume
//
//
//-----------------------------------------------------------------------
void ConfigRegResume( P_CS_REGISTER_PAIR_S regPtr)
{
   UINT32_T i,tmp;

	while(regPtr->registerAddr != 0x0)
    {
      *(regPtr->registerAddr) &= 0xFFFFFFF7;
      tmp = *(regPtr->registerAddr);  // ensure write complete
      regPtr++;
    }
}

//-----------------------------------------------------------------------
// ConfigRegSave
//
//
//-----------------------------------------------------------------------
void ConfigRegSave( P_CS_REGISTER_PAIR_S regPtr)
{
    UINT32_T i,tmp;

	while(regPtr->registerAddr != 0x0)
    {
      regPtr->defaultValue = *(regPtr->registerAddr);
      regPtr++;
    }
}

//-----------------------------------------------------------------------
// ConfigRegRestore
//
//
//-----------------------------------------------------------------------
void ConfigRegRestore( P_CS_REGISTER_PAIR_S regPtr)
{
    UINT32_T i,tmp;

	while(regPtr->registerAddr != 0x0)
    {
      *(regPtr->registerAddr) = regPtr->defaultValue;
      tmp = *(regPtr->registerAddr);  // ensure write complete
      regPtr++;
    }
}

UINT_T ChipSelect2(void)
{
	return InvalidPlatformConfigError;
}


//-----------------------------------------------------------------------
// OneNAND, XIP 
//-----------------------------------------------------------------------
#if XIP
UINT_T ChipSelect0(void)
{
	ConfigRegWrite(cs0_reg);
	ConfigRegWrite( io_pins);
	ConfigRegWrite(common_pins);
	ConfigRegWrite(cs0_common_pins);
	ConfigRegWrite(xip_pins);
	return NoError;

}
#endif

//-----------------------------------------------------------------------
// ChipSelect for DFC
//
//
//-----------------------------------------------------------------------
#if NAND_CODE
void ChipSelectDFC( void )
{
   ConfigRegWrite(dfc_pins);	
   ConfigRegWrite(io_pins);
   ConfigRegWrite(common_pins);
	return;
}
#endif

#if MIFI3
void PlatformUSBChargeConfig(void)
{
	ConfigRegWrite(usb_charge_pins); // for MIFI3 only

	UINT_T RegisterValue;

	RegisterValue = *(UINT_T *)0xd4019020;
	*(UINT_T *)0xd4019020 = RegisterValue & (~BIT28); // GIPO_124 output 0
	RegisterValue = *(UINT_T *)0xd4019020; // read it

	RegisterValue = *(UINT_T *)0xd4019118;
	*(UINT_T *)0xd4019118 = RegisterValue | (BIT23); // GIPO_87 outupt 1
	RegisterValue = *(UINT_T *)0xd4019118; // read it

	RegisterValue = *(UINT_T *)0xd4019014;
	*(UINT_T *)0xd4019014 = RegisterValue | (BIT28); // GIPO_124 set output
	RegisterValue = *(UINT_T *)0xd4019014; // read it

	RegisterValue = *(UINT_T *)0xd401910c;
	*(UINT_T *)0xd401910c = RegisterValue | (BIT23); // GPIO_87 set output
	RegisterValue = *(UINT_T *)0xd401910c; // read it
	
	return;
}
#endif

#if UPDATE_USE_GPIO
void PlatformGPIOConfig(void)
{
	ConfigRegWrite(gpio_pins);

	UINT_T RegisterValue;

	RegisterValue = *(UINT_T *)(GPIO3_BASE + GPIO_PDR);
	*(UINT_T *)(GPIO3_BASE + GPIO_PDR) = RegisterValue & (~BIT28); // GIPO_124 set output
	RegisterValue = *(UINT_T *)(GPIO3_BASE + GPIO_PDR); // read it

	return;
}

UINT_T CheckGPIO(void)
{
	UINT_T RegisterValue, input;

	RegisterValue = *(UINT_T *)(GPIO3_BASE + GPIO_PLR);
	input = (RegisterValue >> 28) & 1; // Low?
	if (input == 0)
		return 1;
	else
		return 0;
}
#endif

#if ICASE
void PlatformUSBLimitSet(void)
{
	//(1) Pin_dir
	reg_write((GPIO1_BASE + GPIO_SDR), BIT18|BIT28);			//CHG_CONT, CHG_ENn --> output
	reg_write((GPIO1_BASE + GPIO_CDR), BIT17);				//CHG_CHGn			--->input
	reg_write((GPIO2_BASE + GPIO_SDR), BIT23|BIT22);			//SM_NCS0,BAT_SENSE_EN		--->output
	reg_write((GPIO3_BASE + GPIO_SDR), BIT28);				//GPIO_124			--->output

	//(2) Pin_level
	reg_write((GPIO1_BASE + GPIO_PCR), BIT28);				//CHG_CONT = 0
	reg_write((GPIO2_BASE + GPIO_PSR), BIT22);				//BAT_SENSE_EN =1

	//(3) P_MUX
	reg_write((0xD401E000+0x304),	0x0880|0);			//CHG_CONT,GPIO60

	reg_write((0xD401E000+0x2E0),	0x0880|1);			//CHG_CHGn,TDS_PAON, AF1, GPIO78
	reg_write((0xD401E000+0x2E4),	0x0880|1);			//CHG_ENn,TDS_PACTRL, AF1, GPIO79

	reg_write((0xD401E000+0x048),	0x0880|1);	//BAT_SENSE_EN,no pullup
	//&&Write_Reg((0xD401E000+0x04C),	0x0880|1);//CHG_AC_USB	SM_NCS0	
	reg_write((0xD401E000+0x0D4),	0x0880|1);	//CHG_AC_USB,VCXO_REQ,AF1,gpio125,
	
	reg_write((0xD401E000+0x0D0),	0x0880|0);	//CHG_MODE	GPIO_124

	// Set Current Limit to 500mA
	reg_write((GPIO3_BASE + GPIO_PCR),BIT29);                 //AC_USB =0
	reg_write((GPIO3_BASE + GPIO_PCR),BIT28);       //chg_mode=0
}
#endif

//-----------------------------------------------------------------------
// Setting up SSP for SPI Flash
//
//
//-----------------------------------------------------------------------
#if SPI_CODE
void ChipSelectSPI( void )
{
	//enabled SSP2 clock, then take out of reset
	serial_outstr("AutoCS 26MHz\n");
	reg_write(APBC_SSP2_CLK_RST, BIT0 | BIT1 | BIT2);
	reg_write(APBC_SSP2_CLK_RST, BIT0 | BIT1 | (2 << 4)); // 26MHZ
	
	ConfigRegWrite(spi_pins);

	return;
}

void ChipSelectSPI_CS(void)
{
	//enabled SSP2 clock, then take out of reset
	serial_outstr("ManuCS 26MHz\n");
	reg_write(APBC_SSP2_CLK_RST, BIT0 | BIT1 | BIT2);
	reg_write(APBC_SSP2_CLK_RST, BIT0 | BIT1 | (2 << 4)); // 26MHZ

	ConfigRegWrite(spi_pins_cs);
	return;
}
#endif

#if HSI
// HSI
void ChipSelectHSI( void )
{
//FIXME: link in the correct HSI clocks
	*(volatile unsigned int *)PMUA_HSI_CLK_RES_CTRL = 0x8;
	*(volatile unsigned int *)PMUA_HSI_CLK_RES_CTRL = 0x9;

	ConfigRegWrite(hsi_pins);

	//Map HSI interrupt to core and enable interrupts at the core.
	EnablePeripheralIRQInterrupt(INT_MIPI_HSI);

}

void RestoreHSIPins( void )
{
	ConfigRegRestore(hsi_pins);
}
#endif

//------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------
void RestoreDefaultConfig(void)
{
	UINT_T Temp;
	
#if NAND_CODE
	ConfigRegRestore(dfc_pins);
#endif
#if XIP
	ConfigRegRestore(cs0_reg );
#endif
	ConfigRegRestore(common_pins );	
	ConfigRegRestore(cs0_common_pins );
	ConfigRegRestore(io_pins );
#if XIP
    ConfigRegRestore(xip_pins );
#endif
#if MMC_CODE
    ConfigRegRestore(mmc3_pins );
#endif
#if SPI_CODE
	//ConfigRegRestore(spi_pins_cs);
#endif
#if LED_DISPLAY
	//ConfigRegRestore(led_pins);
#endif
#if OLED_SUPPORT
	ConfigRegRestore(oled_pins);
#endif
#if I2C
	Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_CDR);
	Temp &= ~BIT25; // GPIO20 is input direction
	*(VUINT_T *)(GPIO0_BASE + GPIO_PCR) = Temp;
	//ConfigRegRestore(pi2c_pins);
	ConfigRegRestore(charger_pins);
#endif
}

void SaveDefaultConfig(void)
{
#if NAND_CODE
	ConfigRegSave(dfc_pins);
#endif
#if XIP
	ConfigRegSave(cs0_reg );
#endif
	ConfigRegSave(common_pins );	
	ConfigRegSave(io_pins );
#if HSI
	ConfigRegSave(hsi_pins );
#endif
#if XIP
    ConfigRegSave(xip_pins );
#endif
    ConfigRegSave(cs0_common_pins );
#if MMC_CODE
    ConfigRegSave(mmc3_pins);
#endif
#if SPI_CODE
	ConfigRegSave(spi_pins);
#endif
#if LED_DISPLAY
	ConfigRegSave(led_pins);
#endif
#if OLED_SUPPORT
	ConfigRegSave(oled_pins);
#endif
#if I2C
	//ConfigRegSave(pi2c_pins);
	ConfigRegSave(charger_pins);
#endif
}

//#if UPDATE_USE_KEY
UINT_T PlatformKeypadConfig(void)
{
   *(volatile unsigned int *)APBC_KPC_CLK_RST = 0x03;
   
   ConfigRegWrite(keypad_pins);	
   // note: any clock and reset enabling for the keypad unit is done in InitKeypad
	return (NoError);
}
//#endif

UINT_T PlatformUARTConfig(void)
{
	ConfigRegWrite(uart_pins);	
	*(VUINT_T *)UART2_UCER = (BIT2 | BIT1 | BIT0);	
	*(VUINT_T *)UART2_UCER = (BIT1 | BIT0);	

	return (NoError);
}

#if USBCI
/**********************************
**      CI2 Specific Calls       **
***********************************/
static UINT32 ci2D0TurnOns = 0;
static UINT32 ci2ACCR1TurnOns = 0;
static UINT32 ci2DriverUp = 0;

//------------------------------------
//
//------------------------------------
void PlatformCI2InterruptEnable()
{
  EnablePeripheralIRQInterrupt(USB0_OTG_INT);
}

//---------------------------------
//
//---------------------------------
void PlatformCI2InterruptDisable()
{
  DisablePeripheralIRQInterrupt(USB0_OTG_INT);
}

//----------------------------------
//
//----------------------------------
UINT_T PlatformCI2Transmit(UINT_T length, UINT8_T* Buff, UINT_T WaitState)
{
    return CI2Transmit(length, Buff, WaitState);
}

//--------------------------------
//
//--------------------------------
void PlatformCI2InterruptHandler()
{
    ImageCI2InterruptHandler();
}

//-------------------------------------
// PlatformCI2Shutdown
//
//   Shut down OTG periperhal
//-------------------------------------
void PlatformCI2Shutdown()
{
    // Don't do anything if the driver
    // isn't UP
    //--------------------------------
    if (ci2DriverUp == 0 )
      return;
      
    BootRomCI2HWShutdown();
   
    // Power down PHY and PLL
    // only turn off only those that BR
    // had to turn on
    //------------------------------------
    //*ACCR1 &= (~ci2ACCR1TurnOns);   //DCB TTC-bringup
    
    // Use D0CKEN_C reg to turn off 26Mhz
    // clock, for PHY PLL input
    // only turn-off only those that BR
    // had to turn on
    //------------------------------------
    //*D0CKEN_C &= (~ci2D0TurnOns);   //DCB TTC-bringup

    //*OSCC &= ~0x800; // turn off CLK_POUT
    *(VUINT_T *)PMUA_USB_CLK_RES_CTRL = 0x0;  //turn off USB AXI clock
    ci2DriverUp = 0;
}

//------------------------------------------------------
// PlatformCI2Ready()
//
//       Wait max of 400us for PLL's to be locked and ready
//------------------------------------------------------
void PlatformCI2Ready( void )
{
  UINT32 startTime, endTime;
  
  
  startTime = GetOSCR0();  // Dummy read to flush potentially bad data
  startTime = GetOSCR0();
    
  do
  {
    if( ((*(volatile UINT_T *)USB2_PLL_CTRL_REG1) & USB2_PLL_READY_MASK) )
    	break;
    
	endTime = GetOSCR0();
	if (endTime < startTime)
		endTime += (0x0 - startTime);      
  }
  while( OSCR0IntervalInMicro(startTime, endTime) < 400 );
  
  //Ensure that phy control comes from OTG
  //-----------------------------------------
 // *USB2_PHY_T0 |= USB2_PHY_T0_OTG_CONTROL_BIT;
 // startTime = *USB2_PHY_T0;  // ensure write complete

}


#define RevisionID_A0	0xA0
#define RevisionID_A1	0xA1
#define RevisionID_Z3	0xE0
#define ChipID_A0		0xC828
#define ChipID_Z3		0x1802
//----------------------------------------------
// OTG PHY setup for TPV
//
//----------------------------------------------
void PlatformCI2Init()
{
	UINT_T Temp;
	UINT32 startTime, endTime;

	Temp = *(VUINT_T *)CHIP_ID;

    // Prevent re-init if already up
    //------------------------------
    if( ci2DriverUp != 0 )
       return;
    
	// USB AXI: Enable the clocks and bring the link out of reset
	if (((Temp & CHIP_ID_REV_ID_MSK) >> CHIP_ID_REV_ID_BASE) == RevisionID_Z3)
	{
		*(VUINT_T *)PMUA_USB_CLK_RES_CTRL = 0x8;
	    *(VUINT_T *)PMUA_USB_CLK_RES_CTRL = 0xb;

		// disable timer2 since OBM doesn't use it for switch
		BU_REG_WRITE(TMR1_CER, BU_REG_READ(TMR1_CER) & ~(0x1 << 0x1));
	}
	else
	{
	    *(VUINT_T *)PMUA_USB_CLK_RES_CTRL |= PMUA_USB_CLK_RES_CTRL_USB_AXICLK_EN;
	    *(VUINT_T *)PMUA_USB_CLK_RES_CTRL |= PMUA_USB_CLK_RES_CTRL_USB_AXI_RST;
	}


    //wait for 200 us
	Delay( 200 );

    // USB2_PLL_CTRL_REG0
    if ((((Temp & CHIP_ID_REV_ID_MSK) >> CHIP_ID_REV_ID_BASE) == RevisionID_A0) 
		|| (((Temp & CHIP_ID_REV_ID_MSK) >> CHIP_ID_REV_ID_BASE) == RevisionID_A1)
		|| (((Temp & CHIP_ID_REV_ID_MSK) >> CHIP_ID_REV_ID_BASE) == RevisionID_Z3))
    {
 		BU_REG_WRITE( USB2_PLL_CTRL_REG0, 0x5af0 );
    }
 	else
 	{
		BU_REG_WRITE( USB2_PLL_CTRL_REG0, BU_REG_READ(USB2_PLL_CTRL_REG0) & ~(USB2_PLL_FBDIV_MASK | USB2_PLL_REFDIV_MASK) );
		BU_REG_WRITE( USB2_PLL_CTRL_REG0, BU_REG_READ(USB2_PLL_CTRL_REG0) | (0xf0<<USB2_PLL_FBDIV_BASE |
											  							 0xd<<USB2_PLL_REFDIV_BASE) );
 	}

	// USB2_PLL_CTRL_REG1
	BU_REG_WRITE( USB2_PLL_CTRL_REG1, BU_REG_READ(USB2_PLL_CTRL_REG1) & ~(USB2_PLL_CAL12_MASK | 
											   							  USB2_PLL_KVCO_MASK | 
											   							  USB2_PLL_ICP_MASK) );
	BU_REG_WRITE( USB2_PLL_CTRL_REG1, BU_REG_READ(USB2_PLL_CTRL_REG1) | (0x3<<USB2_PLL_CAL12_BASE |
											  				   			 0x3<<USB2_PLL_KVCO_BASE |
											  				   			 0x3<<USB2_PLL_ICP_BASE |
											  				   			 0x1<<USB2_PLL_LOCK_BYPASS_BASE |
											  				   			 0x1<<USB2_PLL_PU_PLL_BASE) );

	// USB2_TX_CTRL_REG0
	BU_REG_WRITE( USB2_TX_CTRL_REG0, BU_REG_READ(USB2_TX_CTRL_REG0) & ~(USB2_TX_IMPCAL_VTH_MASK) );
	BU_REG_WRITE( USB2_TX_CTRL_REG0, BU_REG_READ(USB2_TX_CTRL_REG0) | (0x2<<USB2_TX_IMPCAL_VTH_BASE) );
	
	// USB2_TX_CTRL_REG1
	BU_REG_WRITE( USB2_TX_CTRL_REG1, BU_REG_READ(USB2_TX_CTRL_REG1) & ~(USB2_TX_CK60_PHSEL_MASK |
											  				  			USB2_TX_AMP_MASK |
											  				  			USB2_TX_VDD12_MASK) );
	BU_REG_WRITE( USB2_TX_CTRL_REG1, BU_REG_READ(USB2_TX_CTRL_REG1) | (0x4<<USB2_TX_CK60_PHSEL_BASE |
											 				 		   0x4<<USB2_TX_AMP_BASE |
											 				 		   0x3<<USB2_TX_VDD12_BASE) );

    // USB2_TX_CTRL_REG2
	BU_REG_WRITE( USB2_TX_CTRL_REG2, BU_REG_READ(USB2_TX_CTRL_REG2) & ~(USB2_TX_DRV_SLEWRATE_MASK));
	BU_REG_WRITE( USB2_TX_CTRL_REG2, BU_REG_READ(USB2_TX_CTRL_REG2) | (0x3<<USB2_TX_DRV_SLEWRATE_BASE) );
                                                                    
	// USB2_RX_CTRL_REG0
	BU_REG_WRITE( USB2_RX_CTRL_REG0, BU_REG_READ(USB2_RX_CTRL_REG0) & ~(USB2_RX_SQ_THRESH_MASK |
											  							USB2_RX_SQ_LENGTH_MASK) );
	BU_REG_WRITE( USB2_RX_CTRL_REG0, BU_REG_READ(USB2_RX_CTRL_REG0) | (0xa<<USB2_RX_SQ_THRESH_BASE |
										   							   0x2<<USB2_RX_SQ_LENGTH_BASE) );

	// USB2_ANA_REG0
	//BU_REG_WRITE( USB2_ANA_REG0, BU_REG_READ(USB2_ANA_REG0) | (0x1<<USB2_ANA_DIG_SEL_BASE |
	//									   					   0x1<<USB2_ANA_VDD_DIG_TOP_SEL_BASE) );
	
	// USB2_ANA_REG1
	BU_REG_WRITE( USB2_ANA_REG1, BU_REG_READ(USB2_ANA_REG1) | (0x1<<USB2_ANA_PU_BASE) );
	
	// USB2_OTG_REG0
	BU_REG_WRITE( USB2_OTG_REG0, BU_REG_READ(USB2_OTG_REG0) | (0x1<<USB2_OTG_PU_BASE) );
  
    BU_REG_WRITE(USB2_ICID_REG0,  0x9011);
    BU_REG_WRITE(USB2_ICID_REG1,  0x0);

	// Enable the ANA_GRP bit in the PU_REF register
	//	The ANA_GRP bit enables the analog logic blocks
	*(volatile UINT_T *)UTMI_CTRL |= (UTMI_CTRL_PU_REF | UTMI_CTRL_PU_CLK); 

	// Enable the UTMI PLL and power up the UTMI unit
    *(volatile UINT_T *)UTMI_CTRL |= (UTMI_CTRL_PU_PLL | UTMI_CTRL_PU);

    /* toggle VCOCAL_START bit of USB2_PLL_CTRL_REG1 */
    Delay(200);
    BU_REG_WRITE(USB2_PLL_CTRL_REG1, BU_REG_READ(USB2_PLL_CTRL_REG1) | (USB2_PLL_VCOCAL_START_MASK) );
	// For Eshel and MMP3, do not set VCOCAL_START back to zero. Comment the following two lines.
    //Delay(40);
    //BU_REG_WRITE(USB2_PLL_CTRL_REG1, BU_REG_READ(USB2_PLL_CTRL_REG1) & (~USB2_PLL_VCOCAL_START_MASK) );

    /* toggle RCAL_START bit of USB2_TX_CTRL_REG0 */
    Delay(400);
    BU_REG_WRITE(USB2_TX_CTRL_REG0, BU_REG_READ(USB2_TX_CTRL_REG0) | (USB2_TX_RCAL_START_MASK) );
    Delay(40);
    BU_REG_WRITE(USB2_TX_CTRL_REG0, BU_REG_READ(USB2_TX_CTRL_REG0) & (~USB2_TX_RCAL_START_MASK) );
    Delay(400);

    // end of hw setup

    // Step 6: Initialize OTG software and hardware
    // need to poll the PLL_LOCK_RDY bit in ACCR1
    // before turning on OTG "RUN" bit
    //--------------------------------------------
#if CI2_USB_DDR_BUF  
	BootRomCI2SWInit( PlatformCI2InterruptEnable, PlatformCI2Ready,
                     ( CI_OTG_OPTIONS_8BIT_XCVR_WIDTH  |
                       CI_OTG_OPTIONS_UTMI_XCVR_SELECT 
					 ));
	// ensure full spead is not being forced:
	// we use 512 byte packets and must run in USB 2.0 mode
	*(unsigned long*)0xd4208184 &= ~(1u<<24);
#else    
    // Running USB at highspeed but packet size is still 64 bytes. For HSIC requirement.
	BootRomCI2SWInit( PlatformCI2InterruptEnable, PlatformCI2Ready,
                     ( CI_OTG_OPTIONS_8BIT_XCVR_WIDTH  |
                       CI_OTG_OPTIONS_UTMI_XCVR_SELECT
 #if !SLE_TESTING	   //enable HIGH speed for SLE, enable FULL speed for release
					   // | CI_OTG_OPTIONS_FULL_SPEED 
 #endif
                     ));
    // full speed works, so don't force it anymore. try highspeed (if the chirp passed)
	// force full speed mode temporarily
	//*(unsigned long*)0xd4208184 |= (1u<<24);
    *(unsigned long*)0xd4208184 &= ~(1u<<24);
    
#endif    
	
	// Limiting USB burst length to 8. AXI2MC module used in Wukong-Y0 does not support 16.
	*(unsigned long*)0xd4208090 = 0x2;
    
    // THIS IS HSIC REQUIRED STUFF BUT DOES NT HURT USB
    //Program USBMODE[15]=1’b1
    //USB_MODE @0xd42081a8
    BU_REG_WRITE(USB_MODE, (BU_REG_READ(USB_MODE)| (1<<15 | 0x2 | 0x8)));
    
    // We don't have a reliable way to detect if Vbus is high. So set Run bit on controller.
    BU_REG_WRITE(USB_CMD, BU_REG_READ(USB_CMD) | USB_CMD_RS);

	ci2DriverUp = 1;

	//Temp = *(VUINT_T *)USB_PORTSC;
	//serial_outstr("USB_PORTSC\n");
	//serial_outnum(Temp);
	//serial_outstr("\n");
}
#endif


/*
 * Set NAND and UART clocks in the PMUM Clock Gating Register
 */
void CheckDefaultClocks(UINT_T *value)
{
	UINT_T Temp;
	UINT_T ID_Mask;
	UINT_T CPU_Mode;

	// Assign ID mask
	ID_Mask = 0x0000fff0;

	getCPUMode( &CPU_Mode, &ID_Mask);
	if (CPU_Mode == ARM926ID)	// AP = 926
	{
		*(VUINT_T *)PMUM_ACGR |= (PMUM_ACGR_APMU_52M
						| PMUM_ACGR_AP_26M 
						| PMUM_ACGR_AP_SUART
						| PMUM_ACGR_AP_6P5M 
						| PMUM_ACGR_APMU_624M // allow SDH unit to send out commands, assert cmd_oe and let sd_clk toggle. 
						);
	}
	else	
	{
		*(VUINT_T *)PMUM_CCGR = 0xFFFFFFFF;
		/*
		*(VUINT_T *)PMUM_CCGR |= (PMUM_CCGR_APMU_52M
					| PMUM_CCGR_AP_26M 
					| PMUM_CCGR_G_SUART
					| PMUM_CCGR_AP_6P5M 
					| PMUM_CCGR_APMU_624M // allow SDH unit to send out commands, assert cmd_oe and let sd_clk toggle. 
					);
		*/
	}	

	// AIB unit must be out of reset for MFPR set up
	*(VUINT_T *)APBC_AIB_CLK_RST = 0x7;
	Temp = *(VUINT_T *)APBC_AIB_CLK_RST;
	*(VUINT_T *)APBC_AIB_CLK_RST = 0x3;
	Temp = *(VUINT_T *)APBC_AIB_CLK_RST;

	// Get the GPIO unit out of reset, too.
	*(VUINT_T *)APBC_GPIO_CLK_RST = 0x7;
	Temp = *(VUINT_T *)APBC_GPIO_CLK_RST;
	*(VUINT_T *)APBC_GPIO_CLK_RST = 0x3;
	Temp = *(VUINT_T *)APBC_GPIO_CLK_RST;

	*(VUINT_T *)APB_SPARE1_REG |= BIT12 | BIT13;
	*value = *(VUINT_T *)APB_SPARE1_REG;

	return;
}

static unsigned int NF_CLK_default;
void PlatformNandClocksEnable()
{
	UINT_T Temp;

	//save reset value
	NF_CLK_default = *(VUINT_T *)PMUA_NF_CLK_RES_CTRL;
	// Enable DFC clocks
	*(VUINT_T *)PMUA_NF_CLK_RES_CTRL &= ~PMUA_NF_CLK_RES_CTRL_NF_CLK_SEL;
	*(VUINT_T *)PMUA_NF_CLK_RES_CTRL |= (PMUA_NF_CLK_RES_CTRL_NF_AXICLK_EN | PMUA_NF_CLK_RES_CTRL_NF_CLK_EN | PMUA_NF_CLK_RES_CTRL_NF_ECC_CLK_EN);
    Temp = *(VUINT_T *)PMUA_NF_CLK_RES_CTRL;
	return;
}
void PlatformNandClocksDisable()
{
	UINT_T Temp;

	// restore default value
    *(VUINT_T *)PMUA_NF_CLK_RES_CTRL = NF_CLK_default;
	return;
}

#if MMC_CODE
// MMC
CONTROLLER_TYPE ConfigureMMC(UINT8_T FlashNum, UINT_T *pBaseAddress, UINT_T *pInterruptMask, UINT_T *pFusePartitionNumber)
{
  // Tavor TD only supports MMC3 boot	
  ConfigRegSave(mmc3_pins);
  ConfigRegWrite(mmc3_pins);
  *pBaseAddress = SDHC0_2_BASE;
  *pInterruptMask = INT_MMC;
		
  //Enable PMUA clock for this interface
  // Notes:
  // Documentation should state that:
  // PMUA_SDH0_CLK_RES_CTRL -> offset 0x054
  // PMUA_SDH1_CLK_RES_CTRL -> offset 0x058
  // PMUA_SDH2_CLK_RES_CTRL -> offset 0x0E0
  //
  // However, out header file PMUA.H states:
  // PMUA_SDH0_CLK_RES_CTRL -> offset 0x054
  // PMUA_SDH1_CLK_RES_CTRL -> offset 0x0E0

  // We need to write to SDH0 (0x054) and SDH2 (0x0E0) so
  // use SDH0 (correct offset) and SDH1 (really SDH2 offset)
  *(UINT_T *) PMUA_SDH0_CLK_RES_CTRL = 0x09;
  // now write to SDH2 (even though it says SDH1)
  *(UINT_T *) PMUA_SDH1_CLK_RES_CTRL = 0x1B;

  // Now pick the partition we are supposed to boot from.	
 *pFusePartitionNumber = MMC_SD_USER_PARTITION;		

	return MMCSDHC1_1;	   // MMC3
}

void DisableMMCSlots()
{
	DisablePeripheralIRQInterrupt(INT_MMC);

	// Disable PMUA clocks for the SD controllers
	*(UINT_T *) PMUA_SDH1_CLK_RES_CTRL = 0x0;	//Disable PMUA clock for this interface

	// Restores the MMC GPIO's back to their default values
	ConfigRegRestore(mmc3_pins);
}

UINT_T MMCHighSpeedTimingEnabled()
{
    return 0;
}
#endif

// Get the VID and PID from fuses
UINT_T GetUSBIDFuseBits(unsigned short* VID, unsigned short* PID )
{
	return NotSupportedError;
}

/*******************************************************************************************/
/*                                                                                         */
/*                                                                                         */
/*   I2C Stuff                                                                             */
/*                                                                                         */
/*                                                                                         */
/*******************************************************************************************/
void PlatformI2CClocksEnable(void)
{
	unsigned long	temp;

	// the I2C clock must be running for the PJ:
	*(volatile unsigned long*)PMUM_ACGR |= ( (1u<<6) | (1u<<4) );	// not sure if the correct bit is 4 or 6.
	temp = *(volatile unsigned long*)PMUM_ACGR;						// read back to ensure write completes.

	// do the CP, too: the I2C clock must be running
	*(volatile unsigned long*)PMUM_CCGR |= ( (1u<<6) | (1u<<4) );	// not sure if the correct bit is 4 or 6.
	temp = *(volatile unsigned long*)PMUM_CCGR;						// read back to ensure write completes.

	// clock the twsi & pull it out of reset
	*(volatile unsigned long*)APBC_TWSI_CLK_RST=4;					// reset APB and TWSI.
	temp = *(volatile unsigned long*)APBC_TWSI_CLK_RST;			// read back to ensure write completes.
	*(volatile unsigned long*)APBC_TWSI_CLK_RST=7;					// still in reset, but get clocks going.
	temp = *(volatile unsigned long*)APBC_TWSI_CLK_RST;			// read back to ensure write completes.
	*(volatile unsigned long*)APBC_TWSI_CLK_RST=3;					// out of reset, with clocks running.
	temp = *(volatile unsigned long*)APBC_TWSI_CLK_RST;			// read back to ensure write completes.
}

#if I2C
UINT_T PlatformChargerConfig(void)
{
	UINT_T Temp;
	
	ConfigRegWrite(charger_pins);
	
#if MIFI_V3R0 || MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
	//Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_CDR);
	//Temp &= ~BIT25; // GPIO20 is input direction
	//*(VUINT_T *)(GPIO0_BASE + GPIO_PCR) = Temp;
#else
	Temp = *(UINT_T *)(GPIO0_BASE + GPIO_PSR);
	Temp &= ~BIT21; // GIPO21 outupt 0
	Temp &= ~BIT24; // GIPO24 outupt 0
	*(UINT_T *)(GPIO0_BASE + GPIO_PSR) = Temp;

	Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_SDR);
	Temp |= BIT24 | BIT21; // GPIO24/21 is ouput direction
	*(VUINT_T *)(GPIO0_BASE + GPIO_PDR) = Temp;

	Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_CDR);
	Temp &= ~BIT20; // GPIO20 is input direction
	*(VUINT_T *)(GPIO0_BASE + GPIO_PCR) = Temp;
#endif
	return NoError;
}

UINT_T PlatformPI2CConfig(void)
{
	UINT_T Temp;
	
	ConfigRegWrite(pi2c_pins);

	Temp = *(UINT_T *)(GPIO3_BASE + GPIO_PSR);
	Temp &= ~BIT26; // GIPO122 outupt 0
	*(UINT_T *)(GPIO3_BASE + GPIO_PSR) = Temp;
	
	Temp = *(UINT_T *)(GPIO1_BASE + GPIO_PSR);
	Temp &= ~BIT28; // GIPO60 outupt 0
	*(UINT_T *)(GPIO1_BASE + GPIO_PSR) = Temp;

	Temp = *(VUINT_T *)(GPIO3_BASE + GPIO_SDR);
	Temp |= BIT26; // GPIO122 is ouput direction
	*(VUINT_T *)(GPIO3_BASE + GPIO_PDR) = Temp;

	Temp = *(VUINT_T *)(GPIO1_BASE + GPIO_SDR);
	Temp |= BIT28; // GPIO60 is ouput direction
	*(VUINT_T *)(GPIO1_BASE + GPIO_PDR) = Temp;

	return NoError;
}
#endif

#if OLED_SUPPORT
UINT_T PlatformOLEDConfig(void)
{
	UINT_T Temp;
	
	ConfigRegWrite(oled_pins);

	//(2)Pin Dir: out
	Temp = *(UINT_T *)(GPIO1_BASE + GPIO_PDR);
	reg_write((GPIO1_BASE + GPIO_PDR), Temp | BIT3);	//GPIO35

	//(3)Reset
	reg_write((GPIO1_BASE + GPIO_PSR), BIT3);	//GPIO35
	Delay_us(1000 * 50);
	reg_write((GPIO1_BASE + GPIO_PCR), BIT3);
	Delay_us(1000 * 10);
	reg_write((GPIO1_BASE + GPIO_PSR), BIT3);
	Delay_us(1000 * 10);

	return NoError;
}

#if NEZHA_MIFI_V4R1
UINT_T PlatformOLEDPowerON(void)
{
	UINT_T Temp;
	
	ConfigRegWrite(oled_power_pins);

	//out
	Temp = *(UINT_T *)(GPIO0_BASE + GPIO_PDR);
	reg_write((GPIO0_BASE + GPIO_PDR), Temp | BIT25);	//GPIO25

	//set bit
	reg_write((GPIO0_BASE + GPIO_PSR), BIT25);	//GPIO25
	Delay_us(1000 * 50);

	return NoError;
}

UINT_T PlatformOLEDPowerOFF(void)
{
	UINT_T Temp;
	
	//out
	Temp = *(UINT_T *)(GPIO0_BASE + GPIO_PDR);
	reg_write((GPIO0_BASE + GPIO_PDR), Temp | BIT25);	//GPIO25

	//clear bit
	reg_write((GPIO0_BASE + GPIO_PCR), BIT25);	//GPIO25
	Delay_us(1000 * 10);

	return NoError;
}
#endif
#endif

#if LED_DISPLAY
UINT_T PlatformLEDConfig(void)
{
	ConfigRegWrite(led_pins);

#if MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
	reg_write((GPIO0_BASE + GPIO_SDR),(BIT3|BIT13|BIT28));	//output
	reg_write((GPIO1_BASE + GPIO_SDR),(BIT1|BIT2|BIT3|BIT4|BIT17));	//output
	reg_write((GPIO2_BASE + GPIO_SDR),(BIT5|BIT6));	//output
	reg_write((GPIO3_BASE + GPIO_SDR),(BIT30|BIT31));	//output
#elif MIFI_V3R0
	reg_write((GPIO0_BASE + GPIO_SDR),(BIT0|BIT2|BIT3|BIT13|BIT28));	//output
	reg_write((GPIO1_BASE + GPIO_SDR),(BIT1|BIT2|BIT3|BIT4|BIT17));	//output
	reg_write((GPIO3_BASE + GPIO_SDR),(BIT30|BIT31));	//output
#else
	reg_write((GPIO0_BASE + GPIO_SDR),(BIT14|BIT15|BIT16|BIT25|BIT26|BIT27|BIT28));	//output
	reg_write((GPIO1_BASE + GPIO_SDR),(BIT1|BIT2|BIT3|BIT4|BIT17));	//output
#endif

	return NoError;
}
#endif

#if !BOOTROM
void FuseOverwriteForDownload(pFUSE_SET pFuses)
{
	// Nothing to do for WKNG as there are no FUSEs. 
}
#endif

void PlatformSetForceUSBEnumFlag(void)
{
	*(VUINT_T *)FORCE_USB_ENUM_ADDR = FORCEUSBID;
}

UINT_T PlatformCheckForceUSBEnumFlag(void)
{
	
	if (*(VUINT_T *)FORCE_USB_ENUM_ADDR == FORCEUSBID)
		return 1;
	else
		return 0;
}

void PlatformClearForceUSBEnumFlag(void)
{
	*(VUINT_T *)FORCE_USB_ENUM_ADDR = 0x0;
}

void PlatformPrepareOBMVersion(void)
{
	*(VUINT_T *)OBM_VERSION_TO_CP_ADDR = 0x04070214; // update with bootloader.h
}

#if RT8973
UINT_T PlatformUSBSwitchConfig(void)
{
	UINT_T Temp;
	
	ConfigRegWrite(usb_switch_pins);

	return NoError;
}
#endif

#if PRODUCTION_MODE_SUPPORT
void PlatformCheckProductionMode(void)
{
	UINT_T Temp, gotkey, k;
	UINT_T RegisterValue, i = 0, j = 0, production_mode = 0;

#if MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
	production_mode = check_attached();

	#if !SBOOT
	if (production_mode == 0)
	{
		ConfigRegWrite(production_mode_pins);

		*(VUINT_T *)(GPIO1_BASE + GPIO_CDR) |= BIT24;
		Temp = *(VUINT_T *)(GPIO1_BASE + GPIO_CDR);
		Temp = *(VUINT_T *)(GPIO1_BASE + GPIO_PLR);

		for (i = 0; i < 20; i++)
		{
			RegisterValue = *(VUINT_T *)(GPIO1_BASE + GPIO_PLR);
			if (((RegisterValue >> 24) & 1) == 0)
			{
				j += 1;
			}

			if (j > 10)
			{
				production_mode = 1;
				break;
			}

			Delay(1000);
		}
	}
	#endif
#elif MIFI_V2R0 || MIFI_V2R1 || NEZHA_MIFI_V3R1 || NEZHA_MIFI_V4R1R1
	PlatformKeypadConfig();
    InitializeKeypad();

	for (i = 0; i < 20; i++)
    {
        gotkey = ReadKeypad(&k);
		#if NEZHA_MIFI_V4R1R1
		if( k == 0x8 )
		#else
        if( k == 0x4 )
		#endif
        {
        	production_mode = 1;
			break;
        }
    }
#elif NEZHA_MIFI_V4R1
	PlatformKeypadConfig();
    InitializeKeypad();

	for (i = 0; i < 20; i++)
    {
        gotkey = ReadKeypad(&k);
        if( k == 0x1 )
        {
        	production_mode = 1;
			break;
        }
    }
#endif

	if (production_mode == 1)
	{
		*(VUINT_T *)PRODUCTION_MODE_FLAG_ADDR = PRODUCTION_MODE;
		serial_outstr("Production Mode\n");
	}
	else
	{
		*(VUINT_T *)PRODUCTION_MODE_FLAG_ADDR = 0;
		serial_outstr("NO Production Mode\n");
	}
}

void PlatformClearProductionMode(void)
{
	*(VUINT_T *)PRODUCTION_MODE_FLAG_ADDR = 0;
	serial_outstr("Force to Non-Production Mode\n");
}

#endif

void PlatformProtectBootBlocks(void)
{
#if SPI_CODE && (MIFI_V3R2 || MIFI_V3R3) && PRODUCTION_MODE_SUPPORT
	if (*(VUINT_T *)PRODUCTION_MODE_FLAG_ADDR != PRODUCTION_MODE)
		MX_SPINOR_Protect_Blocks();
#endif
}

#if MIFI_V3R3
void PlatformConfigBatteryDetectGPIO(void)
{
	ConfigRegWrite(battery_detect_pins);
}

UINT8_T PlatformGetBatteryStatus(void)
{
	UINT_T Temp, RegisterValue;
	
#if NAND_CODE
	*(VUINT_T *)(GPIO3_BASE + GPIO_CDR) |= BIT28; // configure GPIO124 as input
	Temp = *(VUINT_T *)(GPIO3_BASE + GPIO_CDR);
	Temp = *(VUINT_T *)(GPIO3_BASE + GPIO_PLR);


	RegisterValue = *(VUINT_T *)(GPIO3_BASE + GPIO_PLR);
	if (((RegisterValue >> 28) & 1) == 0) // Low status means batter is present
	{
		serial_outstr("GPIO124: battery detected\n");
		return 1;
	}
	else
	{
		serial_outstr("GPIO124: battery not detected\n");
		return 0;
	}
#else	
	*(VUINT_T *)(GPIO0_BASE + GPIO_CDR) |= BIT8; // configure GPIO8 as input
	Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_CDR);
	Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_PLR);


	RegisterValue = *(VUINT_T *)(GPIO0_BASE + GPIO_PLR);
	if (((RegisterValue >> 8) & 1) == 0) // Low status means batter is present
	{
		serial_outstr("GPIO8: battery detected\n");
		return 1;
	}
	else
	{
		serial_outstr("GPIO8: battery not detected\n");
		return 0;
	}
#endif
}
#endif

#if LWG_LTG_SUPPORT
UINT_T PlatformCheckLTGLWGFlag(void)
{
	return *(VUINT_T *)LTG_LWG_FLAG_ADDR;
}

UINT_T PlatformSetLTGLWGFlag(UINT_T flag)
{
	*(VUINT_T *)LTG_LWG_FLAG_ADDR = flag;
}

UINT_T PlatformClearLTGLWGFlag(void)
{
	*(VUINT_T *)LTG_LWG_FLAG_ADDR = 0x0;
}
#endif

#if MRD_CHECK
void PlatformSetCurrentMRDAddr(UINT_T address)
{
	*(VUINT_T *)MRD_FLASH_ADDRESS = address;
}

void PlatformSetMRDAddr(UINT_T address, UINT_T image_ID)
{
#if LWG_LTG_SUPPORT
	if ((image_ID == RELIABLEID) || (image_ID == LTGRELIABLEID))
		*(VUINT_T *)FIRST_MRD_FLASH_ADDRESS = address;

	if ((image_ID == LWGRESERVED1ID) || (image_ID == LTGRESERVED1ID))
		*(VUINT_T *)SECOND_MRD_FLASH_ADDRESS = address;
#else
	if (image_ID == RELIABLEID)
		*(VUINT_T *)FIRST_MRD_FLASH_ADDRESS = address;

	if (image_ID == LTGRELIABLEID)
		*(VUINT_T *)SECOND_MRD_FLASH_ADDRESS = address;
#endif
}
#endif

UINT_T PlatformCheckResetFlag(void)
{
	return *(VUINT_T *)RESET_FLAG_ADDRESS;
}

#if SILENT_RESET
UINT_T PlatformCheckSilentReset(void)
{
	if ((*(VUINT_T *)SILENT_RESET0_ADDRESS == SILENT_RESET0_MAGIC) &&
		(*(VUINT_T *)SILENT_RESET1_ADDRESS == SILENT_RESET1_MAGIC))
		return 1;
	else
		return 0;
}
#endif

#if SBOOT
UINT_T PlatformCheckSbootMode(void)
{
	UINT_T Temp;
	UINT_T RegisterValue, i = 0, j = 0, sboot_mode = 0;

	ConfigRegWrite(sboot_wps_pins);

	*(VUINT_T *)(GPIO1_BASE + GPIO_CDR) |= BIT24;
	Temp = *(VUINT_T *)(GPIO1_BASE + GPIO_CDR);
	Temp = *(VUINT_T *)(GPIO1_BASE + GPIO_PLR);

	for (i = 0; i < 20; i++)
	{
		RegisterValue = *(VUINT_T *)(GPIO1_BASE + GPIO_PLR);
		if (((RegisterValue >> 24) & 1) == 0)
		{
			j += 1;
		}

		if (j > 10)
		{
			sboot_mode = 1;
			break;
		}

		Delay(1000);
	}

	if (*(VUINT_T *)0x07D7F100 == 0x52415453)
	{
		*(VUINT_T *)0x07D7F100 = 0x0; // clear it
		serial_outstr("AT command force to Sboot\n");
		sboot_mode = 1;
	}
	else
	{
		serial_outstr("No AT command flag\n");
	}

	if (sboot_mode == 1)
	{
		serial_outstr("Sboot Mode\n");
		return 0xaa;
	}
	else
	{
		serial_outstr("NO Sboot Mode\n");
		return 0x55;
	}
}
#endif

#if TR069_SUPPORT
void PlatformSetTR069Flag(void)
{
	*(VUINT_T *)TR069_FLAG_ADDR = 0x50415353; // "PASS"
}
#endif

#if BACKUP_IMAGE
UINT_T PlatformCheckBackupImagesFlag(void)
{
	serial_outstr("0x07D7F040\n");
	serial_outnum(*(VUINT_T *)BACKUP_IMAGES_ADDR);
	serial_outstr("\n");

	#if ZIMI_PB05
	UINT_T m_temp;

	m_temp = *(VUINT_T *)BACKUP_IMAGES_ADDR;
	*(VUINT_T *)BACKUP_IMAGES_ADDR = 0x4D415853;

	return m_temp;
	#endif
	
	return *(VUINT_T *)BACKUP_IMAGES_ADDR;
}

void PlatformClearBackupImagesFlag(void)
{
	*(VUINT_T *)BACKUP_IMAGES_ADDR = 0x4D415853; // CP
}
#endif

#if ZIMI_LED_SUPPORT
void zimi_led_init()
{
	ConfigRegWrite(zimi_led_pins);

	serial_outstr("we init led here\n");

	//set all LED GPIOs to output state
	//GPIO26
	reg_write((GPIO0_BASE + GPIO_SDR), BIT26);
	//GPIO27
	reg_write((GPIO0_BASE + GPIO_SDR), BIT27);
	//GPIO28
	reg_write((GPIO0_BASE + GPIO_SDR), BIT28);
	//GPIO33
	reg_write((GPIO1_BASE + GPIO_SDR),BIT1);
	//GPIO34
	reg_write((GPIO1_BASE + GPIO_SDR),BIT2);
	//GPIO36
	reg_write((GPIO1_BASE + GPIO_SDR), BIT4);
}
#endif

#if ZIMI_PB05
void PlatfromBqConfig()
{
	ConfigRegWrite(zimi_ci2c_pins);
}
#endif
