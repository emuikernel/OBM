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
 ******************************************************************************/


#include "Flash.h"
#include "general.h"
#include "Typedef.h"
#include "misc.h"
#include "Errors.h"
#include "FM.h"
#include "xllp_dfc.h"


/****************   Steps to add a new Flash Device **********************
 *
 * 1. add #include 'file.h' below
 * 2. add pointer to the initialize function of the new flash device
 *        in the InitRoutines function pointer array at the correct
 *        location - should aligned with Fuse Config number for that flash
 *
 *************************************************************************/

//flash device includes
#include "xip.h"
#include "nand.h"
#include "sdmmc_api.h"
#include "PlatformConfig.h"
#include "spi.h"
#include "downloader.h"

/*static*/ FlashProperties_T FlashProp[2];  // 0 - Boot Flash, 1- Caddo Save State Flash
static FlashBootType_T CurrentFlashBootType;
extern SkipBlocksInfoStruct *pSkipAddress;
extern UINT_T upload_nand_spare;

P_FlashProperties_T GetFlashProperties(FlashBootType_T FlashBootType)
{
    if (FlashBootType == BOOT_FLASH)
        return &FlashProp[0];
    else
        return &FlashProp[1];
}

FlashBootType_T GetCurrentFlashBootType()
{
    return CurrentFlashBootType;
}

void SetCurrentFlashBootType(FlashBootType_T FlashBootType)
{
     CurrentFlashBootType = FlashBootType;
     return;
}

InitFlash_F GetInitRoutine(UINT8_T FlashNum){

	InitFlash_F Routine = NULL;
	switch(FlashNum){
		case NAND_FLASH_X16_HM_P:
		case NAND_FLASH_X8_HM_P:
		case NAND_FLASH_X16_BCH_P:
		case NAND_FLASH_X8_BCH_P:
		case NAND_FLASH_X16_BCH_OPT1_P:
		case NAND_FLASH_X8_BCH_OPT1_P:
			#if NAND_CODE
    		Routine = &InitializeNANDDevice;
			#endif
			break;
		case CS0_XIP_FLASH_P:
		case CS2_XIP_SIBLEY_P:
			#if NOR_CODE
		  	Routine = &InitializeXIPDevice;
			#endif
			break;
 		case SDMMC_FLASH_OPT1_P:
		case SDMMC_FLASH_OPT2_P:
		case SDMMC_FLASH_OPT3_P:
		case SDMMC_FLASH_OPT4_P:
        case SDIO_P:
			#if MMC_CODE
		  	Routine = &InitializeSDMMCDevice;
			#endif
			break;
		case SPI_FLASH_P:
			#if SPI_CODE
		  	Routine = &InitializeSPIDevice;
			#endif
			break;
		default:
			break;
		}
	return Routine;

}
//--------------------------------------------------------------------------------------
// Configure_Flashes()
//
// This routine will configure the flashes according what the fuse settings passed in.
//--------------------------------------------------------------------------------------
UINT_T Configure_Flashes (UINT8_T FlashNum, FlashBootType_T fbt)
{
    UINT_T Retval = InvalidBootTypeError;
    UINT_T i;
    InitFlash_F InitializeFunction;
    P_FlashProperties_T pFlashProp, pFlashPropBoot;
    P_FMProperties_T pFMProp = GetFMProperties();
    UINT8_T DefaultPartitionNum;

    // Get a pointer to the FlashProperties structure
    pFlashProp = GetFlashProperties(fbt);
    // Need to force grabbing the BOOT_FLASH struct for reasons found in for loop below
    pFlashPropBoot = GetFlashProperties(BOOT_FLASH);

    // Check, Are we initializing the SaveStateFlash and has the BootFlash already been initialized?
    // And are they the save flash device?
    //  If so, no need to init hardware again, just copy FlashProperties
    if ((fbt == SAVE_STATE_FLASH) && (pFlashPropBoot->FlashSettings.FlashInitialized == TRUE))
    {
        if(pFlashPropBoot->FlashSettings.FlashNum == FlashNum)
        {
            for(i = 0; i < (sizeof(FlashProperties_T) / 4); i++)
                ((UINT_T *)pFlashProp)[i] = ((UINT_T *)pFlashPropBoot)[i];
            return NoError;
        }
    }

    // If the flash that is passed in has already been initialized, return no error.
    if ((pFlashProp->FlashSettings.FlashNum == FlashNum) && (pFlashProp->FlashSettings.FlashInitialized == TRUE) )
        return NoError;

    //Clear the Flash properties structure
    // (Null out pointers)
    //-------------------------------------
    memset( pFlashProp, 0, sizeof(FlashProperties_T));

    //See if we get a pointer to the appropriate
    //init function for the assigned flash
    //------------------------------------------
	InitializeFunction = GetInitRoutine(FlashNum);
    if(InitializeFunction == NULL)
        return InvalidBootTypeError;

    Retval = InitializeFunction(FlashNum, fbt, &DefaultPartitionNum);
    if (Retval == NoError)
    {
        pFMProp->PI_num = DefaultPartitionNum;  // The default partition this device sets to.
        pFlashProp->FlashSettings.FlashInitialized = TRUE;
        pFlashProp->FlashSettings.FlashNum = FlashNum;
        //Clear out the FM properties structure, to get rid of stale data
        if(fbt == BOOT_FLASH)
            ClearFM(BOOT_FLASH);
    }

    return Retval;
}

