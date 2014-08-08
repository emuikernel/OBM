/******************************************************************************
**  COPYRIGHT  2007 Marvell Inernational Ltd.
**  All Rights Reserved
******************************************************************************/

#include "Typedef.h"
#include "FM.h"
#include "Partition.h"
#include "Errors.h"
#include "Flash.h"
#include "misc.h"
#include "loadoffsets.h"

#if COPYIMAGESTOFLASH
#include "BootLoader.h"
#endif

//Currently, the largest page size we'll see is 4KB (1K words)
#define MAX_PAGE_SIZE_WORDS 0x400
#define MAX_TIM_SIZE_BYTES	0x1000

//allocated space for a bad block table and the partition table
UINT_T FM_SPACE = DDR_FM_SPACE_ADDR;

//keep a pointer that points the beginning of the UNUSED data space allocated for FM
UINT_T *pFM_SPACE;

//routine to get a pointer to the FM properties
FMProperties_T FMProperties;

P_FMProperties_T GetFMProperties()
{
	return &FMProperties;
}

/*
 * IntializeFM
 *
 * This function intializes the Flash Management, so as to allow use of Bad Block Management
 * and a Partition Table.  This function supports legacy BBM, as well as the new Marvell method.
 *
 * Inputs:
 *	FM_type - tells us which type of Flash Management we should search for (Legacy, Marvell, both, etc.)
 * Outputs: none
 * Returns: none
 */
void InitializeFM(FM_Method_T FM_type, FlashBootType_T fbt)
{
	UINT_T Retval, j;
	P_FMProperties_T pFMProp = GetFMProperties();

	//if we only allow legacy method, just search for that
	if(FM_type == LEGACY_METHOD)
	{
		pFMProp->FindBBT = &FindBBT_Legacy;
		Retval = pFMProp->FindBBT(0, fbt);
		#if COPYIMAGESTOFLASH
		if(Retval != NoError)
		{
			CreateBBT_Legacy(NULL);
			#if NAND_CODE
			//Retval = ScanNANDForBadBlocks(fbt);
			#endif
		}
		#endif
	}
	else
	{//otherwise, we need to read boot block to see what we have in flash
#if NEW_MARVELL_BBM
		Retval = ScanBootBlock(FM_type, fbt);
#endif
	}
}

//This function will write out any unwritten PT or BBT
void FinalizeFM(FlashBootType_T fbt)
{
	UINT_T Retval, size;
	P_FMProperties_T pFMProp = GetFMProperties();

	//second, write out the PT    - second conditional is a sanity check (can't overwrite TIM)
	if( (pFMProp->PT_state == PT_NEW) && (pFMProp->PT_location > 0) )
	{
		size = sizeof(PartitionTable_T) + pFMProp->pMPT->NumPartitions * sizeof(PartitionInfo_T);
		WriteFlash(pFMProp->PT_location, (UINT)pFMProp->pMPT, size, fbt);
	}
	//third, make sure the BBT is written out.  This is for the case that the partition was already
	//set to 0, and the outstanding BBT did not get written.  NOTE: it must work this way,	since
	//the PT in Block 0 MUST be written before any other BBT in block 0 (MLC restriction on page burning)
	UpdateBBT();

	//lastly, clear out the structure
	ClearFM(fbt);
}

/*
 * ClearFM
 *
 * 	This function clears out the Flash Management data space and
 *	sets all state variables to 'invalid', so as to invalidate the
 *	Flash Management until is has been intialized.
 */
void ClearFM(FlashBootType_T fbt)
{
	UINT_T tempPartNum, i;
	P_FMProperties_T pFMProp = GetFMProperties();

	//Step 1: Set variables to a default state
	//Clear out the properties structure (deletes stale data)
	// '>> 2' converts byte total to DWord total
	tempPartNum = pFMProp->PI_num;		// Save this value as it was determined from flash driver
	for (i = 0; i < (sizeof(FMProperties_T) >> 2); i++)
		((UINT_T*)pFMProp)[i] = 0;
	pFMProp->PI_num = tempPartNum;		// Restore value

	//invalidate the BBT and PT
	SetBBTState(BBT_INVALID, fbt);
	pFMProp->PT_state = PT_INVALID;

	//Clear out our global area space
	pFM_SPACE = FM_SPACE;
	for (i = 0; i < MAX_PAGE_SIZE_WORDS*2; i++)
	{
		*(UINT_T *)(FM_SPACE + i) = 0;
	}

}
/*
 * ScanBootBlock
 *
 * This function will scan the first few pages of the boot block, looking for either a
 * Partition Table, or a Factory Bad Block Table.  Scanning starts at page 1, since
 * page 0 is reserved for TIM.  Since TIM size is dynamic, we will continue reading pages
 * until we find one of the aforementioned objects.
 *
 * Note: Currently, the upper limit (exclusive) of the search will be first page that
 * starts *after* the 8K mark, as it is assumed the TIM won't grow beyond 8K.  (Ex.  For a
 * 2K page size device, we will scan page 1-4).
 */
