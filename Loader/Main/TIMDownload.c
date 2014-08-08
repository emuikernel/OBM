
/******************************************************************************
 *
 *  (C)Copyright 2011 Marvell. All Rights Reserved.
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
 *  FILENAME:	TIMDownload.c
 *
 *  PURPOSE: 	Contains BootLoader's TIM based download code.   
 *
******************************************************************************/


//////////////////////////////////////////////////////////////////////
// Library Support for the TIMDownload.c
// Include any shared libraries in the TIMDownload.h file to keep this 
// code clean. 
//////////////////////////////////////////////////////////////////////
#include "TIMDownload.h"

UINT_T NandID = 0x0; // make sure the nand ID OBM detected is the equals to the nand ID OBM got from SWD
UINT_T Reset_flag = 1;
UINT_T Flash_size = 0x01000000; // default 16MB

//////////////////////////////////////////////////////////////////////
// This is the entry point for the TIM based download functionality. 
//
// Inputs: Pointer to the TIM we just downloaded.  
// Outputs: Returns the next image (OBM) that we will transfer the control to. 
//
// It mainly functions as following:
//		1) Sets the TIM pointers.
//		2) Validates the downloaded TIM.
// 		3) Configures the Flashes.
//		4) Wipes the whole Flash device.
//		5) Checks if there is already a FBBT on the Flash. If there is
//		   NO FBBT on the Flash, creates it. 
//		6) Checks if there is a PT being downloaded. If so, it first 
//		   tries to validate it with the existing HW partitions on the 
//         Flash. If they don't match, re-creates the HW partitions.
//         If HW partitions are not supported, these functions will do 
//         nothing and return. Next, the downloaded PT is loaded into
//         Flash Management (FM).
//		7) Runtime-BBT is generated.
//		8) Writes the downloaded TIM, created FBBT, PT to Flash. 
//		9) Download TIM Images LOOP implements the following:
//			i. Download Image
//			ii. Validate Image
//			iii. Set Partition
//			iv. Erase Image Area from Flash
//			v. Write the image to Flash
//			vi. Verify the Flash write
//		10) The RBBT is written to the Flash. 
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 TIMDownloadMain( pTIM pTIM_h )
{   		        
	pIMAGE_INFO_3_4_0 pImageInfo = NULL;
	UINT_T Retval = NoError;
	UINT8_T FlashNumber = 0;
	//UINT_T buff[256];

    // setup the TIM pointers in pTIM_h
    Retval = LoadTim((UINT8_T*)pTIM_h->pConsTIM, pTIM_h, TRUE);
    if (Retval != NoError)
    {
    	FatalError(Retval);
    }
			
	// Determine the Flash that will be used to store the downloaded images.
	FlashNumber = (pTIM_h->pConsTIM->FlashInfo.BootFlashSign) & 0xFF;
	
	Retval = Configure_Flashes (FlashNumber, BOOT_FLASH);
	if( Retval != NoError)
	   FatalError(Retval);
	
	//turn on the flash management
	InitializeFM(LEGACY_METHOD, BOOT_FLASH);

	//memset(buff, 0, sizeof(buff));
	//xdfc_Read_UID(buff);
	
    // At this point, we can start downloading images one by one. 
    // IMPORTANT: For now, we download all the images to their load address that is 
    // specified in the TIM. We assume that the images will not overwrite the existing
    // DKB code that is already running. 
    //
    // KDA TO DO: 
    // Potentially, we might want to download all the images to a specific 
    // DOWNLOAD_AREAD in the DDR.  
    pImageInfo = DownloadTIMImages(pTIM_h);
    
    return pImageInfo;
}


SkipBlocksInfoStruct *pSkipAddress = NULL;
UINT_T All_FF_flag = 0;

