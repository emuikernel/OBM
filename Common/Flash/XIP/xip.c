/******************************************************************************
**
** INTEL CONFIDENTIAL
** Copyright 2003-2004 Intel Corporation All Rights Reserved.
**
** The source code contained or described herein and all documents
** related to the source code (Material) are owned by Intel Corporation
** or its suppliers or licensors.  Title to the Material remains with
** Intel Corporation or its suppliers and licensors. The Material contains
** trade secrets and proprietary and confidential information of Intel
** or its suppliers and licensors. The Material is protected by worldwide
** copyright and trade secret laws and treaty provisions. No part of the
** Material may be used, copied, reproduced, modified, published, uploaded,
** posted, transmitted, distributed, or disclosed in any way without Intel's
** prior express written permission.
**
** No license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise. Any license under such intellectual property rights
** must be express and approved by Intel in writing.
**
**	 xip.c
******************************************************************************//******************************************************************************
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
**  FILENAME:	Flash.c
**
**  PURPOSE: 	Contain template OEM boot code flash operations
**
******************************************************************************/

#include "general.h"
#include "xip.h"
#include "dma.h"
#include "Errors.h"
#include "Flash.h"
#include "PlatformConfig.h"
#include "loadoffsets.h"
//#include "misc.h"

static XIP_Properties_T XIP_Prop;	// Only need one

P_XIP_Properties_T GetXIPProperties(void)
{
	return &XIP_Prop;
}

// XIP Command table
const XIP_COMMANDS_T XIP_commands[NUM_XIP_DEVICES_SUPPORTED] =
{
// Device	Command	Read	Read	Read	Read	Clear	Program	Program	Prg Buf	Blk Ers	Blk Ers	Blk Lck	Block	Block	Prg Protection
//			Set ID	Array	ID		CFI		Status	Status	Word	Buffer	Confirm	Setup	Confirm	Setup	Lock	Unlock	Register
{/* TYAX */	0x0001, 0x00FF,	0x0090, 0x0098, 0x0070, 0x0050, 0x0040, 0x00E8, 0x00D0, 0x0020, 0x00D0, 0x0060, 0x0001, 0x00D0, 0x00C0},
{/*Sibley*/	0x0200, 0x00FF,	0x0090, 0x0098, 0x0070, 0x0050, 0x0041, 0x00E9, 0x00D0, 0x0020, 0x00D0, 0x0060, 0x0001, 0x00D0, 0x00C0}
};

/*
 *	-- InitializeXIPDevice --
 *
 *	This routine configures the appropriate Chip Select
 *	and fills out the FlashProperties structure with
 *	the relavant information for this XIP device
 */