#if NEW_MARVELL_BBM
UINT_T ScanBootBlock(FM_Method_T FM_type, FlashBootType_T fbt)
{
	UINT_T Retval = NoError;
	UINT_T page_num, page_size, pt_size, search_limit, tmp, pt_id[2];
	P_FMProperties_T pFMProp = GetFMProperties();
	P_FlashProperties_T pFlashP = GetFlashProperties(fbt);

	//grab ID
	pt_id[0] = MARVELL_PARTITION_TABLE_ID0;
	pt_id[1] = MARVELL_PARTITION_TABLE_ID1;

	page_size = GetPageSize(fbt);
	page_num = 1;
	search_limit = 2 * MAX_TIM_SIZE_BYTES; //set the initial search limit to a couple pages beyond TIM
	//NOTE: If PT and FBBT present, 'search_limit' variable is expanded to allow searching for the FBBT


	//Start reading at Page 1, one page at a time, till we find a Partition Table, or BBT
	do
	{
		Retval = ReadFlash((page_size * page_num) + pFlashP->TimFlashAddress, (UINT_T)pFM_SPACE, page_size, fbt);
		page_num++;
		//if there was an error reading the page, continue to the next page
		if(Retval != NoError)
			continue;

		//Did we find a Marvell Partition Table?
		if( 0 == memcmp(pt_id, pFM_SPACE, 8) ) 	//check Partition Table ID
		{
			//set Partition Table pointers
			pFMProp->pMPT = (P_PartitionTable_T)pFM_SPACE;
			//remember where we found the PT
			pFMProp->PT_location = page_size * (page_num - 1) + pFlashP->TimFlashAddress;
			//find out the size of the PT
			pt_size = sizeof(PartitionTable_T) + pFMProp->pMPT->NumPartitions * sizeof(PartitionInfo_T);
			//make sure we read the whole PT.
			if(pt_size > page_size)
			{
				Retval = ReadFlash((page_size * page_num) + pFlashP->TimFlashAddress, (UINT_T)pFM_SPACE + page_size, pt_size - page_size, fbt);
				//catch the error
				if(Retval != NoError)
				{	//Bogus PT.  NULL out pointer, and continue
					pFMProp->pMPT = NULL;
					continue;
				}
				//figure out how many pages we just read
				tmp = pt_size / page_size;
				tmp += (tmp * page_size < pt_size) ? 1 : 0;
				//bump up the page num
				page_num += tmp;
				//incrememnt our search limit to the next page after the end of the PT
				search_limit += tmp * page_size;
			} else
				search_limit += page_size;

			pFMProp->PT_state = PT_VALID;
			//now set the buffer_ptr till after the PT, so we can try and read the FBBT
			pFM_SPACE = (UINT_T *)((UINT_T)FM_SPACE + pt_size);
			continue;  //keep going, so we can try to read the FBBT
		}

		//Marvell BBT?
		if( (pFlashP->FlashSettings.UseBBM == 1) &&				//does this device use BBT's?
		    (BBT_TYPE_MBBT == pFM_SPACE[0]) )
		{
			//set the properties
			pFMProp->BBT_type = BBT_TYPE_MBBT;
			SetBBTState(BBT_UNCHANGED, fbt);
			pFMProp->ScanBBT = &ScanBBT_Marvell;
			pFMProp->FindBBT = &FindBBT_Marvell;
			//set the runtime BBT pointer to this location
			pFMProp->pFBBT = (P_FBBT_Table_T)pFM_SPACE;
			pFMProp->BBT_location = page_num * page_size + pFlashP->TimFlashAddress;
			//don't update pFM_SPACE, but let RBBT overwrite FBBT, as it is more useful
			break;  //break out of while loop
		}

	}while(page_num * page_size <= search_limit); //the partition table cannot be beyond the 8K (+ 1 page) limit

	//if we found a Factory BBT, use it to load the RBBT
	if(pFMProp->pFBBT != NULL)
		pFMProp->FindBBT(pFMProp->pFBBT->BBTLocation, BOOT_FLASH);

	return Retval;
}
#endif

/* LocateBlock
 *
 * This function using FM structures to determine if the input block
 * has been relocated.  It will check to see if legacy BBM is being used (and
 * therefore use that BBT) or if Marvell method is being used.
 *
 * NOTE: this function assumes user already called SetPartition at a higher level
 *
 *	Inputs:
 * 		BlockNum: The block number to check.  This block num is relative to
 *			the partition number
 *		fbt: Flash boot type - BOOT_FLASH or SAVE_STATE_FLASH
 *	Outputs: none
 *	Returns: The block number that the input relocates to (or the same number if the
 *			original block was not relocated)
 */
UINT_T LocateBlock(UINT_T BlockNum, FlashBootType_T fbt)
{
	UINT_T RetBlock;
	P_FMProperties_T pFMProp = GetFMProperties();

	//make sure this device uses BBM
	if(GetFlashProperties(fbt)->FlashSettings.UseBBM == 0)
		return BlockNum;

	//make sure a BBT was found, and scan function was linked
	if(pFMProp->ScanBBT == NULL)
		return BlockNum;

	//scan the BBT
	RetBlock = pFMProp->ScanBBT(BlockNum);
	//Return the relocated block
	return RetBlock;
}

/*
 * Relocate Block
 *
 * This function will take in a block number, call the correct RelocateBlock routine,
 * and return with the new block num
 *
 * Inputs:
 *	 BlockNum:  The bad block to be put into the table
 *   fbt: Flash boot type
 * Outputs:
 *   *NewBlock: sets the new block number
 * Returns:	No Error on a successful relocation
 *			NotSupportedError if the flash device does not use BBM
 */
UINT_T RelocateBlock(UINT_T BlockNum, UINT_T* NewBlock, FlashBootType_T fbt)
{
	UINT_T Retval = NotSupportedError;

#if COPYIMAGESTOFLASH
	//if this flash device does not support BBM, return with an error
	if(GetFlashProperties(fbt)->FlashSettings.UseBBM == 0)
		return NotSupportedError;

	//otherwise, call the relevant flash management Relocate routine
	if(GetFMProperties()->BBT_type == BBT_TYPE_LEGACY)
		Retval = RelocateBlock_Legacy(BlockNum, NewBlock);
	else{
#if NEW_MARVELL_BBM
		Retval = RelocateBlock_Marvell(BlockNum, NewBlock);
#endif
	}
#endif

	return Retval;
}
void CheckFMEraseState(UINT_T BlockNum, FlashBootType_T fbt)
{
	P_FMProperties_T pFMProp = GetFMProperties();

	//if we have a PT, and we are erasing block 0, mark that it needs to be reburnt
	if((pFMProp->PT_state != PT_INVALID) && (BlockNum == 0))
		pFMProp->PT_state = PT_NEW;

	//if we have a BBT, check to see if the block containing the BBT is being erase
	if((pFMProp->BBT_State != BBT_INVALID) && 0 == BlockNum)
		SetBBTState(BBT_ERASED, fbt);
}

/*
 * SetBBTState
 *
 * This function is used to change tbe BBT state.  This allows FM to keep
 * track of whether or not to use the BBT, and if it needs to be re-written
 * when the flash is shutdown.
 */
void SetBBTState(ReloState_T newState, FlashBootType_T fbt)
{
	UINT_T BlkSize, PageSize;
	P_FMProperties_T pFMProps = GetFMProperties();

	//make sure this device uses BBM
	if(GetFlashProperties(fbt)->FlashSettings.UseBBM == 0)
		return;

	// Now set some values that will work
	PageSize = GetPageSize(fbt);
	BlkSize = GetBlockSize(fbt);

	//check if the BBT got erase.  If so, set the slot numbers back to their starting slot
	if ( newState == BBT_ERASED )
	{
	    pFMProps->BBT_Slot = (BlkSize / PageSize) - 1;
		pFMProps->BBT_NextSlot = pFMProps->BBT_Slot - 1;
		pFMProps->BBT_location = pFMProps->BBT_Slot * PageSize;


//		pFMProps->BBT_Slot = (pFMProps->BBT_location & (BlkSize - 1)) / PageSize; //mask off full Blocks, then get page number
		//if this is the rare case where the BBT grows downward, set slots to grow down
//		if(pFMProps->BBT_Slot == (BlkSize / PageSize))
//			pFMProps->BBT_NextSlot = pFMProps->BBT_Slot - 1;
//		else //most common case: grows up
//			pFMProps->BBT_NextSlot = pFMProps->BBT_Slot + 1;
	}

	pFMProps->BBT_State = newState;
}