UINT_T Finalize_Flashes(FlashBootType_T fbt)
{
    UINT_T Retval = NoError;
    P_FlashProperties_T pFlashProp = GetFlashProperties(fbt);
    FinalizeFlash_F final = pFlashProp->FinalizeFlash;

    //if the flash is currently not initialized, just return
    if (pFlashProp->FlashSettings.FlashInitialized == FALSE)
        return NoError;

#if COPYIMAGESTOFLASH
    //Finish any Flash Management
    if(fbt == BOOT_FLASH)
        FinalizeFM(fbt);
#endif

    //Shutdown the flash if the appropriate function is linked
    if(final != NULL)
        Retval = final(fbt);

    //Clear out the Properties structure
    //Clear the Flash properties structure
    // (Null out pointers)
    //-------------------------------------
    memset( pFlashProp, 0, sizeof(FlashProperties_T));

    return Retval;
}

#if COPYIMAGESTOFLASH
/* Check if a certain block is in skip list*/
int CheckIfSkip(int blk_num)
{
	int i, temp;

	temp = pSkipAddress->Total_Number_Of_SkipBlocks;
	for (i = 0; i < temp; i++)
		if(blk_num == pSkipAddress->Skip_Blocks[i])
			return TRUE;

	return FALSE;
			
}
#endif

/* ReadFlash
 *  This API function calls the relative Flash device read command to read data.
 *  This function calls the low level read on a block basis, in order to use
 *  upper level FlashManagement
 *
 *  Inputs:
 *      flash_addr - address of where to begin read in flash device ++
 *      buffer - pointer to location of where to load the data to
 *      total_size - amount of data, in bytes, to read
 *      fbt - flash boot type:  BOOT_FLASH or SAVE_STATE_FLASH
 *  Outputs:
 *      None
 *  Retval value:
 *      NoError - on successful completion
 *      <Other> - Flash device specific read error
 *
 *  ++ if a partition table is being used, this address MUST be relative the partition's
 *          start address, and not the entire flash device
 */