UINT_T InitializeXIPDevice(UINT8_T FlashNum, UINT8_T* P_DefaultPartitionNum)
{
	UINT_T Retval = NoError;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	P_FlashProperties_T pFlashP = GetFlashProperties();
#if 0 //debug code for dumping SMEMC registers
	UINT8_T Msg[9];
	UINT_T MsgInt, i;
	UINT_T smemc[11]={0x20, 0x30, 0x68, 0x90, 0xB0, 0xB4, 0xC0, 0xD0, 0xE0, 0xF0, 0x100};
#endif

	if(FlashNum == 0x3)
	{
		ChipSelect0(); //part of configure
		pXIP_Prop->BaseAddr = CS0Base;
		pFlashP->TimFlashAddress = TIMOffset_CS0;
		pFlashP->FlashType = CS0_XIP_FLASH;
	}
	else
	{
		ChipSelect2();
		pXIP_Prop->BaseAddr = CS2Base;
		pFlashP->TimFlashAddress = TIMOffset_CS2;
		pFlashP->FlashType = CS2_XIP_FLASH;
	}

#if !BOOTROM
	Retval = CFIQuery();
	if(Retval != NoError)
	{
#if 0 // debug code to dump SMEMC reigisters
		for (i = 0; i < 11; i++)
		{
			MsgInt = *(UINT_T *)(smemc[i] + SMC_BASE);
			ConvertIntToBuf8(Msg, MsgInt, 8,16);
			Msg[8] = NULL;
			AddMessage(Msg);
		}
		while (TRUE){};
#endif
		return Retval;
	}
#endif

	//define functions
	pFlashP->ReadFromFlash = &ReadFromXIP;
	pFlashP->WriteToFlash = &WriteToXIP;
	pFlashP->EraseFlash =  &Erase_XIP;
	pFlashP->LockOTP = &LockXIP_OTP;
	pFlashP->WriteOTP = &WriteToXIP_OTP;
	pFlashP->ReadOTP = &ReadXIP_OTP;
	pFlashP->StreamingFlash = FALSE;
	pFlashP->FinalizeFlash = NULL;
	pFlashP->GenerateFBBT = NULL;
	pFlashP->ValidateFlashPartitions = NULL;
	pFlashP->CreateFlashPartitions = NULL;

	//set the min erase size to the largest block size
#if BOOTROM
	pFlashP->BlockSize = MIN_PAGE_SIZE;
#else
//	pFlashP->BlockSize = pXIP_Prop->XIPEraseRegions[pXIP_Prop->NumEraseRegions-1].block_size;
	pFlashP->BlockSize = pXIP_Prop->XIPEraseRegions[0].block_size;
#endif
	pFlashP->PageSize = MIN_PAGE_SIZE; // **Note** XIP does not have pages, so set at a comman value

	*P_DefaultPartitionNum = XIP_DEFAULT_PART;
	return Retval;
}

