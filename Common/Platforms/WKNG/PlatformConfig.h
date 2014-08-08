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
 
 ******************************************************************************
**
**  FILENAME:	PlatformConfig.h
**
**  PURPOSE: 	Platform specific header to set parameters
**
**
******************************************************************************/

#ifndef __platform_config_h
#define __platform_config_h

#include "Typedef.h"
#include "Errors.h"
#include "ProtocolManager.h"
#include "misc.h"
#include "GPIO.h"
#include "general.h"
#include "HM2uart.h"
#include "xllp_ciu2d.h"
#include "smemc.h"
#include "CIU.h"
#include "PMUM.h"
#include "APBC.h"
#include "resume.h"
#include "sdmmc_api.h"
#include "SD.h"
#if BOOTROM
#include "bootrom.h"
#endif

//Number of images to load
#define K_MAX_IMAGES_TO_LOAD_FROM_FLASH 2	// TIM, OBM
#define K_MAX_IMAGES_TO_LOAD_FROM_ROM   0   
#define K_MAX_IMAGES_TO_LOAD   (K_MAX_IMAGES_TO_LOAD_FROM_FLASH + K_MAX_IMAGES_TO_LOAD_FROM_ROM)
#define K_MAX_MEMORY_BOUNDS_CHECK_ENTRIES  (K_MAX_IMAGES_TO_LOAD + 1) // Add 1 for stack and heap in ISRAM
#define KEYID_SOFTWAREUPGRADEREQUEST 0x08		// this is the top left key: Key_WiFi

// Timers
#define APB_CU_ENABLE (*APB_CU_CR = 0x33)
#define TIMER_FREQ(A) (((A)*13) >> 2)

//HSI
#define	HSI_BASE	0xD1000000

// SPI
#define SSP_BASE_FOR_SPI		 SSP3_BASE
#define spi_reg_bit_set(reg, val)
#define spi_reg_bit_clr(reg, val)
#define SSP_RX_DMA_DEVICE		DMAC_SSP_2_RX
#define SSP_TX_DMA_DEVICE		DMAC_SSP_2_TX

//
// Clock in nanoseconds. Not likely to be changing all that often...
//
#define NAND_CONTROLLER_CLOCK  156  // MHz
#define UARTCLK    14745600

//
// Clock in nanoseconds. Not likely to be changing all that often...
//

// Bit 19 of control register must be written as 0
#define DFC_INITIAL_CONTROL 0x8C021FFF
#define	DFC_INITIAL_TIMING0	0x003F3F3F
#define DFC_INITIAL_TIMING1 0x100080FF //tR set to 52.4 usec

#define TIMOffset_CS0 0x00000000 // offset for TIM on CS0

#define ASCR	(volatile unsigned long *)PMUM_CPSR //See PMUM.h
#define ARSR	(volatile unsigned long *)PMUM_CRSR
#define AD3SR	(volatile unsigned long *)PMUM_AWUCRS
#define AD3R	(volatile unsigned long *)PMUM_AWUCRM

#define APB_SPARE1_REG		0xD4090100

#define APPS_PAD_BASE  0xd401e000
#define SMEMC_BASE	   SMC_BASE

// OneNand boots from CS0 only
#define FLEX_BOOTRAM_MAIN_BASE	CS0Base

//
// Multifunction Padring Structures
//
typedef struct
{
  volatile int *    registerAddr;
  unsigned int      regValue;
  unsigned int      defaultValue;
}CS_REGISTER_PAIR_S, *P_CS_REGISTER_PAIR_S;

// Bounds Checking Memory Allocation
typedef struct MemAllocEntry_S
{
	unsigned long StartAddress;
	unsigned long EndAddress;
}MemAllocEntry_T, *pMemAllocEntry_T;

typedef struct MemAllocList_S
{
	unsigned long    MaxEntries;
	unsigned long    ActiveEntries;
    MemAllocEntry_T  MemAllocEntry[K_MAX_MEMORY_BOUNDS_CHECK_ENTRIES];
}MemAllocList_T, *pMemAllocList_T;
//Prototypes
void PlatformI2CClocksEnable(void);
UINT_T PlatformI2CConfig(void);
void ChipSelectSPI(void);
UINT_T ChipSelect2(void);
UINT_T ChipSelect0(void);
void ChipSelectDFC(void);
void RestoreDefaultConfig(void);
void SaveDefaultConfig(void);
UINT_T PlatformUARTConfig(void);

//CI2 USB Functions
void PlatformCI2Init(void);
UINT_T PlatformCI2Transmit(UINT_T, UINT8_T*, UINT_T);
void PlatformCI2Shutdown(void);
void PlatformCI2InterruptHandler(void);
void PlatformCI2InterruptEnable();

// DCB added these to get things compiling
//------------------------------------------
void CheckDefaultClocks(UINT_T *value);
UINT_T PlatformUARTConfig(void);

// MMC
CONTROLLER_TYPE ConfigureMMC(UINT8_T FlashNum, UINT_T *pBaseAddress, UINT_T *pInterruptMask, UINT_T *FusePartitionNumber);
void DisableMMCSlots(void);
UINT_T MMCHighSpeedTimingEnabled();

//NAND
void PlatformNandClocksEnable();
void PlatformNandClocksDisable();
UINT_T GetUSBIDFuseBits(unsigned short* VID, unsigned short* PID );

#if !BOOTROM
// BootLoader Platform Specific Function Prototypes
void FuseOverwriteForDownload(pFUSE_SET pFuses);
#endif

#if MIFI3
void PlatformUSBChargeConfig(void);
#endif

#if UPDATE_USE_GPIO
void PlatformGPIOConfig(void);
UINT_T CheckGPIO(void);
#endif

#if ICASE
void PlatformUSBLimitSet();
#endif

#if I2C
UINT_T PlatformChargerConfig(void);
UINT_T PlatformPI2CConfig(void);
#endif

#if LED_DISPLAY
UINT_T PlatformLEDConfig(void);
#endif

#if OLED_SUPPORT
UINT_T PlatformOLEDConfig(void);
#endif

//external prototypes

extern UINT_T GetOSCR0(void);
extern UINT_T OSCR0IntervalInSec(UINT_T Before, UINT_T After);
extern UINT_T OSCR0IntervalInMilli(UINT_T Before, UINT_T After);
extern UINT_T OSCR0IntervalInMicro(UINT_T Before, UINT_T After);

#endif