UINT_T ReadFlash(UINT_T flash_addr, UINT_T buffer, UINT_T total_size, FlashBootType_T fbt)
{
    UINT_T Retval = NoError, Status = NoError;
    UINT_T BlkNum, BlkSize, BlockMask, PartitionOffset, read_size, read_addr;
    ReadFlash_F read;
    P_FlashProperties_T pFlashP = GetFlashProperties(fbt);

#if NAND_CODE
	P_NAND_Properties_T pNandP = GetNANDProperties();
	UINT_T spare_size = pNandP->SpareAreaSize;
	UINT_T page_per_block = pNandP->PagesPerBlock;
	UINT_T total_spare_size = spare_size * page_per_block;
#endif

    read = pFlashP->ReadFromFlash;

    if(read == NULL)
        return FlashFuncNotDefined;

	if (total_size == 0)
		return NoError;

#if SPI_CODE
	Retval = read(flash_addr, buffer, total_size, fbt);
#else

    if(total_size & 0x3)    //make size 4bytes-align
    	total_size = (total_size+4)&(~3);
	
	//if flash_addr isn't page(2k) align, then return.
	if((flash_addr & (pFlashP->PageSize-1)) )
		return FlashAddrNotChunkAlign;

    //set up initial values
    BlkSize = GetBlockSize(fbt);
    BlockMask = (BlkSize - 1);  // Mask to get offset into a block
    PartitionOffset = GetPartitionOffset(fbt);

    //If the flash uses BBM, read a block at a time
    if(pFlashP->FlashSettings.UseBBM)
    {
        //for the first read, only read to the end of the initial block
        read_size = BlkSize - (BlockMask & flash_addr);

        while (total_size)
        {
            // Find block number
            BlkNum = (flash_addr / BlkSize);
            // Check for relocations
            BlkNum = LocateBlock(BlkNum, fbt);
            // calculate the read address, in case of relocation
            // Read Address = new block * block size + offset into the block
            read_addr = (BlkNum * BlkSize) + (flash_addr & BlockMask);
            // Check read amount
            read_size = read_size > total_size ? total_size : read_size;
            // Read in to the buffer
            Retval = read(read_addr + PartitionOffset, buffer, read_size, fbt);
            // save off any read disturb data
            if (Retval == ReadDisturbError)
                Retval = AddReadDisturbEntry(GetFMProperties()->PI_num, BlkNum);

            if (Retval != NoError)
                break;

            //update counting stats
            // - address pointers
            flash_addr += read_size;
            buffer += read_size;

		#if NAND_CODE
			if (upload_nand_spare == TRUE)
			{
				buffer += total_spare_size;
			}
		#endif
			
            // - size pointers
            total_size -= read_size;
            read_size = BlkSize;
        }   // End of while
        //make sure to return any ReadDisturb that was caught
//      if(Retval == NoError) TBD_TNA - Read disturbs should be saved in another manner.
//          Retval = Status;
    }
    else  //BBM not being used, just read the whole thing
        Retval = read(flash_addr + PartitionOffset, buffer, total_size, fbt);
#endif

    return Retval;
}

/* WriteFlash
 *  This API function calls the relative Flash device write command to program data.
 *  This function calls the low level program on a block basis, in order to use
 *  upper level FlashManagement.
 *
 *  Inputs:
 *      flash_addr - address of where to begin write in flash device
 *      buffer - pointer to location of the data
 *      total_size - amount of data, in bytes, to program
 *      fbt - flash boot type:  BOOT_FLASH or SAVE_STATE_FLASH
 *  Outputs:
 *      None
 *  Retval value:
 *      NoError - on successful completion
 *      <Other> - Flash device specific read error
 */