//this function creates a read disturb table in FM_SPACE... space
P_ReadDisturbList_T CreateReadDisturbList()
{
	P_FMProperties_T pFMProp = GetFMProperties();

	//if the list already exists, just return it
	if(pFMProp->pReadDisturbList != NULL)
		return pFMProp->pReadDisturbList;

	//put structure into data (FM) space
	pFMProp->pReadDisturbList = (P_ReadDisturbList_T) DDR_DISTURBLIST_DDR;
	//pFM_SPACE += sizeof(ReadDisturbList_T);

	//initialize structure
	pFMProp->pReadDisturbList->RDID = READ_DISTURB_LIST_ID;
	pFMProp->pReadDisturbList->num_entries = 0;

	return pFMProp->pReadDisturbList;
}

unsigned int AddReadDisturbEntry(unsigned int partition, unsigned int block)
{
	unsigned int i;
	ReadDisturbEntry_T entry;
	P_ReadDisturbList_T pRDlist = GetFMProperties()->pReadDisturbList;

	//make sure we have a valid list
	if(pRDlist == NULL)
		pRDlist = CreateReadDisturbList();

	entry.partition = partition;
	entry.block = block;

	//parse list and return if the entry is already listed
	for(i = 0; i < pRDlist->num_entries; i++)
		if(entry.value == pRDlist->entries[i].value)
			return NoError;

	//at this point, entry wasn't found.  add it to list
	pRDlist->entries[pRDlist->num_entries].value = entry.value;
	pRDlist->num_entries++;
	return NoError;
}


void ImportReadDisturbList(unsigned int import_list)
{
	unsigned int i;
	P_ReadDisturbList_T import = (P_ReadDisturbList_T) import_list;

	//no need to check if an internal table exists, AddReadDisturbEntry will check for us

	//need to parse thru entries in the import list
	for (i = 0; i < import->num_entries; i++)
		AddReadDisturbEntry(import->entries[i].partition, import->entries[i].block);
}


//this function takes in a pointer to a partition table, and loads in into the FM space,
//sets the state variables, and creates an necessary BBT's
#if NEW_MARVELL_BBM
void LoadExternalPartitionTable(UINT_T *table, UINT_T location)
{
#if COPYIMAGESTOFLASH
	UINT_T size, num_partitions, i, j, pt_id[2];
	P_FMProperties_T pFMProp = GetFMProperties();
	P_PartitionInfo_T pPI;

	//grab ID
	pt_id[0] = MARVELL_PARTITION_TABLE_ID0;
	pt_id[1] = MARVELL_PARTITION_TABLE_ID1;

	//if this is not a MARVELL PT, return
	if( 0 != memcmp(table, pt_id, 8) )
		return;

	//first, move the table into FM_SPACE
	num_partitions = ((P_PartitionTable_T)table)->NumPartitions;
	size = sizeof(PartitionTable_T) + num_partitions * sizeof(PartitionInfo_T);
	//MAX_PAGE...WORDS *4*2 from words to bytes
	if((size + (UINT_T)pFM_SPACE) > ((UINT_T)FM_SPACE + MAX_PAGE_SIZE_WORDS*4*2))
	{
	   AddMessage((UINT8_T*)("***Ran out of FM_SPACE for PT!!\0"));
	   return;
	}
	memcpy(pFM_SPACE, table, size);

	//second, update variables
	pFMProp->pMPT = (P_PartitionTable_T) pFM_SPACE;
	pFMProp->PT_state = PT_NEW;
	pFMProp->PT_location = location;
	pFM_SPACE = (UINT*) ((UINT)pFM_SPACE + size);

	//third, scan through partitions and create BBTs (if necessary)
	for(i = 0; i < num_partitions; i++)
	{
		SetPartition(i, BOOT_FLASH);
		if(pFMProp->BBT_State == BBT_INVALID)
		{
			if(pFMProp->pPI->BBT_Type == BBT_TYPE_LEGACY)
				CreateBBT_Legacy(pFMProp->pPI);
			if(pFMProp->pPI->BBT_Type == BBT_TYPE_MBBT)
				CreateBBT_Marvell(pFMProp->pPI);
		}
#if SCAN_FOR_BADBLOCKS
		//erase all the blocks.	This will force scanning for bad blocks
		for(j = pFMProp->pPI->StartAddr; j < pFMProp->pPI->EndAddr; j += GetBlockSize(BOOT_FLASH))
			EraseFlash(j, GetBlockSize(BOOT_FLASH), BOOT_FLASH);
#endif
	}
#endif
}
#endif

/* SetPartition
 * 	This function sets the FM structures to point to a particular partition.
 *  This includes updating the Partition Info structure in memory, and the runtime
 *	BBT (both from FLASH).  If there is not partition table, this function just returns.
 *
 *	Inputs:
 *		PartitionNum - partition number to set to
 *		fbt - flash boot type to use
 *	Outputs: none
 *	Returns: none
 */
#if NEW_MARVELL_BBM 
void SetPartition(UINT_T PartitionNum, FlashBootType_T fbt)
{
	UINT_T Retval;
	P_FMProperties_T pFMProp = GetFMProperties();
	P_FlashProperties_T pFlashP = GetFlashProperties(fbt);

	//if we don't have a valid Partition Table, return
	if(pFMProp->PT_state == PT_INVALID)
	{
		// before returning, check if partitions are supported by hardware (eg. emmc)
		//if there is a set partition function for the LLD, then call it
		if(pFlashP->ChangePartition != NULL)
			pFlashP->ChangePartition(PartitionNum, fbt);

		return;
	}

	//if we're already set on the right partition, return
	if(pFMProp->PI_num == PartitionNum)
		return;

	//if attempting to set to a Partition not contained in the table, invaliate info and return
	if(PartitionNum > pFMProp->pMPT->NumPartitions)
	{
		pFMProp->pPI = NULL;
		return;
	}

	//write out previous BBT before we change partitions
	//Note:  This routine checks if the current loaded BBT has been modified,
	//			and will only write out a changed BBT
	UpdateBBT();

	//now invalid the BBT, since it will not reflect the new partition
	SetBBTState(BBT_INVALID, fbt);
	//set the partition number in the properties structure
	pFMProp->PI_num = PartitionNum;

	//set the partition info pointer to point to this partition info
	pFMProp->pPI = (P_PartitionInfo_T) ((UINT_T)pFMProp->pMPT + sizeof(PartitionTable_T)
		 + PartitionNum * sizeof(PartitionInfo_T));

	//if there is a partition function for the LLD, we need to call hardware to change partition
	if(pFlashP->ChangePartition != NULL)
		pFlashP->ChangePartition(PartitionNum, fbt);

	//if the device does not use BBM, then return here
	if(pFlashP->FlashSettings.UseBBM == 0)
		return;

	//figure out with BBT type to look for
	if(pFMProp->pPI->BBT_Type == BBT_TYPE_MBBT)
		pFMProp->FindBBT = &FindBBT_Marvell;
	else //anyother BBT type is not support, so just return
		return;

	Retval = pFMProp->FindBBT(pFMProp->pPI->RBBT_Location, fbt);

	//if it failed, try reading backup
	if((Retval != NoError) && (pFMProp->pPI->BackupRBBT_Location != NULL))
		pFMProp->FindBBT(pFMProp->pPI->BackupRBBT_Location, fbt);

}
#else
#if MMC_CODE
void SetPartition(UINT_T PartitionNum, FlashBootType_T fbt)
{
	P_FlashProperties_T pFlashP = GetFlashProperties(fbt);

	if(pFlashP->ChangePartition != NULL)
		pFlashP->ChangePartition(PartitionNum, fbt);

	return;
}
#endif
#endif

