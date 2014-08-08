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


#include "tr069.h"

extern UINT_T NandID;
extern SkipBlocksInfoStruct *pSkipAddress;
extern UINT_T All_FF_flag;

UINT_T upgrade_failed_flag = 0;

LZMA2_Image_List LZMA2_IL;
P_LZMA2_Image_List pLIL = &LZMA2_IL;

#if DECOMPRESS_SUPPORT
UINT_T Image_Process(P_TR069_Firmware pTR069_T, UINT_T flash_address, UINT_T size)
{
	UINT_T Retval = GeneralError;
	UINT_T LoadAddr = DDR_DOWNLOAD_AREA_ADDR;
    UINT_T decompressLength;
	UINT_T LZMA2_Erase_Size = 0;

	serial_outstr("Image_Process flash_address\n");
	serial_outnum(flash_address);
	serial_outstr("\n");
	serial_outstr("Image_Process size\n");
	serial_outnum(size);
	serial_outstr("\n");

	if ((pTR069_T->Upgrade_Method == 1) || (pTR069_T->Upgrade_Method == 2))
	{
		LoadAddr += pTR069_T->FBF_Size;
	}
	
	Retval = ReadFlash(flash_address, LoadAddr, size, BOOT_FLASH);
	if (Retval != NoError)
		return Retval;

	decompressLength = DECOMPRESS_LENGTH;
	Retval = LZMA_Decompress((UINT_T *)DECOMPRESS_TEMP_ADDR,
							&decompressLength,
							LoadAddr,
							size);

	if (Retval != NoError)
		return Retval;
		
	if (size > decompressLength)
		LZMA2_Erase_Size = size;
	else
		LZMA2_Erase_Size = decompressLength;
	
    Retval = EraseFlash(flash_address,
						LZMA2_Erase_Size,
						BOOT_FLASH );
	
    if(Retval != NoError )
        return Retval;

	Retval = WriteFlash(flash_address,
					DECOMPRESS_TEMP_ADDR,
					decompressLength, BOOT_FLASH );

	if(Retval != NoError )
        return Retval;

	serial_outstr("Image_Process Done\n");

	return NoError;
}
#endif