UINT_T WriteFlash(UINT_T flash_addr, UINT_T buffer, UINT_T total_size, FlashBootType_T fbt)
{
    UINT_T Retval = NoError, sparesize = 0;
#if COPYIMAGESTOFLASH
	UINT_T BlkNum, BlkSize, BlockMask, PartitionOffset, write_size, write_addr, retry;
    P_FlashProperties_T pFlashP = GetFlashProperties(fbt);
    WriteFlash_F write = pFlashP->WriteToFlash;
    UINT_T PageSize, PageMask;
    UINT_T UseSpareFlag;

    UseSpareFlag = GetUseSpareArea(BOOT_FLASH);
    
    //if write is not defined, return with error
    if(write == NULL)
        return FlashFuncNotDefined;

	if (total_size == 0)
		return NoError;

#if SPI_CODE
	Retval = write(flash_addr, buffer, total_size, fbt);
#else

    //set up initial values
    retry = TRUE;
    BlkSize = GetBlockSize(fbt);
    BlockMask = (BlkSize - 1);  // Mask to get offset into a block
    PartitionOffset = GetPartitionOffset(fbt);
	PageSize = pFlashP->PageSize;
	PageMask = (PageSize-1);
    //for the first write, only write to the end of the initial block
    write_size = BlkSize - (BlockMask & flash_addr);
    
    if(total_size & 0x3)    //make size 4bytes-align
    	total_size = (total_size+4)&(~3);

	
	//if flash_addr isn't page(2k) align, then return.
	if((flash_addr & PageMask) )
		return FlashAddrNotChunkAlign;

    
	// if this media has BBM support, then attempt retries on failures:
	if( pFlashP->FlashSettings.UseBBM)
	{
		while (total_size)
		{
			// Find block number
			BlkNum = (flash_addr / BlkSize);

			// add here to judge if this block is in the skip list, if it is in skip list, then continue to next block;
			if(pSkipAddress != NULL)
			{
				if(CheckIfSkip(BlkNum))
					goto NEXT_BLOCK;
			}


			// Check for relocations
			BlkNum = LocateBlock(BlkNum, fbt);
			// calculate the writeaddress, in case of relocation
			// Write Address = new block * block size + offset into the block
			write_addr = (BlkNum * BlkSize) + (flash_addr & BlockMask);

            if( UseSpareFlag)
            {
               sparesize = (write_size >> 5);
               write_size = write_size + sparesize;
            }
			// Check write amount
			write_size = write_size > total_size ? total_size : write_size;
			// Write from the buffer
			Retval = write(write_addr + PartitionOffset, buffer, write_size, fbt);
			//if we hit an error, retry once, then Relocate
			if (Retval != NoError)
			{
				if(retry == TRUE)
				{
					//turn off retry flag
					retry = FALSE;
					//try erasing the block
					Retval = EraseFlash(write_addr + PartitionOffset, write_size, fbt);
					//if the erase succeeded, retry writing same block
					if(Retval == NoError)
						continue;
					//if erase failed, fall through to relocate
				}
				//reset retry flag, since we will be using a different block on the next try
				retry = TRUE;
				//no need to capture the return BlkNum, as the next LocateBlock will catch it
				Retval = RelocateBlock(BlkNum, &BlkNum, fbt);
				if(Retval != NoError)
					return Retval;
				//retry writing, with the updated block
				continue;
			}
			//update counting stats
			// - address pointers
NEXT_BLOCK:
            if(UseSpareFlag)
			   flash_addr += (write_size-sparesize);
            else
			   flash_addr += write_size;
                     
			buffer += write_size;
			// - size pointers
			total_size -= write_size;
			write_size = BlkSize;
			retry = TRUE;
		}   // End of while
	}
	else
	{
	
	#if MMC_CODE
		// DMA max is 16M
		while(total_size)
		{
			if(total_size > 0x1000000)
			{
				Retval = write(flash_addr + PartitionOffset, buffer, 0x1000000, fbt);
				if(Retval != NoError)
					return Retval;

				flash_addr += 0x1000000;
				total_size -= 0x1000000;
				buffer += 0x1000000;
			}
			else
			{
				Retval = write(flash_addr + PartitionOffset, buffer, total_size, fbt);
				total_size = 0;
			}
		}

	#else
		// this media has no BBM support. use call the media-specific write routine.
		Retval = write(flash_addr + PartitionOffset, buffer, total_size, fbt);
	#endif
	}
#endif
#endif
    return Retval;
}

void ResetBBT()
{
	// create a new BBT
	CreateBBT_Legacy(NULL);
}