UINT_T CFIQuery()
{
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	UINT_T	qtaddr, i;
	UINT_T base = pXIP_Prop->BaseAddr;
	UINT16_T val1, val2, value;
	UINT_T tmp;

	i = 0;

	do{
		*(UINT16_T*)base = READQUERYCMD;//put device in CFI Query mode
		value = *(UINT16_T*)(base | CFI_Q);
		if(value == 0x51)	//if we find 'Q', we are in correct mode
			break;
		if(i++>3)	//try 3 times to put flash into Query mode
			return 1;

	}while(TRUE);


	val1 = *(UINT16_T*)(base | CFI_VID_lower);//lower half of VID
	val2 = *(UINT16_T*)(base | CFI_VID_upper);//upper half of VID
	value = val1 | (val2 << 8);//put full VID together

	//now link in appropriate command set based on VID
	pXIP_Prop->pXIPcmds = NULL;
	for(i=0;i<NUM_XIP_DEVICES_SUPPORTED;i++)
	{
		if(XIP_commands[i].CommandSetID == value)
		{
			pXIP_Prop->pXIPcmds = (P_XIP_COMMANDS_T)&(XIP_commands[i]);
			break;
		}
	}
	if(pXIP_Prop->pXIPcmds == NULL)
		return 2;

	//get the write buffer size
	val1 = *(UINT16_T*)(base | CFI_WRITE_BUFFER_lower);
	val2 = *(UINT16_T*)(base | CFI_WRITE_BUFFER_upper);
	pXIP_Prop->WriteBufferSize = 1 << (val1 | (val2 << 8));

	//grab erase region data
	pXIP_Prop->NumEraseRegions = *(UINT16_T*)(base | CFI_ERASE_REGIONS);;
	tmp = 0;
	for(i=0;i<pXIP_Prop->NumEraseRegions;i++)
	{
		pXIP_Prop->XIPEraseRegions[i].region_offset = tmp;
		val1 = *(UINT16_T*)(base | (CFI_ERASE_REGIONS + CFI_ERASE_BLK_lower + i*8));
		val2 = *(UINT16_T*)(base | (CFI_ERASE_REGIONS + CFI_ERASE_BLK_upper + i*8));
		pXIP_Prop->XIPEraseRegions[i].num_blocks = (val1 | (val2 << 8)) + 1;
		val1 = *(UINT16_T*)(base | (CFI_ERASE_REGIONS + CFI_ERASE_SZ_lower + i*8));
		val2 = *(UINT16_T*)(base | (CFI_ERASE_REGIONS + CFI_ERASE_SZ_upper + i*8));
		pXIP_Prop->XIPEraseRegions[i].block_size = (val1 | (val2 << 8)) << 8;
		pXIP_Prop->XIPEraseRegions[i].region_size = pXIP_Prop->XIPEraseRegions[i].block_size * pXIP_Prop->XIPEraseRegions[i].num_blocks;
		tmp += pXIP_Prop->XIPEraseRegions[i].region_size;
	}

	//now get Intel specific query table address
	val1 = *(UINT16_T*)(base | CFI_QTTable_lower);
	val2 = *(UINT16_T*)(base | CFI_QTTable_upper);

	qtaddr = (val1 | (val2 << 8)) << 1;
	qtaddr |= base;
	//use query table to fill in protection reg informartion
	value = *(UINT16_T*)(qtaddr + CFI_PROTECTION_REGION);
	if(value != 2)
		return 3;
	//protection field 1 info
	val1 = *(UINT16_T*)(qtaddr + CFI_PROT1_ADDR_lower);
	val2 = *(UINT16_T*)(qtaddr + CFI_PROT1_ADDR_upper);
	value = (val1 | (val2 << 8)) << 1;
	pXIP_Prop->XIPProtLockRegisters[0].lock_reg_addr = value;
	val1 = *(UINT16_T*)(qtaddr + CFI_PROT1_UNWRITABLE_SZ);
	val2 = 1 << val1;
	pXIP_Prop->XIPProtLockRegisters[0].prot_reg_addr = value + 2 + val2;
	val2 = *(UINT16_T*)(qtaddr + CFI_PROT1_WRITABLE_SZ);
	pXIP_Prop->XIPProtLockRegisters[0].prot_reg_size = 1 << val2;
	pXIP_Prop->XIPProtLockRegisters[0].num_prot_regs = 1;

	//protection field 2 info
	value = 0;
	for(i=0;i<4;i++)
	{
		val1 = *(UINT16_T*)(qtaddr + CFI_PROT2_ADDR_b0 + (i << 1));
		value |= val1 << (i << 3);
	}
	value <<= 1;
	pXIP_Prop->XIPProtLockRegisters[1].lock_reg_addr = value;
	pXIP_Prop->XIPProtLockRegisters[1].prot_reg_addr = value + 2;
	val1 = *(UINT16_T*)(qtaddr + CFI_PROT2_TOTAL_lower);
	val2 = *(UINT16_T*)(qtaddr + CFI_PROT2_TOTAL_upper);
	value = val1 | (val2 << 8);
	pXIP_Prop->XIPProtLockRegisters[1].num_prot_regs = value;
	value = *(UINT16_T*)(qtaddr + CFI_PROT2_WRITABLE_SZ);
	pXIP_Prop->XIPProtLockRegisters[1].prot_reg_size = 1 << value;

	//restore flash to read array mode
	*(UINT16_T*)base = pXIP_Prop->pXIPcmds->ReadArrayCmd;
	return NoError;
}

/*
 *	-- ReadFromXIP --
 *
 *	Read flash routine for XIP that calls into the DMA
 *  directory to move data by a MEM2MEM call
 */

UINT_T ReadFromXIP(UINT_T offset, UINT_T dest, UINT_T size)
{
	UINT_T Retval = NoError;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	UINT_T flash_addr = offset + pXIP_Prop->BaseAddr;

	//if the load address equals the flash start address no need to copy run from XIP
	if(flash_addr == dest)
		return NoError;

#if !BOOTROM
	//make sure flash is in read array mode
	*(UINT16_T*)flash_addr = pXIP_Prop->pXIPcmds->ReadArrayCmd;
#endif

	memcpy((UINT8_T *)dest, (UINT8_T *)flash_addr, size);
	return Retval;
#if 0 // we will not use DMA on the test chip
	  // will be added back in for the next stepping
	return Mem2Mem(flash_addr, dest, size);
#endif
}

/*
 *	-- WriteToXIP --
 *
 *	Write flash routine for XIP that writes
 *	to the XIP device in Buffered Programming mode
 *
 */