UINT_T SD_TR069_Upgrade(P_TR069_Firmware pTR069_T)
{
	UINT_T Retval = GeneralError;
	UINT_T Buffer = DDR_TEMP_ADDR;
	UINT_T LoadAddr = DDR_DOWNLOAD_AREA_ADDR;
	MasterBlockHeader *pMasterHeader = NULL;
	PDeviceHeader_11 pDevHeader_11=NULL;
	PImageStruct_11 pImage_11 = NULL;
    UINT_T imagenum, i, skip_blocks, decompressLength;
    UINT_T imageType;
    UINT_T temp_p = NULL;
	UINT_T NandID_Right;
	UINT_T imageChecksum = 0;
	
	Retval = ReadFlash(pTR069_T->FBF_Flash_Address, LoadAddr, pTR069_T->FBF_Size, BOOT_FLASH);
	if (Retval != NoError)
		return Retval;

	pMasterHeader = (MasterBlockHeader *)LoadAddr;

	if (pMasterHeader->Format_Version != 11)
	    return FBF_VersionNotMatch;
	

	if (pMasterHeader->nOfDevices != 1)
	    return FBF_DeviceMoreThanOne;

	temp_p = pMasterHeader->deviceHeaderOffset[0] + LoadAddr;
	pDevHeader_11 = (PDeviceHeader_11)temp_p;

	// check nand ID
#if NAND_CODE
	NandID_Right = pDevHeader_11->FlashOpt.NandID;
	if (NandID != NandID_Right)
		return NANDIDDISMATCH;
#endif

	// skip blocks
	skip_blocks = pDevHeader_11->FlashOpt.Skip_Blocks_Struct.Total_Number_Of_SkipBlocks;
	if (skip_blocks > 0)
	{
		pSkipAddress = &pDevHeader_11->FlashOpt.Skip_Blocks_Struct;
	}
	else
		pSkipAddress = NULL;

	for ( imagenum = 0; imagenum < pDevHeader_11->nOfImages; imagenum++ )
	{
	    temp_p = (UINT_T)&pDevHeader_11->imageStruct_11[imagenum];
	    pImage_11 = (PImageStruct_11)temp_p;
				
		// add checksum for images in FBF file
		imageChecksum = CalcImageChecksum((UINT_T*)(LoadAddr+(pImage_11->First_Sector<<13)), pImage_11->length);
		if (imageChecksum != pImage_11->ChecksumFormatVersion2)
			return CRCFailedError;
		
	    imageType = IMAGE_TYPE(pImage_11->commands);
	    if ( imageType == DLCMD_YAFFS_IMAGE_TYPE)
	    {    
	        SetUseSpareArea( TRUE, BOOT_FLASH );
	        All_FF_flag = 1;
	    }
	    else if(imageType == DLCMD_UBIFS_IMAGE_TYPE)
	    {
	        SetUseSpareArea( FALSE, BOOT_FLASH);
	        All_FF_flag = 1;
	    }
	    else
		{
	        SetUseSpareArea( FALSE, BOOT_FLASH );
	        All_FF_flag = 0;
	    }

        Retval = EraseFlash(pImage_11->Flash_Start_Address,
							pImage_11->Flash_erase_size,
							BOOT_FLASH );
		
        if(Retval != NoError )
            return Retval;

		if ((pImage_11->commands & 0xF) == 2) //erase only
			continue;
		
    	Retval = WriteFlash( pImage_11->Flash_Start_Address,
						LoadAddr+(pImage_11->First_Sector<<13),
						pImage_11->length, BOOT_FLASH );

		if (Retval != NoError )
			return Retval;
	}

	#if DECOMPRESS_SUPPORT
	memset(pLIL, 0, sizeof(LZMA2_Image_List));
	i = 0;

	for ( imagenum = 0; imagenum < pDevHeader_11->nOfImages; imagenum++ )
	{
		temp_p = (UINT_T)&pDevHeader_11->imageStruct_11[imagenum];
	    pImage_11 = (PImageStruct_11)temp_p;

		imageType = IMAGE_TYPE(pImage_11->commands);
		if ( imageType == DLCMD_LZMA2_IMAGE_TYPE)
		{
			if (i == 0)
			{
				pLIL->LZMA2_List[i].Image_Address = pImage_11->Flash_Start_Address;
				pLIL->LZMA2_List[i].Image_Size = pImage_11->length;
				pLIL->LZMA2_List[i].Image_ID = pImage_11->Image_ID;
			}
			else
			{
				if (pLIL->LZMA2_List[i - 1].Image_ID == pImage_11->Image_ID)
				{
					// the flash address in FBF is decreased progressively,
					// so, we only record the last flash address
					pLIL->LZMA2_List[i].Image_Address = pImage_11->Flash_Start_Address;

					// combine image length
					pLIL->LZMA2_List[i].Image_Size = pLIL->LZMA2_List[i - 1].Image_Size + pImage_11->length;

					// record image ID
					pLIL->LZMA2_List[i].Image_ID = pImage_11->Image_ID;
				}
				else
				{
	
					Retval = Image_Process(pTR069_T,
									pLIL->LZMA2_List[i - 1].Image_Address,
									pLIL->LZMA2_List[i - 1].Image_Size);

					if (Retval != NoError)
						return Retval;

					// record the info of new LZMA2 image
					pLIL->LZMA2_List[i].Image_Address = pImage_11->Flash_Start_Address;
					pLIL->LZMA2_List[i].Image_Size = pImage_11->length;
					pLIL->LZMA2_List[i].Image_ID = pImage_11->Image_ID;
				}
			}

			i++;
		}
		else
		{
			if ((i > 0) && (pLIL->LZMA2_List[i - 1].Image_ID != 0))
			{
				Retval = Image_Process(pTR069_T,
									pLIL->LZMA2_List[i - 1].Image_Address,
									pLIL->LZMA2_List[i - 1].Image_Size);

					if (Retval != NoError)
						return Retval;

				i = 0; // clear loop
			}
		}
	}

	// process the last LZMA2 image
	if ((i > 0) && (pLIL->LZMA2_List[i - 1].Image_ID != 0))
	{
		Retval = Image_Process(pTR069_T,
								pLIL->LZMA2_List[i].Image_Address,
								pLIL->LZMA2_List[i].Image_Size);

		if (Retval != NoError)
			return Retval;
	}
	#endif

	if (pTR069_T->Erase_PSM)
	{
		serial_outstr("Erase_PSM\n");
		Retval = EraseFlash(pTR069_T->PSM_Erase_Address, pTR069_T->PSM_Erase_Size, BOOT_FLASH);
		if (Retval != NoError)
			return Retval;
	}

	if (pTR069_T->Erase_FS)
	{
		serial_outstr("Erase_FS\n");
		Retval = EraseFlash(pTR069_T->FS_Erase_Address, pTR069_T->FS_Erase_Size, BOOT_FLASH);
		if (Retval != NoError)
			return Retval;
	}

	return NoError;
}