/*	GetPartitionOffset
 *
 *	This function returns the logical offset of where the currently set partition begins.
 *	If no partition table exists OR no partition was set this function returns 0.
 *
 */
UINT_T GetPartitionOffset(FlashBootType_T fbt)
{
	P_FMProperties_T pFMProp = GetFMProperties();

	//if we don't have a Partition table, return an offset of 0
	if(pFMProp->PT_state == PT_INVALID)
		return 0;

	//if the partition was not set, return an offset of 0
	if(pFMProp->pPI == NULL)
		return 0;

	return pFMProp->pPI->StartAddr;

}

/* CreateBBT_Legacy
 *   This function will creates a legacy BBT.  If the Partition Info pointer is valid, use the values in
 * it to create the BBT.  If the Partition Info pointer is NULL, this function will create a default BBT
 * that will get written after the TIM.
 *
 * NOTE!!::  To support older flash management, you can change the initial slot for the BBT from right after
 *			the TIM, to the last slot of block zero in this routine.
 * Inputs:
 *      pPI
 * Outputs:
 *      None
 *
 */
void CreateBBT_Legacy(P_PartitionInfo_T pPI)
{
#if COPYIMAGESTOFLASH

    int i;
    UINT_T Retval = NoError;
    UINT_T PageSize, BlkSize, mask;

    P_FMProperties_T pFMProp = GetFMProperties();
    P_ReloTable_T pLBBT = pFMProp->pLBBT;

    //If this flash device does not use BBM, just return
    if(GetFlashProperties(BOOT_FLASH)->FlashSettings.UseBBM == 0)
        return;

    //Initialize FMProperties structure:
    SetBBTState(BBT_INVALID, BOOT_FLASH);
	PageSize = GetPageSize(BOOT_FLASH);
	BlkSize = GetBlockSize(BOOT_FLASH);
	memset(pFM_SPACE, 0xFF, PageSize);

	pFMProp->pLBBT = (P_ReloTable_T) pFM_SPACE;
	pFMProp->pLBBT->Header = (USHORT) BBT_TYPE_LEGACY;
	pFMProp->pLBBT->NumReloc = 0;
	pFMProp->BBT_type = BBT_TYPE_LEGACY;

	//NOTE::  SET THE STARTING SLOT HERE!!!
	// For now use the last page in block zero to match XDB
	   //	pFMProp->BBT_Slot = (PageSize >= MAX_TIM_SIZE_BYTES) ? 1 : MAX_TIM_SIZE_BYTES / PageSize;
	    pFMProp->BBT_Slot = (BlkSize / PageSize) - 1;
		pFMProp->BBT_NextSlot = pFMProp->BBT_Slot - 1;
		pFMProp->BBT_location = pFMProp->BBT_Slot * PageSize;
 
    // initialize the FMProps for the new BBT
    SetBBTState(BBT_CHANGED, BOOT_FLASH);

	//increment our memory pointer (DWORD pointer!!)
//	pFM_SPACE = (UINT*) ((UINT)pFM_SPACE + PageSize);

#endif
    return;
}


/* CreateBBT_Marvell
 *   This function creates a Marvell BBT. If the Partition Info pointer is valid, it will that to
 * create the BBT.  If the Partition Info pointer is NULL, this function will create a default BBT
 * that will get written after the TIM.
 *
 * NOTE:: All Marvell BBT's MUST grow upwards.  A downward growing BBT is Legacy ONLY
 *
 * Inputs:
 *      Offset - location of where this BBT resides
 *		pPI - pointer the partition info
 * Outputs:
 *      None
 *
 */
#if NEW_MARVELL_BBM 
void CreateBBT_Marvell(P_PartitionInfo_T pPI)
{
    int i;
#if COPYIMAGESTOFLASH

    UINT_T Retval = NoError;
    UINT_T PageSize, BlkSize, mask;

    P_FMProperties_T pFMProp = GetFMProperties();
	P_MBBT_Table_T pRBBT = pFMProp->pRBBT;

    //If this flash device does not use BBM, just return
    if(GetFlashProperties(BOOT_FLASH)->FlashSettings.UseBBM == 0)
        return;

    //Initialize FMProperties structure:
    SetBBTState(BBT_INVALID, BOOT_FLASH);
	PageSize = GetPageSize(BOOT_FLASH);
	BlkSize = GetPageSize(BOOT_FLASH);
	memset(pFM_SPACE, 0xFF, PageSize);

	//initialize the FMProps for the new BBT
	pFMProp->pRBBT = (P_MBBT_Table_T) pFM_SPACE;
	pFMProp->pRBBT->Identifier = (UINT) BBT_TYPE_MBBT;
	pFMProp->pRBBT->Version = MARVELL_BBT_VERSION;
	pFMProp->pRBBT->Type = BBT_TYPE_MBBT_RUN;
	pFMProp->pRBBT->NumRelo = 0;
	pFMProp->BBT_type = BBT_TYPE_MBBT;

	mask = BlkSize - PageSize;

	if(pPI == NULL)
	{	//create default BBT
		//right after the TIM (no PT present)
		pFMProp->pRBBT->BBTLocation = (PageSize >= MAX_TIM_SIZE_BYTES) ? PageSize : MAX_TIM_SIZE_BYTES;
		pFMProp->pRBBT->BackupBBTLoc = pFMProp->pRBBT->BBTLocation;	//set at same location, so only 1 BBT can be written
		pFMProp->BBT_location = pFMProp->pRBBT->BBTLocation;	//block 0
		pFMProp->BBT_Slot = pFMProp->pRBBT->BBTLocation / PageSize; //get the page number the start location
	}
	else
	{
		pFMProp->pRBBT->BBTLocation = pPI->RBBT_Location;
		pFMProp->pRBBT->BackupBBTLoc = pPI->BackupRBBT_Location;
		pFMProp->BBT_location = pPI->RBBT_Location;
		pFMProp->pRBBT->PartitionID = pPI->Indentifier;
		pFMProp->BBT_Slot = 0;
	}


    pFMProp->BBT_NextSlot = pFMProp->BBT_Slot + 1;
	SetBBTState(BBT_CHANGED, BOOT_FLASH);

	//increment our memory pointer
	pFM_SPACE = (UINT*) ((UINT)pFM_SPACE + PageSize);
#endif
    return;
}
#endif