UINT_T WriteToXIP(UINT_T offset, UINT_T source, UINT_T size)
{
	UINT_T Retval = NoError;
	UINT_T max_write, addr;
	UINT_T count = 0;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();

	addr = pXIP_Prop->BaseAddr + offset;

	//Flash is written in Object Mode
	max_write = pXIP_Prop->WriteBufferSize;

	while(size > 0){
		if(max_write > size)
			max_write = size;

		Retval = ProgramXIPBlock(addr, source, max_write);

		if(Retval != NoError)
			break;

		addr += max_write;
		source += max_write;
		size -= max_write;
	}

	return Retval;
}

/*
 *	-- Erase_XIP --
 *
 *	Erase flash routine for XIP that erases 1 full block at a time
 *		note:  if user inputs determine that a partial block should be erased,
 *				this function will erase that WHOLE block: no partial blocks allowed
 */

UINT_T Erase_XIP (UINT_T flashoffset, UINT_T size)
{
	UINT_T Retval;//status flag
	UINT_T region_start, blk_sz, offset_in_reg, blk_st_addr, partial_size;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	P_XIP_ERASE_BLOCK_T pXIP_EraseBlk = pXIP_Prop->XIPEraseRegions;

	Retval = NoError;
	do{
		region_start = FindEraseRegion(flashoffset);
		blk_sz = pXIP_Prop->XIPEraseRegions[region_start].block_size;
		offset_in_reg = flashoffset - pXIP_Prop->XIPEraseRegions[region_start].region_offset;

		//partial block check - if the offset is not on a block boundary, move it to one (erase whole block)
		partial_size = offset_in_reg % blk_sz;
		//partial_size will be 0 if we are on a block boundary
		blk_st_addr = offset_in_reg - partial_size;
		size += partial_size;

		Retval = EraseBlock(blk_st_addr);
		if (Retval != NoError)
			break;

		size = size - blk_sz + partial_size;
		flashoffset = flashoffset - blk_sz + partial_size;

	}while(size > 0);

	return Retval;
 }

/*
	Helper Function to find which erase region you're in based on the offset
*/
UINT_T FindEraseRegion(UINT_T offset)
{
	UINT_T region_sz, i;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	P_XIP_ERASE_BLOCK_T pXIP_EraseBlk = pXIP_Prop->XIPEraseRegions;

	for(i=0; i < pXIP_Prop->NumEraseRegions; i++)
	{
		region_sz = pXIP_Prop->XIPEraseRegions[i].region_size;
		if(offset < region_sz)
			return i;
		offset -= region_sz;
	}
	return 0;
}
/*
	Grunt routine that erases a block
	Parameters:
		flashoffset - must give a block aligned offset in flash
*/
UINT_T EraseBlock(UINT_T flashoffset)
{
	UINT_T Retval = NoError;
	UINT16_T ProgramStatus;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	P_XIP_COMMANDS_T pcmds = pXIP_Prop->pXIPcmds;
	UINT_T Addr = flashoffset + pXIP_Prop->BaseAddr;

	// Issue clear status command to clear status register
	*(UINT16_T *)Addr = pcmds->ClearStatusRegisterCmd;

	// Issue block unlock setup command.  Done to unlock block, just in case....
	*(UINT16_T *)Addr = pcmds->LockBlockSetupCmd;
	*(UINT16_T *)Addr = pcmds->UnlockBlockCmd; // Issue unlock block confirm

	//Issue Block Erase
	*(UINT16_T *)Addr = pcmds->BlockEraseSetupCmd;

	//Issue Block Erase Confirm
	*(UINT16_T *)Addr = pcmds->BlockEraseConfirmCmd;

	// Read status Register
	do
	{
		ProgramStatus   = *(UINT16_T *)Addr;
	} while ((ProgramStatus & DeviceReadyStatus_16) != DeviceReadyStatus_16);

	// Issue clear status command to clear status register
	*(UINT16_T *)Addr = pcmds->ClearStatusRegisterCmd;
	// check for program error
	if ((ProgramStatus & EraseErrorStatus_16) == EraseErrorStatus_16)
	{
		Retval = EraseError;
	}
	// Put block back in Read-Array Mode
	*(UINT16_T *)Addr = pcmds->ReadArrayCmd; // Restore read array mode

	return Retval;
}


