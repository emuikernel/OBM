/******************************************************************************
 *
 *  (C)Copyright 2014 Marvell Hefei Branch. All Rights Reserved.
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

#ifndef __GIGA_H__
#define __GIGA_H__

#include "Typedef.h"
#include "predefines.h"
#include "spi.h"
#include "Errors.h"
#include "PlatformConfig.h"
#include "xllp_dmac.h"

// Header file for GigaDevice GD25LA256C

//
// status register
//
// 7      6      5      4      3      2      1		0
// +------+------+------+------+------+------+------+------+
// | SRP0 |  QE  |  BP3 | BP2  | BP1  |  BP0 | WEL  | WIP  |
// +------+------+------+------+------+------+------+------+

#define GIGA_STATUS_WIP		BIT_0	// write in process
#define GIGA_STATUS_WEL		BIT_1	// write enable latch
#define GIGA_STATUS_BP0		BIT_2	// level of protected blcok
#define GIGA_STATUS_BP1		BIT_3	// level of protected blcok
#define GIGA_STATUS_BP2		BIT_4	// level of protected blcok
#define GIGA_STATUS_BP3		BIT_5	// level of protected blcok
#define GIGA_STATUS_BP4		BIT_6	// level of protected blcok
#define GIGA_STATUS_SRP0	BIT_7	// status register protect 0

//
// status register 1
//
// 7      6      5      4      3      2      1		0
// +------+------+------+------+------+------+------+------+
// | SUS1 | CMP  |  LP3 | LB2  | EN4B | SUS2 |  QE  | SRP1 |
// +------+------+------+------+------+------+------+------+

#define GIGA_STATUS_SRP1	BIT_0	// status register protect 0
#define GIGA_STATUS_QE		BIT_1	// Quad enable
#define GIGA_STATUS_SUS2	BIT_2	// read only for program suspend
#define GIGA_STATUS_EN4B	BIT_3	// 4Bytes Mode enabled
#define GIGA_STATUS_LB2		BIT_4	// OTP bits for security register
#define GIGA_STATUS_LB3		BIT_5	// OTP bits for security register
#define GIGA_STATUS_CMP		BIT_6	// block protect
#define GIGA_STATUS_SUS1	BIT_7	// read only for erase suspend


// 
// OBM needs SPI Nor to protect blocks 0, 1, ...
//
// BP4		BP3		BP2		BP1		BP0		(CMP bit = 0)
// -----+--------+-------+-------+-------+------------------------------------
//	0		1		0		0		1 		(8 blocks, protected block 0th~7th)


// commands
#define GIGA_SPI_CMD_WRITE_STATUS_REG	0x01
#define GIGA_SPI_CMD_ENABLE_4BYTE_MODE	0xB7
#define GIGA_SPI_CMD_DISABLE_4BYTE_MODE	0xE9
#define GIGA_SPI_CMD_READ_STATUS		0x05
#define GIGA_SPI_CMD_READ_STATUS1		0x35
#define GIGA_SPI_CMD_READ				0x03
#define GIGA_SPI_CMD_PROGRAM			0x02
#define GIGA_SPI_CMD_SECTOR_ERASE		0xd8

typedef enum
{
	Giga_Status_Register		= 0,
	Giga_Status_Register1		= 1,
	
	Giga_Max_Register 		= 0xff
}Giga_Register;

// functions
void Giga_SPINor_Protect_Blocks(void);
void Giga_SPINor_UnProtectBlocks(void);
UINT_T Giga_SPINor_Read(UINT_T FlashOffset, UINT_T Buffer, UINT_T Size);
UINT_T Giga_SPINor_Write(UINT_T Address, UINT_T Buffer, UINT_T Size);
UINT_T Giga_SPINor_Erase(UINT_T Address, UINT_T Size);

#endif