UINT_T WebUI_Upgrade(P_TR069_Firmware pTR069_T)
{
	UINT_T Retval = GeneralError;

	#if DECOMPRESS_SUPPORT
	if (pTR069_T->Webdata_LZMA2)
	{
		Retval = Image_Process(pTR069_T, pTR069_T->Webdata_Address, pTR069_T->Webdata_Size);
		
		if(Retval != NoError )
            return Retval;
	}

	if (pTR069_T->WIFI_LAMA2)
	{
		Retval = Image_Process(pTR069_T, pTR069_T->WIFI_Address, pTR069_T->WIFI_Size);
		
		if(Retval != NoError )
            return Retval;
	}

	if (pTR069_T->WIFI_NoCal_LZMA2)
	{
		Retval = Image_Process(pTR069_T, pTR069_T->WIFI_NoCal_Address, pTR069_T->WIFI_NoCal_Size);
		
		if(Retval != NoError )
            return Retval;
	}
	#endif

	return NoError;
}

UINT_T firmware_upgrade(pTIM pTIM_h)
{
	UINT_T Retval = GeneralError;
	UINT_T TR069_Addr = FBF_HEADER_FLASH_ADDRESS;
	UINT_T TR069_FBF_Flag = FBF_HEADER_IDENTIFIER;
	UINT_T Buffer = DDR_TEMP_ADDR;
	P_TR069_Firmware pTR069_T = NULL;
    UINT_T temp_p = NULL;
	pWTP_RESERVED_AREA_HEADER pWRAH = NULL;
	pTR069_INFO pTR = NULL;

	pWRAH = FindPackageInReserved(&Retval, pTIM_h, TR069INFOID);
	if ((pWRAH == NULL) || (Retval != NoError))
	{
		serial_outstr("TR069 Not Supported\n");
		return NotSupportedError; // no TR069_INFO find, just return
	}
	else
	{
		pTR = (pTR069_INFO) pWRAH;
		if (pTR->Enable != 1)
		{
			serial_outstr("TR069 Not Enabled\n");
			return NotEnabledError; // TR069_INFO find, but not enabled, just return
		}
		else
		{
			TR069_Addr = pTR->FlashAddr;
			TR069_FBF_Flag = pTR->Magic;
			serial_outstr("pTR->FlashAddr\n");
			serial_outnum(TR069_Addr);
			serial_outstr("\n");
			serial_outstr("pTR->Magic\n");
			serial_outnum(TR069_FBF_Flag);
			serial_outstr("\n");
		}
	}

	Retval = ReadFlash(TR069_Addr, Buffer, sizeof(TR069_Firmware), BOOT_FLASH);
	if (Retval != NoError)
		return Retval;

	pTR069_T = (P_TR069_Firmware)Buffer;
	if ((pTR069_T->Header != TR069_FBF_Flag) || (pTR069_T->Upgrade_Flag == 0))
		return NotFoundError;

	if (pTR069_T->Upgrade_Flag == 2)
	{
		serial_outstr("Upgrade Failed Flag Detected\n");
		upgrade_failed_flag = 1; // boot to backup system
		return NotSupportedError;
	}

	#if OLED_SUPPORT || LED_DISPLAY
	Firmware_Upgrade_Start();
	#endif

	switch (pTR069_T->Upgrade_Method)
	{
		case 0: // TR069
		case 2: // SD
			serial_outstr("TR069/SD upgrade\n");
			Retval = SD_TR069_Upgrade(pTR069_T);
			break;

		case 3: // WebUI
			serial_outstr("WebUI upgrade\n");
			Retval = WebUI_Upgrade(pTR069_T);
			break;

		default:
			serial_outstr("Unspported upgrade method\n");
			serial_outnum(pTR069_T->Upgrade_Method);
			serial_outstr("\n");
			Retval =  NotSupportedError;
			break;
	}

	if (Retval != NoError)
		return Retval;

		// clear TR069 upgrade flag
	pTR069_T->Upgrade_Flag = 0;
	Retval = EraseFlash(TR069_Addr, sizeof(TR069_Firmware), BOOT_FLASH);
	if (Retval != NoError)
		return Retval;

	Retval = WriteFlash(TR069_Addr, Buffer, sizeof(TR069_Firmware), BOOT_FLASH);
	if (Retval != NoError)
		return Retval;

	serial_outstr("Clear upgrade flag OK\n");

	SetUseSpareArea( FALSE, BOOT_FLASH );
	UpdateBBT();

	#if SPI_CODE
	Giga_Disable4BytesMode();
	#endif

	#if OLED_SUPPORT || LED_DISPLAY
	Firmware_Upgrade_Done();
	#endif

	return NoError;
}
 