UINT_T EraseAllFlash(FlashBootType_T fbt)
{
	UINT_T Retval = NoError;
#if (COPYIMAGESTOFLASH)	
	P_FlashProperties_T pFlashP = GetFlashProperties(fbt);
	UINT_T initial_blk, end_blk;

	#if SPI_CODE
	Retval = SPINOR_Wipe();
	#else

    initial_blk = pFlashP->NumBlocks - 1;
	end_blk = initial_blk - (pFlashP->NumBlocks / 50); //2% of whole device
	// erase all blocks except reserved area
	Retval = EraseFlash(0, pFlashP->BlockSize*(end_blk-1), fbt);
	#endif
#endif
	return Retval;
}

UINT_T EraseFlash(UINT_T flash_addr, UINT_T size, FlashBootType_T fbt)
{
    UINT_T Retval = NoError;

#if (COPYIMAGESTOFLASH)
    UINT_T erase_addr, total_blks, BlkSize, PageSize, PartitionOffset, BlkNum, retry;
    P_FlashProperties_T pFlashP = GetFlashProperties(fbt);
    P_FMProperties_T pFMProps = GetFMProperties();

    EraseFlash_F erase = pFlashP->EraseFlash;
	

    if(erase == NULL)
        return FlashFuncNotDefined;

	if (size == 0)
		return NoError;

#if SPI_CODE
	Retval = erase(flash_addr, size, fbt);
#else
    //set up initial values
    BlkSize = pFlashP->BlockSize;
    PageSize = pFlashP->PageSize;
    retry = TRUE;
    PartitionOffset = GetPartitionOffset(fbt);
    // normalize addr and size to the whole blocks
    BlkNum = (flash_addr / BlkSize);
    erase_addr = BlkNum * BlkSize;

    while (erase_addr < (flash_addr + size))
    {
		// add here to judge if this block is in the skip list, if it is in skip list, then continue to next block;
		
		if(pSkipAddress != NULL)
		{
			if(CheckIfSkip(BlkNum))
				goto NEXT_BLOCK;
		}
        //check to see if the block being erase contains a BBT or PT
        CheckFMEraseState(BlkNum, fbt);

        // Check for relocations
        BlkNum = LocateBlock(BlkNum, fbt);
        // erase the current block
        Retval = erase( (BlkNum * BlkSize) + PartitionOffset, BlkSize, fbt);
        //erase failure means either the block is bad, or device is wedged
        if (Retval != NoError)
        {
            //try a reset first
            if(retry == TRUE)
            {   //clear the flag, and reset the device
                retry = FALSE;
                Retval = ResetFlash(fbt);
                //if the reset failed, device is wedged.  return error
                if(Retval != NoError)
                    return Retval;
                else //if reset succeeded, try the block again
                    continue;
            }
            else
            {   //at this point, reset didn't help.  so block is BAD

				// if this media doesn't support BBM, there is no other recovery possible.
				if( !pFlashP->FlashSettings.UseBBM)
					return Retval;

                //relocate block, but don't worry about catching new BlkNum, as next LocateBlock will pick it up
                Retval = RelocateBlock(BlkNum, &BlkNum, fbt);
                if(Retval != NoError)
                    return Retval;

                //reset flag for the new block
                retry = TRUE;
                continue;
            }
        }

NEXT_BLOCK:
        //update counting stats
        // - address pointers
        erase_addr += BlkSize;
        BlkNum = erase_addr / BlkSize;
        retry = TRUE;
    }// End of while
#endif
#endif
    return Retval;
}

UINT_T ResetFlash(FlashBootType_T fbt)
{
    ResetFlash_F reset = GetFlashProperties(fbt)->ResetFlash;
    if(reset == NULL)
        return FlashFuncNotDefined;
    else
        return reset(fbt);
}

UINT_T ReadOTP(UINT_T OTPAddress, UINT_T LocalBuffer, UINT_T size, FlashBootType_T fbt)
{
#if (COPYIMAGESTOFLASH)
    ReadOTP_F readotp = GetFlashProperties(fbt)->ReadOTP;

    if(readotp == NULL)
        return FlashFuncNotDefined;
    else
        return readotp(OTPAddress, LocalBuffer, size);
#else
    return NoError;
#endif
}