void UpdateBBT()
{
#if COPYIMAGESTOFLASH
    UINT_T addr = 0;
    UINT_T Retval, BlkSize, PageSize, NumPages, TIMpagesUsed, bbt;
    P_FMProperties_T pFMProps = GetFMProperties();
	P_ReloTable_T pBBT;
	P_FlashProperties_T pFlashProp = GetFlashProperties(BOOT_FLASH);
	UINT_T maxBlockNum, i;

    //If this flash device does not use BBM, just return
	if(pFlashProp->FlashSettings.UseBBM == 0)
		return;

    //update BBT (only if there are changes)
    if(pFMProps->BBT_State == BBT_UNCHANGED)
        return;

    //if we never found a BBT, just return
    if(pFMProps->BBT_State == BBT_INVALID)
        return;

	
    // Now set some values that will work
    PageSize = GetPageSize(BOOT_FLASH);
    BlkSize = GetBlockSize(BOOT_FLASH);


	//are we burning to block 0, where TIM resides??
	if(pFMProps->BBT_location < BlkSize)
	{
		//Find and read first page after the TIM
	    TIMpagesUsed = PageSize >= MAX_TIM_SIZE_BYTES ? 1 : MAX_TIM_SIZE_BYTES / PageSize;

	    NumPages = BlkSize / PageSize;

	    //Did we run out of BBT Slots?
	    if((pFMProps->BBT_Slot < TIMpagesUsed) )
	    {
		    // ASSUMES ONLY TIM and BBT are located in block 0
	        //save data in lower portion of block 0
	        Retval = ReadFlash(0, TEMP_BUFFER_AREA, MAX_TIM_SIZE_BYTES, BOOT_FLASH);
	        if(Retval != NoError)
				return;
			
	        //erase all of block 0
	        Retval = EraseFlash(0, BlkSize, BOOT_FLASH);
	        if(Retval != NoError)
	            return;

	        //restore TIM data in lower portion of block 0
	        Retval = WriteFlash(0, TEMP_BUFFER_AREA, MAX_TIM_SIZE_BYTES, BOOT_FLASH);
	        if(Retval != NoError)
	            return;

			//reset slot counters
//			if(pFMProps->BBT_Slot > pFMProps->BBT_NextSlot)
//			{
				pFMProps->BBT_Slot = NumPages - 1;
				pFMProps->BBT_NextSlot = NumPages - 2;
				pFMProps->BBT_location = pFMProps->BBT_Slot*PageSize;
//			}
//			else
//			{
//				pFMProps->BBT_Slot = TIMpagesUsed;
//				pFMProps->BBT_NextSlot = TIMpagesUsed + 1;
//			}
	    }
	}
//next_step:
	if(pFMProps->BBT_type == BBT_TYPE_LEGACY)
		bbt = (UINT)pFMProps->pLBBT;
	else
		bbt = (UINT)pFMProps->pRBBT;


	//get the block offset from BBT_location
	addr = pFMProps->BBT_location & ~(BlkSize - 1);
	//get the page offset from SLot
	addr |= pFMProps->BBT_Slot * PageSize;

	//write out BBT
    WriteFlash(addr, bbt, PageSize, BOOT_FLASH);

	//update slot info
    //if(pFMProps->BBT_Slot > pFMProps->BBT_NextSlot)
    //{
		pFMProps->BBT_Slot--;
		pFMProps->BBT_NextSlot--;
	//}
	//else
	//{
	//	pFMProps->BBT_Slot++;
	//	pFMProps->BBT_NextSlot++;
	//}

	//set the state back to unchanged
    SetBBTState(BBT_UNCHANGED, BOOT_FLASH);
	
	// make sure the related block is not out of range, 0 to max block number
	maxBlockNum = pFlashProp->NumBlocks - 1;
	
	if(pFMProps->BBT_type == BBT_TYPE_LEGACY)
	{
		pBBT = pFMProps->pLBBT;
		
		for (i = 0; i < pBBT->NumReloc; i++)
		{
			if ((pBBT->Relo[i].From > maxBlockNum) || (pBBT->Relo[i].To > maxBlockNum))
			{
				FatalError(BBTOUTOFRANGE);
			}
		}
	}

#endif
    return;
}


/* ScanBBT_Legacy - uses Legacy BBT structure
 *	This function will take in a Block Number and return with the relocated
 *	 block number.  If the block is not relocated, then it will simply return
 *	 with the original block number
 *
 *	Inputs:
 *		BlockNum: The block number to look up in the relocation table
 *	Outputs:
 *		None
 *	Retval value: Relocated block number
 */
UINT_T ScanBBT_Legacy(UINT_T BlockNum)
{
	P_FMProperties_T pFMProp = GetFMProperties();
	P_ReloTable_T pBBT = pFMProp->pLBBT;
	UINT_T i;

	//if no BBT was loaded, return original block num
	if(pFMProp->BBT_State == BBT_INVALID)
		return BlockNum;

	//block 0 is guaranteed by manufacture
	if(BlockNum == 0)
		return 0;

	// Has this block been relocated?	If so we'll need to use the relocated block
	for (i = 0; i < pBBT->NumReloc; )
	{
		if (pBBT->Relo[i].From == BlockNum)
		{
			BlockNum = pBBT->Relo[i].To;
			i = 0;	// Start over in case remapped block is now bad
			continue;
		}
		i++;
	}
	return BlockNum;
}

/* ScanBBT_Marvell - uses new Marvell BBT structure
 *	This function will take in a Block Number and return with the relocated
 *	 block number.  If the block is not relocated, then it will simply return
 *	 with the original block number
 *
 *	Inputs:
 *		BlockNum: The block number to look up in the relocation table
 *	Outputs:
 *		None
 *	Retval value: Relocated block number
 */
#if NEW_MARVELL_BBM 
UINT_T ScanBBT_Marvell(UINT_T BlockNum)
{
	UINT_T i;
	P_FMProperties_T pFMProp = GetFMProperties();

	//block 0 is guaranteed by manufacture
	if(BlockNum == 0)
		return 0;

	//if no BBT is loaded, just return block num
	if(pFMProp->BBT_State == BBT_INVALID)
		return BlockNum;

	//If there is no RBBT, just return
	if(NULL == pFMProp->pRBBT)
		return BlockNum;

	//sanity check: is this a Marvell BBT that we are pointing to?
	if(pFMProp->pRBBT->Identifier != BBT_TYPE_MBBT)
		return BlockNum;

	// Has this block been relocated?	If so we'll need to use the relocated block
	for (i = 0; i < pFMProp->pRBBT->NumRelo; )
	{
		if (pFMProp->pRBBT->Entry[i].From == BlockNum)
		{
			BlockNum = pFMProp->pRBBT->Entry[i].To;
			i = 0; 	// Start over in case remapped block is now bad
			continue;
		}
		i++;
	}
	return BlockNum;
}