/*
 *	XIP OTP Write command
 *		offset - number of bytes from beginning of OTP are to start writing data
 *		data - pointer to data to be written
 *		size - number of bytes of 'data' to be written
 */
UINT_T WriteToXIP_OTP(UINT_T offset, UINT_T data, UINT_T size)
{
	UINT_T Retval = NoError;
	UINT_T wrt_amt, addr, avail;
	volatile UINT16_T* otp_addr;
	UINT16_T* word;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	P_XIP_COMMANDS_T pcmds = pXIP_Prop->pXIPcmds;

	word = (UINT16_T *)data;
	//program 1 16-bit word at a time
	while(size > 0)
	{
		Retval =  FindOTPField(offset, &addr, &avail);
		if(Retval != NoError)
			break;

		wrt_amt = size > avail ? avail : size;
		size -= wrt_amt;
		offset += wrt_amt;
		otp_addr = (UINT16_T *)(pXIP_Prop->BaseAddr | addr);

		// Issue clear status command to clear status register
    	*otp_addr = pcmds->ClearStatusRegisterCmd;
		*otp_addr = pcmds->ProgramProtectionRegCmd;  // Issue protection program command
		do{

			*otp_addr = *word++;
	    	do
    		{
       	 		Retval = *otp_addr;
    		} while ((Retval & DeviceReadyStatus_16) != DeviceReadyStatus_16);

			Retval = NoError;
			otp_addr += 2;
			wrt_amt -= 2; //write 16 bit at a time

		}while(wrt_amt > 0);
		*otp_addr = pcmds->ReadArrayCmd;
	}

	return Retval;

}

/*
 *	XIP OTP Read command
 *		offset - number of bytes from beginning of OTP area to start reading data
 *		buffer - pointer to data to be returned
 *		size - number of bytes to be returned
 */
UINT_T ReadXIP_OTP(UINT_T offset, UINT_T data, UINT_T size)
{
	UINT_T Retval = NoError;
	UINT_T rd_amt, addr, avail;
	volatile UINT16_T* otp_addr;
	UINT16_T* word;

	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	P_XIP_COMMANDS_T pcmds = pXIP_Prop->pXIPcmds;

	word = (UINT16_T *)data;
	otp_addr = (UINT16_T *)pXIP_Prop->BaseAddr;
	// Issue clear status command to clear status register
	*otp_addr = pcmds->ClearStatusRegisterCmd;
	*otp_addr = pcmds->ReadIdentifierCmd;  // Issue read ID command

	while(size > 0)
	{
		Retval =  FindOTPField(offset, &addr, &avail);
		if(Retval != NoError)
			break;

		rd_amt = size > avail ? avail : size;
		size -= rd_amt;
		offset += rd_amt;
		otp_addr = (UINT16_T *)(pXIP_Prop->BaseAddr | addr);

		do{
			*word++ = *otp_addr++;
			rd_amt -= 2; //reading 16 bit at a time
		}while(rd_amt > 0);
	}
	//restore flash mode
	*otp_addr = pcmds->ReadArrayCmd;

	return Retval;
}


/*
 *	XIP OTP Lock command
 *		This method locks the 3 registers that contain JTAG and OEM key hash data
 */
