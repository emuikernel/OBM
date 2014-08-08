/******************************************************************************
**
**  COPYRIGHT (C) 2002, 2003 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be construed as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document.
**  Except as permitted by such license, no part of this document may be
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation.
**
**  FILENAME:   Flash.c
**
**  PURPOSE:    Contain template OEM boot code flash operations
**
******************************************************************************/

#include "Flash.h"
#include "nand.h"
#include "general.h"
#include "xllp_dfc.h"
#include "FM.h"
#include "ProtocolManager.h"
#if COPYIMAGESTOFLASH
#include "BootLoader.h"
#endif
#if USE_DMA
#include "xllp_dmac.h"
#endif

NAND_Properties_T NAND_Prop;    // Only need one
extern UINT_T NandID;
extern UINT_T upload_nand_spare;

P_NAND_Properties_T GetNANDProperties(void)
{
    return &NAND_Prop;
}

/*
 * Function initializes NAND device and fills out Flash Properties Struct
 */

UINT_T InitializeNANDDevice(UINT8_T FlashNum, FlashBootType_T FlashBootType, UINT8_T* P_DefaultPartitionNum)
{
    UINT_T Retval = NoError;
    P_NAND_Properties_T pNandP = GetNANDProperties();
    P_FlashProperties_T pFlashP = GetFlashProperties(FlashBootType);

    // Set Current FlashBootType so XllpDfcInit can determine why it is called.
    SetCurrentFlashBootType(FlashBootType);

	// Pick correct bus width and set the ECC type
	switch (FlashNum)
	{
		case NAND_FLASH_X16_HM_P:
			pNandP->FlashBusWidth = FlashBusWidth16;
			pNandP->ECCMode = ECC_HAMMING;
			break;
		case NAND_FLASH_X16_BCH_P:
			pNandP->FlashBusWidth = FlashBusWidth16;
			pNandP->ECCMode = ECC_BCH;
			break;
		case NAND_FLASH_X8_HM_P:
			pNandP->FlashBusWidth = FlashBusWidth8;
			pNandP->ECCMode = ECC_HAMMING;
			break;
		case NAND_FLASH_X8_BCH_P:
			pNandP->FlashBusWidth = FlashBusWidth8;
			pNandP->ECCMode = ECC_BCH;
			break;
		default:
			return DFCInitFailed;
	}

	PlatformNandClocksEnable();
		
    Retval = XllpDfcInit(pNandP->FlashBusWidth, pNandP);

    if (Retval != NoError)
        //return DFCInitFailed;
        return NANDNotFound;

	//set ECC with the correct ECCMode
	xdfc_enable_ecc( 1 );
    //define functions
    pFlashP->ReadFromFlash = &ReadNAND;
    pFlashP->WriteToFlash = &WriteNAND;
    pFlashP->EraseFlash =  &EraseNAND;
    pFlashP->ResetFlash = &ResetNAND;
    pFlashP->BlockSize = pNandP->BlockSize;
    pFlashP->PageSize = pNandP->PageSize;
	pFlashP->NumBlocks = pNandP->NumOfBlocks;
    pFlashP->FlashSettings.UseBBM = 1;
    pFlashP->FlashSettings.UseSpareArea = 0;
    pFlashP->FlashSettings.SASize = 0;
    pFlashP->FlashSettings.UseHwEcc = 1;  // default is HW ECC ON
    pFlashP->FlashType = NAND_FLASH;
    // init the TIM load address
    //---------------------------------------
    pFlashP->TimFlashAddress = TIMOffset_NAND;
	pFlashP->StreamingFlash = FALSE;
    *P_DefaultPartitionNum = NAND_DEFAULT_PART;
	pFlashP->FinalizeFlash = NULL;

	NandID = (pNandP->FlashID << 8) | pNandP->ManufacturerCode;

    return Retval;
}

/*
 This routine will read in data from the DFC.
*/
UINT_T ReadNAND_nonDMA(UINT_T Address, UINT_T Destination, UINT_T ReadAmount, FlashBootType_T FlashBootType)
{
    P_NAND_Properties_T pNAND_Prop;
    pNAND_Prop = GetNANDProperties();

	UINT_T temp_buffer[2]; // temp use, no data needed

    // For monolithic operation call legacy read routine
    if (pNAND_Prop->PageSize <= TWO_KB)
    {
    	if (upload_nand_spare == TRUE)
    	{
    		return xdfc_read_nonDMA((UINT_T *)Destination, Address, ReadAmount, temp_buffer, pNAND_Prop);
    	}
		else
		{
        	return xdfc_read_nonDMA((UINT_T *)Destination, Address, ReadAmount, NULL, pNAND_Prop);
		}
    }
    else{    // Call the new routine that reads page at a time.
#if NAND_LP
		return xdfc_read_LP((UINT_T *)Destination, Address, ReadAmount, NULL, pNAND_Prop);
#else
		return NoError;
#endif
    }
}

UINT_T ReadNAND (UINT_T Address, UINT_T Destination, UINT_T ReadAmount, FlashBootType_T FlashBootType)
{
	return xdfc_read(Destination, Address, ReadAmount, NULL, GetNANDProperties());
}

/*
    WriteNAND - This function is a wrapper to work with DFC
 */

UINT_T WriteNAND (UINT_T flash_addr, UINT_T source, UINT_T WriteAmount, FlashBootType_T FlashBootType)
{
    UINT_T Retval = NoError;
    P_NAND_Properties_T pNandP = GetNANDProperties();
    P_FlashProperties_T pFlashP = GetFlashProperties(FlashBootType);
    if (pNandP->PageSize <= TWO_KB)
    {
        Retval = xdfc_write((UINT_T *) source,
                        flash_addr,
                        WriteAmount,
                        pFlashP->FlashSettings.UseSpareArea,
                        pFlashP->FlashSettings.UseHwEcc,
                        pNandP);
    }
    else
    {
#if NAND_LP    
        Retval = xdfc_write_LP((UINT_T *) source,
                            flash_addr,
                            WriteAmount,
							pFlashP->FlashSettings.UseSpareArea,
                            pNandP);
#endif
	}
    return Retval;  // Return value
}

/*
 *
 *  Erases one block at a time
 *      note:  if user inputs determine that a partial block should be erased,
 *              this function will erase that WHOLE block: no partial blocks allowed
 *
 */
UINT_T EraseNAND (UINT_T flashoffset, UINT_T size, FlashBootType_T fbt)
{
    return xdfc_erase(flashoffset, GetNANDProperties());
}

//Wrapper to link the dfc reset routine to the flash API
UINT_T ResetNAND(FlashBootType_T fbt)
{
    return xdfc_reset(GetNANDProperties());
}