#endif

/*
 * RelocateBlock_Legacy
 */
UINT_T RelocateBlock_Legacy(UINT_T BlockNum, UINT_T* ReloBlock)
{
    UINT_T BlockRelocatedTo;
    UINT_T num_relo, BlkSize, initial_blk, end_blk, i;
    P_FMProperties_T pFMP = GetFMProperties();
    P_ReloTable_T pBBT = pFMP->pLBBT;
	P_FlashProperties_T pFlashProp = GetFlashProperties(BOOT_FLASH);

	//if our BBT is not valid
	if((pBBT == NULL) || ((UINT16)BBT_TYPE_LEGACY != pBBT->Header))
		return BBTReadError;

    //block 0 is guaranteed by manufacture: DO NOT RELOCATE IT
    if(BlockNum == 0)
	{
        *ReloBlock = 0;
        return NoError;
	}

	BlkSize = GetBlockSize(BOOT_FLASH);
	num_relo = pBBT->NumReloc;

	//get RP boundaries
    initial_blk = pFlashProp->NumBlocks - 1;
	end_blk = initial_blk - (pFlashProp->NumBlocks / 50); //2% of whole device

	//start at first RP block
	BlockRelocatedTo = initial_blk;

	//if the table is not empty, we need to find the next available RP block
	if (num_relo != 0)
	{
		//run through all the entries to find that smallest 'To' entry
		for(i = 0; i < pBBT->NumReloc; i++)
			if(BlockRelocatedTo > pBBT->Relo[i].To)
				BlockRelocatedTo = pBBT->Relo[i].To;

		//since we just found the last 'used' RP block, try the next one
		BlockRelocatedTo--;

		//now we have to make sure this block isn't marked factory bad [From X, To -1]
		i = 0;
		do {
			if(pBBT->Relo[i].From == BlockRelocatedTo)
			{	//this block is bad.
				BlockRelocatedTo--;	// try next one
				i = 0;				// restart search
				continue;
			}
			i++;
		} while(i < pBBT->NumReloc);

		//lastly, make sure we didn't exceed RP
		if(BlockRelocatedTo < end_blk)
			return BBTExhaustedError;
	}


    SetBBTState(BBT_CHANGED, BOOT_FLASH);
	*ReloBlock = (UINT) BlockRelocatedTo;
    // Relocate it
    pBBT->Relo[num_relo].To = BlockRelocatedTo;
    pBBT->Relo[num_relo].From = BlockNum;
    pBBT->NumReloc++;

    return NoError;
}

/* RelocateBlock_Marvell
 *
 */
#if NEW_MARVELL_BBM 
UINT_T RelocateBlock_Marvell(UINT_T BlockNum, UINT_T* ReloBlock)
{
    UINT_T BlockRelocatedTo, initial_blk, end_blk;
    UINT_T num_relo, BlkSize, i, next_blk;
	P_FMProperties_T pFMProp = GetFMProperties();

	//block 0 is guaranteed by manufacture: DO NOT RELOCATE IT
    if(BlockNum == 0)
	{
        *ReloBlock = 0;
        return NoError;
	}

	//if no BBT is loaded, just return block num
	if(pFMProp->BBT_State == BBT_INVALID)
		return BBTReadError;

	//is there a BBT for this partition?  If not, return error
	if(NULL == pFMProp->pRBBT)
		return BBTReadError;

	num_relo = pFMProp->pRBBT->NumRelo;
	BlkSize = GetBlockSize(BOOT_FLASH);

	//figure out where the RP starts and ends
	if(pFMProp->PT_state == PT_INVALID)
	{
		initial_blk = GetFlashProperties(BOOT_FLASH)->NumBlocks - 1;
		end_blk = initial_blk - (GetFlashProperties(BOOT_FLASH)->NumBlocks / 50); //2% of whole device
	}
	else
	{
		initial_blk = pFMProp->pPI->ReserveStartAddr / BlkSize;
		end_blk = pFMProp->pPI->ReserveSize / BlkSize; //get number of blocks in RP
		//from the initial block, either add or subtract total RP blocks.  The add/sub '1' is to account for adding rules.
		end_blk = (pFMProp->pPI->ReserveAlgorithm == PI_RP_ALGO_UPWD) ? initial_blk + end_blk - 1 : initial_blk - end_blk + 1;
	}

	//set at beginning of RP
	BlockRelocatedTo = initial_blk;
	//determine if 'next' block is +1 or -1
	next_blk = (initial_blk > end_blk) ? -1 : 1;

	//if this is not the first entry, need to calculate next available RP block
    if(num_relo != 0)
	{	//find the 'latest' TO block
		for (i = 0; i < pFMProp->pRBBT->NumRelo; i++)
			//RP grows towards beginning of device, find 'smallest' To entry
			//RP grows towards end of device, find 'largest' To entry
			if( ((initial_blk > end_blk) && (BlockRelocatedTo > pFMProp->pRBBT->Entry[i].To)) ||
				((initial_blk < end_blk) && (BlockRelocatedTo < pFMProp->pRBBT->Entry[i].To)) )
				BlockRelocatedTo = pFMProp->pRBBT->Entry[i].To;

		//try the next block (either +1 or -1, as determined previously)
		BlockRelocatedTo += next_blk;

		//now make sure this block isn't 'bad'
		i = 0;
		do
		{
			if (BlockRelocatedTo == pFMProp->pRBBT->Entry[i].From)
			{	//if this block is in the table, it is perminently bad.  try next one
				BlockRelocatedTo += next_blk;
				i = 0; //restart search
				continue;
			}
			i++;
		} while(i < pFMProp->pRBBT->NumRelo);

		//at this point, we have found a good block
		//check to ensure we haven't eclipsed the end of the RP
		if( ((initial_blk > end_blk) && (BlockRelocatedTo < end_blk)) ||	//RP 'under' run
			((initial_blk < end_blk) && (BlockRelocatedTo > end_blk)) ||	//RP 'over' run
			((initial_blk == end_blk) && (BlockRelocatedTo != end_blk)) )	//rare case of only 1 RP block
			return BBTExhaustedError;
	}

    SetBBTState(BBT_CHANGED, BOOT_FLASH);

    // Relocate it
    pFMProp->pRBBT->Entry[num_relo].To = BlockRelocatedTo;
    pFMProp->pRBBT->Entry[num_relo].From = BlockNum;
	*ReloBlock = BlockRelocatedTo;
    pFMProp->pRBBT->NumRelo++;

    return NoError;
}