UINT_T LockXIP_OTP(void)
{
	UINT_T Retval;
	volatile UINT16_T *addr;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	P_XIP_COMMANDS_T pcmds = pXIP_Prop->pXIPcmds;

	addr = (UINT16_T*) pXIP_Prop->XIPProtLockRegisters[0].lock_reg_addr;

	*addr = pcmds->ClearStatusRegisterCmd;
	*addr = pcmds->ProgramProtectionRegCmd;
	*addr = 0x2;//write bit 1, which will lock OTP 0
	do{
		Retval = *(volatile UINT16_T *)pXIP_Prop->BaseAddr;
	} while ((Retval & DeviceReadyStatus_16) != DeviceReadyStatus_16);

	addr = (UINT16_T*) pXIP_Prop->XIPProtLockRegisters[1].lock_reg_addr;

	*addr = pcmds->ReadArrayCmd;
	*addr = pcmds->ClearStatusRegisterCmd;
	*addr = pcmds->ProgramProtectionRegCmd;
	*addr = 0x3;//write bit 0 and bit 1: locks OTP 1 and OTP 2
	do{
		//*WordProgramAddr =  ReadStatusRegisterCmd; // Issue read status command
		Retval = *(volatile UINT16_T *)pXIP_Prop->BaseAddr;
	} while ((Retval & DeviceReadyStatus_16) != DeviceReadyStatus_16);

	*addr = pcmds->ReadArrayCmd;  	// Restore read array mode

	return NoError;
}

/*
	Helper Function to find which OTP field you're in based on the offset
*/
UINT_T FindOTPField(UINT_T offset, UINT_T *addr, UINT_T *amt_avail)
{
	UINT_T i, field_sz;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();

	for(i=0; i < MAX_PROT_LOCK_REG_FIELDS; i++)
	{
		field_sz = pXIP_Prop->XIPProtLockRegisters[i].prot_reg_size *
					pXIP_Prop->XIPProtLockRegisters[i].num_prot_regs;
		if(offset < field_sz){
			*addr = pXIP_Prop->XIPProtLockRegisters[i].prot_reg_addr + offset;
			*amt_avail = field_sz - offset;
			break;
		}
		offset -= field_sz;
	}
	if(i == MAX_PROT_LOCK_REG_FIELDS)
		return OTPError;
	return NoError;
}


UINT_T ProgramXIPBlock(UINT_T location, UINT_T source, UINT_T size)
{
	UINT_T Retval = NoError;
	volatile UINT16_T ProgramStatus;
	UINT16_T* word;
	UINT16_T* addr2;
	volatile UINT16_T *addr;
	P_XIP_Properties_T pXIP_Prop = GetXIPProperties();
	P_XIP_COMMANDS_T pcmds = pXIP_Prop->pXIPcmds;
	UINT_T padding = 0;

	word = (UINT16_T*) source;
	addr = (volatile UINT16_T*) location;
	if ( size < pXIP_Prop->WriteBufferSize)
			padding = pXIP_Prop->WriteBufferSize -size;

	// Issue clear status command to clear status register
	*addr = pcmds->ClearStatusRegisterCmd;

	// Issue block unlock setup command.  Done to unlock block, just in case....
	*addr = pcmds->LockBlockSetupCmd;
	*addr = pcmds->UnlockBlockCmd; // Issue unlock block confirm

	// Issue Buffer Program Command
	*addr = pcmds->BufferedProgramCmd;
	ProgramStatus = *addr;
	// Read status Register
	while ((ProgramStatus & DeviceReadyStatus_16) != DeviceReadyStatus_16)
	{
		*addr = pcmds->BufferedProgramCmd;
		ProgramStatus = *addr;
	};

	// Write Nibble Count
	*addr = size+padding-1;
	addr2 = (UINT16_T *)addr;
	// write 16bit words to buffer
	do{
		*addr2++ = *word++;
		size -= 2;
	}while(size > 0);

	while(padding > 0){
		*addr2++ = 0xFFFF;
		padding -= 2;
	}

	// Write Buffer Confirm Cmd
	*addr = pcmds->BufferedProgramConfirmCmd;

	// Read status Register
	do
	{
		ProgramStatus = *addr;
	} while ((ProgramStatus & DeviceReadyStatus_16) != DeviceReadyStatus_16);

	// check for program error
	if ((ProgramStatus & ProgramErrorStatus_16) == ProgramErrorStatus_16)
	{
		Retval = ProgramError;
	}
	// Issue clear status command to clear status register
	*addr = pcmds->ClearStatusRegisterCmd;
	*addr = pcmds->ReadArrayCmd; // Restore read array mode

	return Retval;
}