UINT_T WriteOTP(UINT_T OTPAddress, UINT_T LocalBuffer, UINT_T size, FlashBootType_T fbt)
{

#if (COPYIMAGESTOFLASH)
    WriteOTP_F writeotp = GetFlashProperties(fbt)->WriteOTP;

    if(writeotp == NULL)
        return FlashFuncNotDefined;
    else
        return writeotp(OTPAddress, LocalBuffer, size);
#else
    return NoError;
#endif
}

UINT_T LockOTP(FlashBootType_T fbt)
{
#if (COPYIMAGESTOFLASH)
    LockOTP_F lockotp = GetFlashProperties(fbt)->LockOTP;

    if(lockotp == NULL)
        return FlashFuncNotDefined;
    else
        return lockotp();
#else
    return NoError;
#endif
}
UINT_T GetBlockSize(FlashBootType_T fbt)
{
    return GetFlashProperties(fbt)->BlockSize;
}

UINT_T GetPageSize(FlashBootType_T fbt)
{
    return GetFlashProperties(fbt)->PageSize;
}

UINT_T GetEraseRegionSize(FlashBootType_T fbt)
{
    return GetFlashProperties(fbt)->BlockSize;
}

#if COPYIMAGESTOFLASH
UINT_T GetUseSpareArea(FlashBootType_T fbt)
{
    return GetFlashProperties(fbt)->FlashSettings.UseSpareArea;
}

UINT_T SetUseSpareArea(UINT_T bUseSpareArea, FlashBootType_T fbt)
{
    P_FlashProperties_T pFlashProps = GetFlashProperties(fbt);
    pFlashProps->FlashSettings.UseSpareArea = ((bUseSpareArea == 0) ? 0 : 1);

    return GetUseSpareArea(fbt);
}

UINT_T GetSpareAreaSize(FlashBootType_T fbt)
{
    return GetFlashProperties(fbt)->FlashSettings.SASize;
}

UINT_T SetSpareAreaSize(UINT_T SASize, FlashBootType_T fbt)
{
    P_FlashProperties_T pFlashProps = GetFlashProperties(fbt);
    pFlashProps->FlashSettings.SASize = SASize;

    return GetSpareAreaSize(fbt);
}

UINT_T GetUseHwEcc(FlashBootType_T fbt)
{
    return GetFlashProperties(fbt)->FlashSettings.UseHwEcc;
}

UINT_T SetUseHwEcc(UINT_T bUseHwEcc, FlashBootType_T fbt)
{
    P_FlashProperties_T pFlashProps = GetFlashProperties(fbt);
    pFlashProps->FlashSettings.UseHwEcc = ((bUseHwEcc == 0) ? 0 : 1);

#if NAND_CODE
    xdfc_enable_ecc( bUseHwEcc );
#endif

    return GetUseHwEcc(fbt);
}
#endif

UINT_T ConvertToLogicalAddr(UINT_T FlashLocation, FlashBootType_T fbt)
{
    ConvertToLogicalAddr_F logaddr = GetFlashProperties(fbt)->pConvertToLogicalAddr;

    if(logaddr == NULL)
        return FlashLocation;
    else
        return logaddr(FlashLocation, fbt);

}

#if 0
UINT_T FlashInitAfterDownload( void *pTimHeader, DataSearchFunc_F FindPackageFunc, DataSearchFunc_F FindFirstPackageFunc,
                               DataSearchFindNext_F FindNextPackageFunc, FlashBootType_T fbt )
{
   UINT_T *pkgIDs;
   FlashInitAfterTimDownload_F FlashInitFunc = GetFlashProperties(fbt)->FlashInitAftDl;

   if( FlashInitFunc == NULL )
     return FlashFuncNotDefined;

   pkgIDs = GetFlashProperties(fbt)->ReservedPkgIDs;
   return FlashInitFunc( pTimHeader, FindPackageFunc, FindFirstPackageFunc, FindNextPackageFunc,  pkgIDs );
}
#endif