//////////////////////////////////////////////////////////////////////
// This is the actual image downloading function for the TIM based 
// download mode. 
//
// Inputs: TIM which includes a list of all the images that will be 
// 		   downloaded. 
// Outputs: Returns the OBM image to whom we will trasfer control to. 
//
// It mainly iterates through all the images listed in the TIM and 
//		1) If NOT TIM, downloads the image.
//		2) Validates the image.
//		3) Calls the Erase and Write function for this image.
//		4) Finally, finds the OBM image in the TIM and returns it.   
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 DownloadTIMImages (pTIM pTIM_h)
{
	UINT_T Retval = NoError;
	FUNC_STATUS FS_Retval;
	UINT_T ImageIndex, ImageSize;
	pIMAGE_INFO_3_4_0 pImageInfo = NULL;
	UINT_T LoadAddress;
	UINT_T NumImages;
	unsigned long       loadAddr;
    int         OKtoErase = 1;
	UINT8_T     FlashNumber = (pTIM_h->pConsTIM->FlashInfo.BootFlashSign) & 0xFF;

	#if FBF_NEW
    MasterBlockHeader* pMasterHeader = NULL;
	PDeviceHeader_11 pDevHeader_11=NULL;
	PImageStruct_11 pImage_11 = NULL;
    UINT_T imagenum, i, skip_blocks;
    UINT_T imageType;
    UINT_T temp_p = NULL;
	UINT_T PartitionNumber=0;
	UINT_T NandID_Right;
	UINT8_T option = 1;
	UINT8_T productMode = 0;

	UINT_T imageChecksum = 0;
    #endif

	#if I2C && FAN_54013
	DisableCharger();
	#endif

	// Depending on the version of the TIM, determine the size of each image in bytes. 
	// We will use this size to iterate through the TIM binary from image to image. 
	if (pTIM_h->pConsTIM->VersionBind.Version >= TIM_3_4_00)
		ImageSize = sizeof(IMAGE_INFO_3_4_0);
	else
		ImageSize = sizeof(IMAGE_INFO_3_2_0);

	// This is the very initial TIM image! Assumes that the TIM image is located at the top of the 
	// TIM file. Otherwise, we would skip images. 
	NumImages	= pTIM_h->pConsTIM->NumImages;
	pImageInfo	= (IMAGE_INFO_3_4_0*)((unsigned char*)pTIM_h->pImg + (NumImages-1)*ImageSize);  // index to last image info record.

	// For each image, download, validate and copy
    while (NumImages && (Retval == NoError))
    {
        if (NumImages > 1)
        {
            loadAddr = DDR_DOWNLOAD_AREA_ADDR;
            FS_Retval = HandleRequest(loadAddr, pImageInfo->ImageID);
            Retval = FS_Retval.ErrorCode;
			if(Retval == UnknownImageError)
			{
				Retval = NoError;
				goto nextimage;
			}
            if (Retval != NoError)
				FatalError(Retval);

        }
        else loadAddr = (unsigned long)((UINT8_T*)pTIM_h->pConsTIM);    // first image, TIM. use the address provided by caller.

        #if COPYIMAGESTOFLASH
		if(pImageInfo->FlashEntryAddr == 0xFFFFFFFF)
			goto nextimage;
		
        // Copy the Image
        AddMessageError(REPORT_NOTIFICATION, PlatformBusy);
		
        #if FBF_NEW		
		
        if ((pImageInfo->ImageID & 0xFFFFFF00) == FBFIDENTIFIER0)
        {
	        pMasterHeader = (MasterBlockHeader*)loadAddr;

	        if ((pMasterHeader->Format_Version != 11))
	        {
	            FatalError(FBF_VersionNotMatch);
	        }
			
	        if (pMasterHeader->nOfDevices != 1)
	        {
	            FatalError(FBF_DeviceMoreThanOne);
	        }
			
	        temp_p = pMasterHeader->deviceHeaderOffset[0] + loadAddr;
	        pDevHeader_11 = (PDeviceHeader_11)temp_p;

			// check nand ID
		#if NAND_CODE
			NandID_Right = pDevHeader_11->FlashOpt.NandID;
		
			if (NandID != NandID_Right)
			{
				FatalError(NANDIDDISMATCH);
			}
		#endif

		#if SPI_CODE
			AddMessageError(REPORT_FLASH_SIZE_NOTIFICATION, Flash_size);
		#endif
	
		    // skip blocks
 			skip_blocks = pDevHeader_11->FlashOpt.Skip_Blocks_Struct.Total_Number_Of_SkipBlocks;
			if (skip_blocks > 0)
			{
				pSkipAddress = &pDevHeader_11->FlashOpt.Skip_Blocks_Struct;
			}
			else
				pSkipAddress = NULL;

			productMode = pDevHeader_11->ProductMode;
			option = pDevHeader_11->OptValue;
			Reset_flag = option;
			
			if (OKtoErase)
			{
				if (pDevHeader_11->FlashOpt.EraseAll == 2) // erase all flash partition without burning
				{
					AddMessageError(REPORT_NOTIFICATION, PlatformEraseAllFlashWithoutBurn); // notify SWD

					Retval = EraseAllFlash(BOOT_FLASH);
					if (Retval != NoError)
					{
						FatalError(Retval);
					}

					AddMessageError(REPORT_NOTIFICATION, PlatformEraseAllFlashWithoutBurnDone); // notify SWD
					AddMessageError(REPORT_NOTIFICATION, PlatformReady);

					break;
				}
				
				// check erase all flag
				if (pDevHeader_11->FlashOpt.EraseAll == 1) // erase user partition with burning
				{
					// check reset BBT flag
					if (pDevHeader_11->FlashOpt.ResetBBT)
					{
						AddMessageError(REPORT_NOTIFICATION, PlatformResetBBT); // notify SWD that OBM/flasher will reset BBT
						ResetBBT();
					}
					
					AddMessageError(REPORT_NOTIFICATION, PlatformEraseAllFlash); // notify SWD that OBM/flasher will erase all flash
					
					Retval = EraseAllFlash(BOOT_FLASH);
					if (Retval != NoError)
					{
						FatalError(Retval);
					}
					
					OKtoErase = 0;
					
					AddMessageError(REPORT_NOTIFICATION, PlatformEraseAllFlashDone); // notify SWD that OBM/flasher erases all flash done
				}
			}

	        for ( imagenum = 0; imagenum < pDevHeader_11->nOfImages; imagenum++ )
	        {
	            temp_p = (UINT_T)&pDevHeader_11->imageStruct_11[imagenum];
	            pImage_11 = (PImageStruct_11)temp_p;

				imageChecksum = CalcImageChecksum((UINT_T*)(loadAddr+(pImage_11->First_Sector<<12)),
													pImage_11->length);
				if (imageChecksum != pImage_11->ChecksumFormatVersion2)
					FatalError(CRCFailedError);

				#if MMC_CODE
				PartitionNumber = ((pImage_11->commands & 0xFF000000)>>24);
				if((pTIM_h->pConsTIM->VersionBind.Version) >= TIM_3_2_00)
				{
					SetPartition(PartitionNumber);
				}
				#endif

	            if(OKtoErase)
	            {
	                Retval = EraseFlash(pImage_11->Flash_Start_Address,
										pImage_11->Flash_erase_size,
										BOOT_FLASH );
					
	                if(Retval != NoError )
	                    FatalError(Retval);
					
	               //if( FlashNumber == 0x0a ) OKtoErase = 0;
	            }
				
				if ((pImage_11->commands & 0xF) == 2) //erase only
					continue;
				
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

			#if DEBUG_SPI_PROTECT
				if (pImage_11->Flash_Start_Address == 0x0)
					Giga_SPINor_Protect_Blocks();
					//MX_SPINOR_Protect_Blocks();

				serial_outstr("pImage_11->Flash_Start_Address\n");
				serial_outnum(pImage_11->Flash_Start_Address);
				serial_outstr("\n");
			#endif

	            Retval = WriteFlash( pImage_11->Flash_Start_Address,
									loadAddr+(pImage_11->First_Sector<<12),
									pImage_11->length, BOOT_FLASH );
				
	            if(Retval != NoError )
	                FatalError(Retval);

				AddMessageError(REPORT_BURNT_IMAGE_LENGTH, pImage_11->length); // report burnt image length
	        }
		
        }

		AddMessageError(REPORT_NOTIFICATION, PlatformReady);
        #endif
        #endif

nextimage:		
        NumImages--;
        pImageInfo = (IMAGE_INFO_3_4_0*)((unsigned char*)pImageInfo - ImageSize);   // index to previous image info record.
    }                        // end WHILE

	ClearPortInterruptFlag();
    
	SetUseSpareArea( FALSE, BOOT_FLASH );
	UpdateBBT();

	#if SPI_CODE
	Giga_Disable4BytesMode();
	#endif

	#if LWG_LTG_SUPPORT
	PlatformClearLTGLWGFlag(); // clear the last LTG/LWG flag for upgrade
	#endif

	// The next image to which we will transfer control to will be the OBM. 
	// So, find the OBM image in the TIM and return it. 
   	pImageInfo = FindImageInTIM(pTIM_h, OBMIDENTIFIER);

	return pImageInfo;	
}
