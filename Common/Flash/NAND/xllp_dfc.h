/******************************************************************************
 *
 *  (C)Copyright 2005 - 2008 Marvell. All Rights Reserved.
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
 *
 *
 *  FILENAME: xllp_dfc.h
 *
 *  PURPOSE:  XLLP interface to the data flash controller (DFC).
 *
******************************************************************************/

#ifndef __XLLP_DFC_H__
#define __XLLP_DFC_H__

#include "Typedef.h"
#include "xllp_dfc_defs.h"
#include "nand.h"

#define DMA_OPERATION_TIMEOUT_MS        10
#define NFU_FIFO_SIZE					2048
#define DFC_SR_WRTOCLR_MASK 			0x001F9FFF		// all of the non-reserved fields above

/* NAND command has four parts: */
typedef struct NAND_CMD_S
{
    NDCB0_REG cb0; 
    NDCB1_REG cb1; 
    NDCB2_REG cb2; 
    NDCB3_REG cb3; 
} NAND_CMD_T, *P_NAND_CMD_T;


///////////////////////////////////////////////////////////////////////////////
/////
///     D F C   I N T E R F A C E   R O U T I N E S
///
///////////////////////////////////////////////////////////////////////////////
UINT_T XllpDfcInit      (UINT_T width, P_NAND_Properties_T pNAND_Prop);
INT xdfc_write      (P_DFC_BUFFER buffer, UINT_T address, UINT_T bytes, UINT_T bUseSpareArea, UINT_T bUseHwEcc, P_NAND_Properties_T pNAND_Prop);
UINT_T xdfc_write_LP   (P_DFC_BUFFER buffer, UINT_T address, UINT_T bytes, UINT_T bUseSA,P_NAND_Properties_T pNAND_Prop);
UINT_T xdfc_read_nonDMA(P_DFC_BUFFER buffer, UINT_T address, UINT_T bytes, UINT_T* SpareArea, P_NAND_Properties_T pNAND_Prop);
UINT_T xdfc_read_LP (P_DFC_BUFFER buffer, unsigned int address, unsigned int bytes, unsigned int* SpareArea, P_NAND_Properties_T pNAND_Prop);
UINT_T xdfc_erase      (UINT_T address, P_NAND_Properties_T pNAND_Prop);
// for FBF downloading
void xdfc_enable_ecc( unsigned int bEnable );
UINT_T xdfc_read(unsigned int  buffer, 
                 unsigned int  address, 
                 unsigned int  bytes, 
                 unsigned int* SpareArea, 
                 P_NAND_Properties_T pNAND_Prop);

//workhorses
UINT_T xdfc_reset      (P_NAND_Properties_T pNAND_Prop);
UINT_T xdfc_readstatus (P_DFC_BUFFER buffer, P_NAND_Properties_T pNAND_Prop);
UINT_T xdfc_readID     (P_DFC_BUFFER buffer, DEVICE_TYPE dev_type);
void xdfc_setTiming (FLASH_TIMING* flash);
void Delay          (UINT_T);
UINT_T xdfc_getStatusPadding(P_NAND_Properties_T pNAND_Prop);
UINT_T xdfc_getSpareArea_LP(P_NAND_Properties_T pNAND_Prop);
UINT_T  xdfc_getECCArea_LP(P_NAND_Properties_T pNAND_Prop);


// helper routines
DEVICE_TYPE XllpCheckDeviceType( P_DFC_BUFFER buffer, P_NAND_Properties_T pNAND_Prop);
void XllpLoadProperties(P_DFC_BUFFER buffer, P_NAND_Properties_T pNAND_Prop);
void getAddr(UINT_T address, NDCB1_REG * addr1234, NDCB2_REG * addr5, P_NAND_Properties_T pNAND_Prop);
void getEraseAddr(UINT_T address, NDCB1_REG * addr1234, NDCB2_REG * addr5, P_NAND_Properties_T pNAND_Prop);
UINT_T _WaitForDFCOperationComplete(UINT_T statusMask, UINT32 TimeOutMillisec);
UINT_T _WaitForDFCOperationPulldown(UINT_T statusMask, UINT32 TimeOutMicrosec);
unsigned int xdfc_stripPad(unsigned int upper, unsigned int lower);
void XllpDfcDeviceSpecificInit(P_NAND_Properties_T pNAND_Prop);

/********************* Internal Debug macro definitions ******************************/
// DFC Register access macros

#define DFC_REG_WRITE(regAddress, wval) \
        ( (* ( (volatile UINT32*)(regAddress) ) ) = wval);

#define DFC_REG_READ(regAddress, rval) \
        rval = (* ( (volatile UINT32*)(regAddress)));



#endif
