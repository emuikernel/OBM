/******************************************************************************
**  COPYRIGHT  2007 Marvell Inernational Ltd.
**  All Rights Reserved
******************************************************************************/

#ifndef __FM_H__
#define __FM_H__

#include "Typedef.h"
#include "Flash.h"
#include "BBM.h"
#include "Partition.h"

//Flash Management (FM) structure contains:
typedef struct
{
	/* Pointers to all possible fields */
	//Partition Table
	P_PartitionTable_T pMPT;
	P_PartitionInfo_T pPI;
	//Factory BBT
	P_FBBT_Table_T pFBBT;
	//Runtime BBT - Marvell BBT version
	P_MBBT_Table_T pRBBT;
	//Legacy BBT
 	P_ReloTable_T pLBBT;

	//information about the parition table
	PTState_T 	PT_state;
	UINT_T		PT_location;	//location of where the PT was found/where to put it (in flash)
	UINT_T		PI_num;			//which partition the PI struct relates to

	//information about the current BBT (either Legacy or Runtime)
	ReloState_T	BBT_State;
	BBT_TYPE	BBT_type;
	UINT_T		BBT_location;	// this is the location of where the FIRST bbt is to be written
	UINT_T		BBT_Slot;		// Indicates which page in Block BBT located
	UINT_T		BBT_NextSlot;	// Indicates which page a new BBT should be burn into

	//other info
	P_ReadDisturbList_T pReadDisturbList;

	//BBT function pointers - links to either legacy or Marvell BBT functions
	UINT_T (*FindBBT) (UINT_T BlockOffset, FlashBootType_T);
	UINT_T (*ScanBBT) (UINT_T BlockNum);
	UINT_T (*RelocateBlock) (UINT_T BlockNum, FlashBootType_T);
}
FMProperties_T, *P_FMProperties_T;

//Flash Management methods
typedef enum
{
	MARVELL_METHOD	= 1,
	LEGACY_METHOD	= 2,
	ALL_METHODS		= 0xFF
} FM_Method_T;

//Function Prototypes
P_FMProperties_T GetFMProperties();

void InitializeFM(FM_Method_T FM_type, FlashBootType_T fbt);
void FinalizeFM(FlashBootType_T fbt);
void ClearFM(FlashBootType_T fbt);
UINT_T ScanBootBlock(FM_Method_T FM_type, FlashBootType_T fbt);

UINT_T LocateBlock(UINT_T BlockNum, FlashBootType_T fbt);
UINT_T RelocateBlock(UINT_T BlockNum, UINT_T* NewBlock, FlashBootType_T fbt);
void LoadExternalPartitionTable(UINT_T *table, UINT_T location);
void SetPartition(UINT_T PartitionNum, FlashBootType_T fbt);
UINT_T GetPartitionOffset(FlashBootType_T fbt);

void CheckFMEraseState(UINT_T BlockNum, FlashBootType_T fbt);
void SetBBTState(ReloState_T newState, FlashBootType_T fbt);

void UpdateBBT();
void CreateBBT_Legacy(P_PartitionInfo_T pPI);
void CreateBBT_Marvell(P_PartitionInfo_T pPI);
UINT_T ScanBBT_Legacy(UINT_T BlockNum);
UINT_T ScanBBT_Marvell(UINT_T BlockNum);
UINT_T RelocateBlock_Legacy(UINT_T BlockNum, UINT_T* ReloBlk);
UINT_T RelocateBlock_Marvell(UINT_T BlockNum, UINT_T* ReloBlk);
UINT_T FindBBT_Legacy(UINT_T BlockOffset, FlashBootType_T fbt);
UINT_T FindBBT_Marvell(UINT_T BlockOffset, FlashBootType_T fbt);

UINT_T BinarySearch(UINT_T StartAddr, UINT_T EndAddr, UINT_T buffer, UINT_T* SlotNum, 
					UINT8_T *pMagicNumber, UINT_T MagicNumSize, UINT_T MagicNumOffset, FlashBootType_T fbt);

P_ReadDisturbList_T CreateReadDisturbList();
unsigned int AddReadDisturbEntry(unsigned int partition, unsigned int block);
void ImportReadDisturbList(unsigned int import_list);



#endif