#endif

/* FindBBT_Legacy
 *	This function searchs for the Legacy Bad Block Table (LBBT) within a block
 *   It will start by checking the first and last page for BBTs.  If both are found, it
 *   will load the one with the most relocations.  if only one is found, it will setup a
 *   search in that direction.  If neither are found, it will return with an error.
 *
 *	If a BBT is found, the FM structure will be loaded with the relevant BBT data.
 *
 * Inputs:
 *		Flash Boot Type
 * Outputs:
 *		None
 *
 *	NOTE:  This function will update the Flash Management Properties structure
 */
UINT_T FindBBT_Legacy(UINT_T BlockOffset, FlashBootType_T fbt)
{
	UINT_T Retval1, Retval2;
	UINT_T Slot, StartSlot, EndSlot;
	UINT_T PageSize, BlkSize, NumPages, id;
	UINT_T pBuffer[MAX_PAGE_SIZE_WORDS];
	P_ReloTable_T pTempBBT = (P_ReloTable_T)pBuffer;
	P_FMProperties_T pFMProp = GetFMProperties();
	P_ReloTable_T pBBT = pFMProp->pLBBT;

	//If this flash device does not use BBM, just return
	if(GetFlashProperties(fbt)->FlashSettings.UseBBM == 0)
		return NoError;

	// Now set some values that will work
	PageSize = GetPageSize(fbt);
	BlkSize = GetBlockSize(fbt);
	NumPages = BlkSize / PageSize;
	id = (UINT_T) BBT_TYPE_LEGACY;

	//we need to get a pointer of where to put the BBT
	//if we already had a BBT (from another partition), reuse the same pointer
	if(pFMProp->pLBBT != NULL)
		pBBT = pFMProp->pLBBT;
	else
	{//if we never had a BBT, get the next spot in the global space
		pBBT = pFMProp->pLBBT = (P_ReloTable_T)pFM_SPACE;
//		pFM_SPACE = (UINT*) ((UINT)pFM_SPACE + PageSize);
	}

	//invalidate state
	SetBBTState(BBT_INVALID, fbt);
	//null out, to ignore stale data
	pBBT->Header = NULL;
	//set the scan to Legacy BBTs.  If we dont' find a BBT, it'll get cleared below
	pFMProp->ScanBBT = &ScanBBT_Legacy;
	pFMProp->BBT_location = BlockOffset;

	if(BlockOffset == 0)
		//Find and read first page after the TIM
		Slot = PageSize >= MAX_TIM_SIZE_BYTES ? 1 : MAX_TIM_SIZE_BYTES / PageSize;
	else
		Slot = 0;

	//read the first slot
	Retval1 = ReadFlash((Slot * PageSize) + BlockOffset, (UINT_T)pBBT, PageSize, fbt);
	//read the last slot
	Retval2 = ReadFlash((BlkSize - PageSize) + BlockOffset, (UINT_T)pTempBBT, PageSize, fbt);

	//first slot contains BBT
	if((Retval1 == NoError) && (pBBT->Header == (USHORT)BBT_TYPE_LEGACY))
		if((Retval2 == NoError) && (pTempBBT->Header == (USHORT)BBT_TYPE_LEGACY))
		//start slot = good BBT    end slot = good BBT
		{	//figure out which one is "more" valid
			if(pBBT->NumReloc < pTempBBT->NumReloc)
			{	//grows upward
				memcpy(pBBT, pTempBBT, PageSize);
				pFMProp->BBT_Slot = NumPages - 1;
				pFMProp->BBT_NextSlot = pFMProp->BBT_Slot + 1;
				pFMProp->BBT_location += Slot * PageSize;
			}
			else
			{	//grows downward
				pFMProp->BBT_Slot = Slot;
				pFMProp->BBT_NextSlot = Slot; //can't subtract 1, since SLot could be 0
				pFMProp->BBT_location += (NumPages - 1) * PageSize;
			}
			SetBBTState(BBT_UNCHANGED, fbt);
			pFMProp->BBT_type = BBT_TYPE_LEGACY;
			//at this point, no need to search
			return NoError;
		}
		else //start slot = good BBT     end slot = no BBT
		{	//set search params for "upward" search
			StartSlot = Slot + 1;
			EndSlot = NumPages - 2;
			pFMProp->BBT_location += Slot * PageSize;
		}
	else //first slot does NOT contain BBT
		if((Retval2 == NoError) && (pTempBBT->Header == (USHORT)BBT_TYPE_LEGACY))
		//start slot = NO BBT     end slot = good BBT
		{	//move the found BBT into the allocated space
			memcpy(pBBT, pTempBBT, PageSize);
			//set search params for "downward" search
			StartSlot = NumPages - 2;
			EndSlot = Slot + 1;
			Slot = NumPages - 1; //remember where we found the first BBT
			pFMProp->BBT_location += Slot * PageSize;
		}
		else
		{	//if neither slot have a good BBT, clear out pointers and return error
			//pFMProp->ScanBBT = NULL;
			pFMProp->pLBBT = NULL;
			return BBTReadError;
		}
	// do the binary search
	Retval1 = BinarySearch((StartSlot * PageSize) + BlockOffset,
						 (EndSlot * PageSize) + BlockOffset,
						 (UINT_T)pTempBBT,
						 &Slot,
						 (UINT8_T*)&id,
						 2,
						 0,
						 fbt);

	//if binary search found a more recent BBT, copy it to the allocated space
	if(Retval1 == NoError)
		memcpy(pBBT, pTempBBT, PageSize);

	//set slot information
	pFMProp->BBT_Slot = Slot;
	pFMProp->BBT_NextSlot = StartSlot > EndSlot ? Slot - 1 : Slot + 1;

	SetBBTState(BBT_UNCHANGED, fbt);
	pFMProp->BBT_type = BBT_TYPE_LEGACY;
//	pFM_SPACE = (UINT*) ((UINT)pFM_SPACE + PageSize);

	return NoError;
}

/* FindBBT_Marvell
 *	This function searchs block 0 for the Bad Block Table (BBT).
 *  In order to handle the multiple schemes, the search thusly:
 *		Check the page after the TIM
 *		if found, binary search upwards for latest update
 *		else NOT found, check the last page in block 0
 *			if found, binary search downwards for latest update
 *			else no BBT found
 *
 * Inputs:
 *		Flash Boot Type
 * Outputs:
 *		None
 *
 *	NOTE:  This function will update the Flash Management Properties structure
 *			BBT will be loaded into global space
 *			BBT_State will be set accordingly
 *			BBT_Slot will be found
 */
