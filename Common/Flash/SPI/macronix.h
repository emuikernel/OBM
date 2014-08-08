/******************************************************************************
 *
 *  (C)Copyright 2013 Marvell Hefei Branch. All Rights Reserved.
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

#ifndef __MACRONIX_H__
#define __MACRONIX_H__

#include "Typedef.h"
#include "predefines.h"
#include "spi.h"
#include "Errors.h"
#include "PlatformConfig.h"
#include "xllp_dmac.h"

// Header file for Macronix MX25U25635F

//
// status register
//
// 7      6      5      4      3      2      1		0
// +------+------+------+------+------+------+------+------+
// | SRWD |  QE  |  BP3 | BP2  | BP1  |  BP0 | WEL  | WIP  |
// +------+------+------+------+------+------+------+------+

#define MX_STATUS_WIP		BIT_0	// write in process
#define MX_STATUS_WEL		BIT_1	// write enable latch
#define MX_STATUS_BP0		BIT_2	// level of protected blcok
#define MX_STATUS_BP1		BIT_3	// level of protected blcok
#define MX_STATUS_BP2		BIT_4	// level of protected blcok
#define MX_STATUS_BP3		BIT_5	// level of protected blcok
#define MX_STATUS_QE		BIT_6	// Quad enable
#define MX_STATUS_SRWD		BIT_7	// status register write protect

// 
// OBM needs SPI Nor to protect blocks 0, 1
//
// BP3		BP2		BP1		BP0		(T/B bit = 1)
// -----+--------+-------+-------+------------------------------------------
//	0		0		1		0		2 (2 blocks, protected block 0th~1th)

//
// configuration register
//
// 7      6      5      4      3      2      1		0
// +------+------+------+------+------+------+------+------+
// | DC1  |  DC0 |4BYTE | Rerd | TB   | ODS2 | ODS1 | ODS0 |
// +------+------+------+------+------+------+------+------+

#define MX_CONFIG_ODS0		BIT_0	// output driver strength
#define MX_CONFIG_ODS1		BIT_1	// output driver strength
#define MX_CONFIG_ODS2		BIT_2	// output driver strength
#define MX_CONFIG_TB		BIT_3	// top/bottom selected, 0=Top area protect; 1=Bottom area protect
#define MX_CONFIG_Rerd		BIT_4	// reserved
#define MX_CONFIG_4BYTE		BIT_5	// 0=3-byte address mode; 1=4-byte address mode
#define MX_CONFIG_DC0		BIT_6	// dummy cycle 0
#define MX_CONFIG_DC1		BIT_7	// dummy cycle 1

//
// security register
//
// 7      6      5      4      3      2      1		0
// +------+------+------+------+------+------+------+------+
// | Rerd |E_FAIL|P_FAIL| Rerd | ESB  | PSB  | LDSO | SOTP |
// +------+------+------+------+------+------+------+------+

#define MX_CONFIG_SOTP		BIT_0	// Secured OTP indicator bit
#define MX_CONFIG_LDSO		BIT_1	// indicate if lock-down
#define MX_CONFIG_PSB		BIT_2	// program suspend bit
#define MX_CONFIG_ESB		BIT_3	// erase suspend bit
#define MX_CONFIG_Rerd		BIT_4	// reserved
#define MX_CONFIG_PFAIL		BIT_5	// 0=normal Program succeed; 1=indicate Program failed
#define MX_CONFIG_EFAIL		BIT_6	// 0=normal Erase succeed; 1=indicate Erase failed
#define MX_CONFIG_Rerd2		BIT_7	// reserved

// commands
#define MX_SPI_CMD_READ_STATUS_REG		0x05
#define MX_SPI_CMD_READ_CONFIG_REG		0x15
#define MX_SPI_CMD_WRITE_STATUS_REG		0x01
#define MX_SPI_CMD_WRITE_SECURITY_REG	0x2F
#define MX_SPI_CMD_READ_SECURITY_REG	0x2B

#define MX_SPI_CMD_PAGE_PROGRAM			0x12
#define MX_SPI_CMD_READ					0x13
#define MX_SPI_CMD_SECTOR_ERASE			0xdc

typedef enum
{
	MX_Status_Register		= 0,
	MX_Config_Register		= 1,
	MX_Security_Register	= 2,
	
	MX_Max_Register 		= 0xff
}MX_Register;

// functions
void MX_SPINOR_Protect_Blocks(void);
void MX_SPINOR_UnProtectBlocks(void);
UINT_T MX_SPINOR_CheckStatus(void);
UINT_T MX_SPINOR_Read(UINT_T FlashOffset, UINT_T Buffer, UINT_T Size);
UINT_T MX_SPINOR_Write(UINT_T Address, UINT_T Buffer, UINT_T Size);
UINT_T MX_SPINOR_Erase(UINT_T Address, UINT_T Size);
#endif