#if NEW_MARVELL_BBM 
UINT_T FindBBT_Marvell(UINT_T BlockOffset, FlashBootType_T fbt)
{
	UINT_T Retval = NoError;
	UINT_T Slot, StartSlot, EndSlot;
	UINT_T PageSize, BlkSize, id;
	P_MBBT_Table_T pBBT;
	P_FMProperties_T pFMProp = GetFMProperties();

	//If this flash device does not use BBM, just return
	if(GetFlashProperties(fbt)->FlashSettings.UseBBM == 0)
		return NoError;

	// Now set some values that will work
	PageSize = GetPageSize(fbt);
	BlkSize = GetBlockSize(fbt);
	id = (UINT_T) BBT_TYPE_MBBT;

	//we need to get a pointer of where to put the BBT
	//if we already had a runtime BBT (from another partition), reuse the same pointer
	if(pFMProp->pRBBT != NULL)
		pBBT = pFMProp->pRBBT;
	else
	{//if we never had a runtime BBT, get the next spot in the global space
		pBBT = pFMProp->pRBBT = (P_MBBT_Table_T)pFM_SPACE;
		pFM_SPACE = (UINT*) ((UINT)pFM_SPACE + PageSize);
	}

	//invalidate state
	SetBBTState(BBT_INVALID, fbt);
	//null out, to ignore stale data
	pBBT->Identifier = NULL;

	Retval = BinarySearch(BlockOffset,
						 BlockOffset + BlkSize - 1,
						 (UINT_T)pBBT,
						 &pFMProp->BBT_Slot,
						 (UINT8_T*)&id,
						 4,
						 0,
						 fbt);

	//if binary search found a BBT, update state info
	if(Retval == NoError)
	{
		pFMProp->BBT_NextSlot = pFMProp->BBT_Slot + 1;
		SetBBTState(BBT_UNCHANGED, fbt);
		pFMProp->ScanBBT = &ScanBBT_Marvell;
		pFMProp->BBT_type = BBT_TYPE_MBBT;
		pFMProp->BBT_location = BlockOffset;
	}
	else //otherwise, clear out state info
	{
		pFMProp->pRBBT = NULL;
		pFMProp->ScanBBT = NULL;
		pFM_SPACE = (UINT*) ((UINT)pFM_SPACE - PageSize); //set pointer back down a page
	}

	return Retval;

}
#endif

/* Binary Search Function
 *	This function searchs within a flash block for the "last" page with a matching MagicNumber at
 *	an offset into the page as specified by the MagicNumOffset input.  The page is then loaded
 *	into memory at the address specified by the buffer pointer input.  The direction
 *	of the search is determined by the values of the StartAddr and EndAddr inputs.
 *
 *		Example
 *
 * Inputs:
 *		StartAddr:  Absolute address of first possible entry location
 *		EndAddr: Absolute address of the last possible entry location
 *		MagicNumber: Pointer to a value to be checked when searching the block
 *		MagicNumSize: Size, in bytes, of the magic word.
 *		MagicNumOffset: Byte offset into the page at with to check for the MagicNumber
 *		fbt: Flash Boot Type that we are to search
 * Outputs:
 *		buffer: Pointer to where the page will be loaded
 *		SlotNum: Page number of the last found page
 *
 * Return Value:
 *		NoError			if page found
 *		SlotNotFound	if no valid page found
 *
 *	NOTE:  buffer MUST be allocated to a full flash Page, or else data corruption will occur
 */
UINT_T BinarySearch(UINT_T StartAddr, UINT_T EndAddr, UINT_T buffer, UINT_T* SlotNum,
					UINT8_T *pMagicNumber, UINT_T MagicNumSize, UINT_T MagicNumOffset, FlashBootType_T fbt)
{
	UINT_T Retval;
	UINT_T PageSize, BlkSize, BlkOffset, CurrentPageOffset;
	INT_T MaxSlot, MinSlot, CurrentSlot, LastFoundSlot; //page number inside the block
	UINT8_T direction; // 0 = up, 1 = down
	UINT8_T found = 0; // used on each page check: 0 = not found, 1 = found

	// Now set some values that will work
	PageSize = GetPageSize(fbt);
	BlkSize = GetBlockSize(fbt);
	BlkOffset = StartAddr & ~(BlkSize - 1);
	//chop off the block address, since we're only searching WITHIN a block
	StartAddr &= (BlkSize - 1);
	EndAddr &= (BlkSize - 1);
	//set this to an invalid number.  At the end, it will tell us if we found a good slot
	LastFoundSlot = -1;

	if(StartAddr > EndAddr)
	{
		MaxSlot = StartAddr / PageSize;
		MinSlot = EndAddr / PageSize;
		CurrentSlot = MaxSlot;
		direction = 1; //search downward
	}
	else
	{
		MaxSlot = EndAddr / PageSize;
		MinSlot = StartAddr / PageSize;
		CurrentSlot = MinSlot;
		direction = 0; //search upward
	}

	//Now we must locate the last used slot.  Do this by searching for the magic number.
	// We start at the boundary (to minimize search time) and do a binary search to locate the last used slot.
	// Note: found variable will be cleared on each iteration
	for(;MinSlot <= MaxSlot; found = 0)
	{
		//get the flash address to read
		CurrentPageOffset = ((UINT_T)CurrentSlot * PageSize) + BlkOffset;

		Retval = ReadFlash(CurrentPageOffset, buffer, PageSize, fbt);

		// Did we find a valid entry?
		if((Retval == NoError) && (memcmp(((void *)(buffer + MagicNumOffset)), pMagicNumber, MagicNumSize) == 0))
		{
			found = 1;
			//save the slot number
			LastFoundSlot = CurrentSlot;
		}

		//if: searching upward and DO NOT find a slot -OR- searching downward and find a slot
		//		lower the max slot
		//else: searching upward and find a slot -OR- downward and DO NOT find a slot
		//		raise minimum slot
		if (found == direction)
		//Set new Max slot to the next page below
			MaxSlot = CurrentSlot - 1;
		else
		//Set new Min slot to the next page above
			MinSlot = CurrentSlot + 1;

		//adjust current slot to midpoint of max and min to get next offset
		CurrentSlot = (MaxSlot + MinSlot) >> 1;
	} // End while

	//Check to make sure we even found a good slot
	if(LastFoundSlot == -1)
		Retval = NotFoundError;
	else
	{
		*SlotNum = (UINT_T)LastFoundSlot;
		//The last valid slot probably got overwritten during the search
		//So we need to re-read the last valid entry in the buffer
		CurrentPageOffset = ((UINT_T)LastFoundSlot * PageSize) + BlkOffset;
		Retval = ReadFlash(CurrentPageOffset, buffer, PageSize, fbt);
	}
	return Retval;
}


