
/******************************************************************************
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
 *
 ******************************************************************************
 *
 *
 *  FILENAME:	BootMode.c
 *
 *  PURPOSE: 	Contains BootLoader's BootMode Code. Mainly handles the 
 * 				SINGLE and DUAL TIM MODE boot functions. 
 *
******************************************************************************/


//////////////////////////////////////////////////////////////////////
// Library Support for the BootMode.c
// Include any shared libraries in the BootMode.h file to keep this 
// code clean. 
//////////////////////////////////////////////////////////////////////
#include "BootMode.h"

#if DECOMPRESS_SUPPORT
#include "LzmaDecode.h"
#endif
#include "timer.h"

#if I2C
#include "I2C.h"
extern UINT_T power_up_flag;
#endif

#if SBOOT
extern UINT_T sboot_flag;
#endif

#if ZIMI_PB05
#include "zimi_bq24295.h"
#endif

UINT_T MPUFlag = 0;
UINT_T back_image_key = 0;
extern UINT_T nextImageSize;
extern UINT_T upgrade_failed_flag;

pIMAGE_INFO_3_4_0 LoadAllImages( pTIM pTIM_h);
pIMAGE_INFO_3_4_0 LoadSilentImages(pTIM pTIM_h);
pIMAGE_INFO_3_4_0 LoadBackupImages(pTIM pTIM_h);

//////////////////////////////////////////////////////////////////////
// This is the entry point for the Boot Mode. 
//
// Inputs: Current TIM pointer, BootMode(SINGLE or DUAL)
// Outputs: Returns the next image that we will transfer the control to.
//
// It mainly functions as following depending on the BOOT MODE:
// 1) SINGLE TIM BOOT MODE:
//		a) Configures the Flashes and Initializes the Flash Management	
//		b) Loads (and validates) all the images using a loop.
//		c) If TRUSTED, provisions the platform before finishing
//		d) Returns the next image that we will transfer the control to. 
// 2) DUAL TIM BOOT MODE:
//		a) Configures the Flashes and Initializes the Flash Management
// 		b) Loads (and validates) the second TIM. 	
//		c) Configures the Flashes and Initializes the Flash Management 
//		   using the new TIM.
//		d) Loads (and validates) all the images using a loop.
//		e) If TRUSTED, provisions the platform before finishing
//      f) Returns the next image that we will transfer the control to.
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 BootModeMain(pTIM pTIM_h, OPERATING_MODE_T BootMode, pFUSE_SET pFuses)
{
    UINT_T Retval = NoError;
    pIMAGE_INFO_3_4_0 pBootImageInfo = NULL;
	UINT_T FlashNumber = 0;

	// Determine the Flash that will be used to store the downloaded images.
	FlashNumber = (pTIM_h->pConsTIM->FlashInfo.BootFlashSign) & 0xFF;
	
	Retval = Configure_Flashes (FlashNumber, BOOT_FLASH);
	if( Retval != NoError)
	   FatalError(Retval);
	
	//turn on the flash management
	InitializeFM(LEGACY_METHOD, BOOT_FLASH);

	// Fall through to SINGLE TIM Mode. 
	// Remember in both SINGLE and DUAL TIM Modes, we need to 
	// go through the SINGLE TIM Mode branch at the end. 

#if ZIMI_PB05
	UINT16_T vbat_mv;
	extern PowerUPType_t powerup;
	extern int isTempDetect;
	if(powerup == PowerUP_ONKEY)
	{
		UINT8_T m_boot;

		if(isTempDetect)
		{
			UINT8_T mm = check_BootONKey(10, 0);
			if(mm == 0)
			{
				serial_outstr("zimi# check onkey 10ms failed\n");
				System_poweroff();
			}
			bqTempDetect();   //here delay 500ms

			m_boot = check_BootONKey(1000, 0);
		}
		else
			m_boot = check_BootONKey(1, 1);
		if(m_boot == 0)
		{
			serial_outstr("zimi# check 3s onkey failed\n");
			System_poweroff();
		}
	}

	#if ZIMI_LED_MODE
	GetBatInstantVolt(&vbat_mv,USB_NotConnect);
	UINT8_T m_temp = IIC_read(0x07) & 0xff;

	if(3500<vbat_mv && vbat_mv<4050)
	{
		 m_temp |= 0x20;
		 IIC_write(0x07,m_temp);
		 Delay_us(300*1000);
		 GetBatInstantVolt(&vbat_mv,USB_NotConnect);
		 m_temp &= ~0x20;
		 IIC_write(0x07,m_temp);
	}
	else
	{
		GetBatInstantVolt(&vbat_mv,USB_NotConnect);
	}

	*(VUINT_T *)ZIMI_BATVOL_INIT_FLAG_ADDR = vbat_mv;

	if(vbat_mv<3600)
	{
		serial_outstr("zimi# bat led low\n");
		LED_BAT_R_ON();
	}
	else if(vbat_mv<3750)
	{
		serial_outstr("zimi# bat led middle\n");
		LED_BAT_R_ON();
		LED_BAT_G_ON();
	}
	else
	{
		serial_outstr("zimi# bat led high\n");
		LED_BAT_G_ON();
	}
	#else
	#if 0   //case: big/small system led: all green
	extern PowerUPType_t powerup;
	if(powerup != PowerUP_USB)
	{
		LED_WIFI_G_ON();
		LED_BAT_G_ON();
		LED_LTE_G_ON();
	}
	else  //xyl USB mode, we light BAT led only according to VBAT
	{
		if(vbat_mv<3650)
		{
			serial_outstr("zimi# bat led low\n");
			LED_BAT_R_ON();
		}
		else if(vbat_mv<3950)
		{
			serial_outstr("zimi# bat led middle\n");
			LED_BAT_R_ON();
			LED_BAT_G_ON();
		}
		else
		{
			serial_outstr("zimi# bat led high\n");
			LED_BAT_G_ON();
		}
	}
	#endif
	#endif
#endif

#if TR069_SUPPORT
	Retval = firmware_upgrade(pTIM_h);
	serial_outstr("TR069 return\n");
	serial_outnum(Retval);
	serial_outstr("\n");
	if (Retval == NoError)
	{
		PlatformSetTR069Flag();
		do_wdt_reset();
	}
	else
#endif
	{
	#if SILENT_RESET
		if (PlatformCheckSilentReset())
			pBootImageInfo = LoadSilentImages(pTIM_h);
		else
	#endif
	#if BACKUP_IMAGE
		if (back_image_key || upgrade_failed_flag || (PlatformCheckBackupImagesFlag() == 0x4D494E53))
			pBootImageInfo = LoadBackupImages(pTIM_h);
		else
	#endif

			pBootImageInfo = LoadAllImages(pTIM_h);
	}
									  				
    return pBootImageInfo;
}

#if MRD_CHECK
pIMAGE_INFO_3_4_0 pMRD_valid	= NULL;
pIMAGE_INFO_3_4_0 pMRD_invalid	= NULL;
UINT_T CheckMRD(pIMAGE_INFO_3_4_0 pImage)
{
	RD_HEADER	*pRDHeader = NULL;
	RD_ENTRY	*pRDEntry = NULL;
	UINT_T MRDLength = 0;
	UINT_T EntryLength = 0;
	UINT_T HeaderLength = 0;
	UINT_T imageChecksum = 0;
			
	pRDHeader = (RD_HEADER *)pImage->LoadAddr; // points to RD_HEADER			
	if (pRDHeader->validBufferStamp != VALID_BUF_STMP)	// check MRD
	{
		serial_outstr("MRD invalid stamp\n");
		
		if (pMRD_invalid == NULL)
		{
			pMRD_invalid = pImage;
		}
		
		goto exit;
	}

	HeaderLength = sizeof(RD_HEADER); // length of RD_HEADER
	pRDEntry = (RD_ENTRY*)((unsigned char *)pRDHeader + HeaderLength);	// points to RD_ENTRY
	MRDLength += HeaderLength; // add header length to MRD length

	// length of MRD is 128KB (0x20000)
	while ((pRDEntry->entryType != END_BUF_STMP) && (MRDLength < MRD_LENGTH))
	{
		// the pointer pFileToWrite is not saved,
		// so, the length of one entry does not include the size of pointer pFileToWrite
		EntryLength = sizeof(RD_ENTRY) - sizeof(unsigned long *) + pRDEntry->size.entrySize; //length of a whole entry
		MRDLength += EntryLength; // add entry length to MRD length
		
		pRDEntry = (RD_ENTRY *)((unsigned char *)pRDEntry + EntryLength); // find next entry
	}

	// MRD CRC doesn't include stamp and crc itself
	imageChecksum = CalcMRDChecksum(&pRDHeader->version, MRDLength - 8);
	if (pRDHeader->bufCheckSum != imageChecksum)
	{
		serial_outstr("CRC check fail with flash address\n");
		serial_outnum(pImage->FlashEntryAddr);
		serial_outstr("\n");

		if (pMRD_invalid == NULL)
		{
			pMRD_invalid = pImage;
		}
	}
	else
	{
		serial_outstr("CRC check OK with flash address\n");
		serial_outnum(pImage->FlashEntryAddr);
		serial_outstr("\n");

		if (pMRD_valid == NULL)
		{
			pMRD_valid = pImage;
		}
	}

exit:

	// OBM only pass RBLI to CP or RBLI/RBLR if LWG/LTG enabled
	// this wil make sure the RBLR or LTG1/LWG1 is always OK
#if LWG_LTG_SUPPORT
	if ((pImage->ImageID == RELIABLEID) || (pImage->ImageID == LTGRELIABLEID))
#else
	if (pImage->ImageID == RELIABLEID)
#endif
	{
		PlatformSetCurrentMRDAddr(pImage->FlashEntryAddr);
		serial_outstr("MRD FlashAddress Passed to CP:\n");
		serial_outnum(pImage->FlashEntryAddr);
		serial_outstr("\n");
	}
	
	return NoError;
}

UINT_T FinalizeMRD(void)
{
	UINT_T Retval = NoError;
	
	// no valid MRD, fatal error
	if (pMRD_valid == NULL)
	{
		serial_outstr("No valid MRD\n");
		zimi_force_minisys(NOVALIDMRD);
		FatalError(NOVALIDMRD);
	}

	// both of the MRDs are OK, no problem
	if (pMRD_invalid == NULL)
	{
		serial_outstr("Primary/Backup MRD are both OK\n");

		// reload valid MRD
		Retval = ReadFlash(pMRD_valid->FlashEntryAddr, pMRD_valid->LoadAddr, pMRD_valid->ImageSize, BOOT_FLASH);
		if (Retval != NoError)
		{
			zimi_force_minisys(Retval);
			FatalError(Retval);
		}
		
		return NoError;
	}

	serial_outstr("Start to write valid MRD to corrupted MRD\n");
	
	// one MRD is OK, the other MRD has problem
	// we don't know which one is valid, so, we need to reload valid MRD
	Retval = ReadFlash(pMRD_valid->FlashEntryAddr, pMRD_valid->LoadAddr, pMRD_valid->ImageSize, BOOT_FLASH);
	if (Retval != NoError)
	{
		zimi_force_minisys(Retval);
		FatalError(Retval);
	}

	// and then erase invalid MRD
	Retval = EraseFlash(pMRD_invalid->FlashEntryAddr, pMRD_invalid->ImageSize, BOOT_FLASH);
	if (Retval != NoError)
	{
		zimi_force_minisys(Retval);
		FatalError(Retval);
	}

	// and then write valid MRD to flash
	Retval = WriteFlash(pMRD_invalid->FlashEntryAddr, pMRD_valid->LoadAddr, pMRD_invalid->ImageSize, BOOT_FLASH);
	if (Retval != NoError)
	{
		zimi_force_minisys(Retval);
		FatalError(Retval);
	}

	serial_outstr("Write valid MRD to corrupted MRD Done\n");

	return NoError;
}
#endif

//////////////////////////////////////////////////////////////////////
// This function mainly loads ALL the images available in the TIM 
// (except TIM, DUALTIM, and OBM) and validates them. 
// It essentially returns the next image that we will transfer the 
// control to. 
//
// Inputs: Current TIM pointer, BootMode(SINGLE or DUAL)
// Outputs: Returns the next image that we will transfer the control to.
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 LoadAllImages( pTIM pTIM_h)
{
	UINT_T Retval = NoError;
    UINT_T ImageID = 0;									// Initialize it as an invalid image
	UINT_T ImageIndex, ImageSize, decompressLength, start, end, start_read, end_read;
	pIMAGE_INFO_3_4_0 pImageInfo = NULL;
	pIMAGE_INFO_3_4_0 pNextImageToTransferTo = NULL;	// Used for storing the next image we want to transfer control to
														// after loading all the available images.


	flash_test();
	
	#if ZIMI_PB05
	extern PowerUPType_t powerup;
	extern int isInNormalSys;
	isInNormalSys = 1;   //set Normal sys flag
	if(powerup != PowerUP_USB)
	{
		#if ZIMI_LAST_LED_MODE
		if(powerup == PowerUP_ONKEY)
		{
			zimi_blink_led();
		}
		else if(powerup == PowerUP_Reset)
		{
			LED_ALL_ON();
		}
		#else
		LED_WIFI_G_ON();
		LED_BAT_G_ON();
		LED_LTE_G_ON();
		#endif
	}
	else  //xyl USB mode, we light BAT led only according to VBAT
	{
		#if 0
		UINT16_T vbat_mv;


		GetBatInstantVolt(&vbat_mv,USB_NotConnect);
		UINT8_T m_temp = IIC_read(0x07) & 0xff;

		if(3500<vbat_mv && vbat_mv<4050)
		{
			 m_temp |= 0x20;
		 	IIC_write(0x07,m_temp);
		 	Delay_us(300*1000);
			//GetBatInstantVolt(&vbat_mv,USB_NotConnect);
			ReadBatVolt(&vbat_mv);
		 	m_temp &= ~0x20;
		 	IIC_write(0x07,m_temp);
		}
		else
		{
			//GetBatInstantVolt(&vbat_mv,USB_NotConnect);
			ReadBatVolt(&vbat_mv);
		}

		
		serial_outstr("zimi# last vbat\n");
		serial_outnum(vbat_mv);
		serial_outstr("\n");

		*(VUINT_T *)ZIMI_BATVOL_INIT_FLAG_ADDR = vbat_mv;

		if(vbat_mv<3600)
		{
			serial_outstr("zimi# bat led low\n");
			LED_BAT_R_ON();
		}
		else if(vbat_mv<3750)
		{
			serial_outstr("zimi# bat led middle\n");
			LED_BAT_R_ON();
			LED_BAT_G_ON();
		}
		else
		{
			serial_outstr("zimi# bat led high\n");
			LED_BAT_G_ON();
		}
		#endif
	}
	#endif


#if LWG_LTG_SUPPORT
	VUINT_T LWGLTGFlag = PlatformCheckLTGLWGFlag();
	pWTP_RESERVED_AREA_HEADER pWRAH = NULL;
	pLTG_LWG_Select pLL = NULL;
	UINT_T choice = 0;
#endif

#if DECOMPRESS_SUPPORT
	OBM_MPUCache_Init();
	MPUFlag = 1;
#endif

	// Depending on the version of the TIM, determine the size of each image in bytes. 
	// We will use this size to iterate through the TIM binary from image to image. 
	if (pTIM_h->pConsTIM->VersionBind.Version >= TIM_3_4_00)
		ImageSize = sizeof(IMAGE_INFO_3_4_0);
	else
		ImageSize = sizeof(IMAGE_INFO_3_2_0);

	// This is the very initial TIM image! Assumes that the TIM image is located at the top of the 
	// TIM file. Otherwise, we would skip images. 
	pImageInfo = pTIM_h->pImg;

#if LWG_LTG_SUPPORT
	serial_outstr("LWG/LTG switch flag\n");
	serial_outnum(LWGLTGFlag);
	serial_outstr("\n");

	if ((LWGLTGFlag != PLATFORM_5MODE_LTG_VER) &&
		(LWGLTGFlag != PLATFORM_5MODE_LWG_VER) &&
		(LWGLTGFlag != PLATFORM_3MODE_LTG_VER) &&
		(LWGLTGFlag != PLATFORM_3MODE_LWG_VER))
	{
		pWRAH = FindPackageInReserved(&Retval, pTIM_h, LTGLWGSELECTID);
		if ((pWRAH == NULL) || (Retval != NoError))
		{
			PlatformSetLTGLWGFlag(PLATFORM_3MODE_LTG_VER);
			LWGLTGFlag = PLATFORM_3MODE_LTG_VER;
			serial_outstr("LTG/LWG select package not found, default to 3Mode LTG\n");
		}
		else
		{
			pLL = (pLTG_LWG_Select) pWRAH;
			choice	= pLL->Choice;
			switch (choice)
			{
				case 0: // 5Mdoe LTG
					PlatformSetLTGLWGFlag(PLATFORM_5MODE_LTG_VER);
					LWGLTGFlag = PLATFORM_5MODE_LTG_VER;
					serial_outstr("Select to 5Mode LTG\n");
					break;
					
				case 1: // 5Mode LWG
					PlatformSetLTGLWGFlag(PLATFORM_5MODE_LWG_VER);
					LWGLTGFlag = PLATFORM_5MODE_LWG_VER;
					serial_outstr("Select to 5Mode LWG\n");
					break;
					
				case 2: // 3Mode LTG
					PlatformSetLTGLWGFlag(PLATFORM_3MODE_LTG_VER);
					LWGLTGFlag = PLATFORM_3MODE_LTG_VER;
					serial_outstr("Select to 3Mode LTG\n");
					break;
					
				case 3: // 3Mode LWG
					PlatformSetLTGLWGFlag(PLATFORM_3MODE_LWG_VER);
					LWGLTGFlag = PLATFORM_3MODE_LWG_VER;
					serial_outstr("Select to 3Mode LWG\n");
					break;
					
				default:
					PlatformSetLTGLWGFlag(PLATFORM_3MODE_LTG_VER);
					LWGLTGFlag = PLATFORM_3MODE_LTG_VER;
					serial_outstr("Wrong package setting found, default to 3Mode LTG\n");
					break;
			}
		}
	}
#endif

	// Read in the number of images from the TIM and iterate through each of these images. 
	// We load them and we validate them. 
	for( ImageIndex = 0; ImageIndex < pTIM_h->pConsTIM->NumImages; ImageIndex++ )
    {
		// We skip the TIM, DUALTIM, and OBM images. We load/validate all the others. 
    	if( (pImageInfo->ImageID != TIMIDENTIFIER) && 
            (pImageInfo->ImageID != OBMIDENTIFIER) &&  
            (pImageInfo->ImageID != OBM2IDENTIFIER) &&
			(pImageInfo->LoadAddr != 0xFFFFFFFF)
          )	
		{
		#if I2C
			//battery_process_step3();
		#endif

		#if BACKUP_IMAGE
		if (pImageInfo->ImageID == SBOOT_ID)
		{
			serial_outstr("skip to load backup CP\n");
			pImageInfo = (IMAGE_INFO_3_4_0*)((unsigned char*)pImageInfo + ImageSize);
			continue;
		}
		#endif

		#if SBOOT
		if ((sboot_flag == 0x55) && (pImageInfo->ImageID == SBOOT_ID))
		{
			serial_outstr("skip to load sboot\n");
			pImageInfo = (IMAGE_INFO_3_4_0*)((unsigned char*)pImageInfo + ImageSize);
			continue;
		}
		#else
		if (pImageInfo->ImageID == SBOOT_ID)
		{
			serial_outstr("skip to load sboot\n");
			pImageInfo = (IMAGE_INFO_3_4_0*)((unsigned char*)pImageInfo + ImageSize);
			continue;
		}
		#endif

		#if LWG_LTG_SUPPORT
		if ((LWGLTGFlag == PLATFORM_5MODE_LWG_VER) || 
			(LWGLTGFlag == PLATFORM_3MODE_LWG_VER))
		{
			if ( (pImageInfo->ImageID == LTGOSLOADERID) || 
	            (pImageInfo->ImageID == LTGRELIABLEID) ||  
	            (pImageInfo->ImageID == LTGDSPID) ||
				(pImageInfo->ImageID == LTGSKYLARKID) ||
				(pImageInfo->ImageID == LTGRESERVED1ID) ||
				(pImageInfo->ImageID == LTGRESERVED2ID) ||
				(pImageInfo->ImageID == LTGRESERVED3ID)
	          )
			{
				serial_outstr("Skip to load LTG image flash address\n");
				serial_outnum(pImageInfo->FlashEntryAddr);
				serial_outstr("\n");
				pImageInfo = (IMAGE_INFO_3_4_0*)((unsigned char*)pImageInfo + ImageSize);
				continue;
			}
		}
		else if ((LWGLTGFlag == PLATFORM_5MODE_LTG_VER) || 
				(LWGLTGFlag == PLATFORM_3MODE_LTG_VER))
		{
			if ( (pImageInfo->ImageID == OSLOADERID) || 
	            (pImageInfo->ImageID == RELIABLEID) ||  
	            (pImageInfo->ImageID == LWGDSPID) ||
				(pImageInfo->ImageID == LWGSKYLARKID) ||
				(pImageInfo->ImageID == LWGRESERVED1ID) ||
				(pImageInfo->ImageID == LWGRESERVED2ID) ||
				(pImageInfo->ImageID == LWGRESERVED3ID)
	          )
	        {
				serial_outstr("Skip to load LWG image flash address\n");
				serial_outnum(pImageInfo->FlashEntryAddr);
				serial_outstr("\n");
				pImageInfo = (IMAGE_INFO_3_4_0*)((unsigned char*)pImageInfo + ImageSize);
				continue;
			}
		}
		#endif
		
			// Store a pointer to the OSLO image because we will transfer control to it!		    
			// If this image is not found in the TIM, then we return NULL and 
			// we will fail in BootLoaderMain.
		#if LWG_LTG_SUPPORT
			if (((LWGLTGFlag == PLATFORM_5MODE_LTG_VER) || 
				(LWGLTGFlag == PLATFORM_3MODE_LTG_VER)) && 
				(pImageInfo->ImageID == LTGOSLOADERID))
			{
				#if USE_SERIAL_DEBUG
				serial_outstr("LTG uboot\n");
				#endif
			    pNextImageToTransferTo = pImageInfo;
			}

			if (((LWGLTGFlag == PLATFORM_5MODE_LWG_VER) || 
				(LWGLTGFlag == PLATFORM_3MODE_LWG_VER)) && 
				(pImageInfo->ImageID == OSLOADERID))
			{
				#if USE_SERIAL_DEBUG
				serial_outstr("LWG uboot\n");
				#endif
		    	pNextImageToTransferTo = pImageInfo;
			}
		#elif SBOOT
			if ((pImageInfo->ImageID == OSLOADERID) && (sboot_flag == 0x55))
			{
				#if USE_SERIAL_DEBUG
				serial_outstr("uboot\n");
				#endif
			    pNextImageToTransferTo = pImageInfo;
			}

			if ((sboot_flag == 0xaa) && (pImageInfo->ImageID == SBOOT_ID))
			{
				#if USE_SERIAL_DEBUG
				serial_outstr("Sboot\n");
				#endif
			    pNextImageToTransferTo = pImageInfo;
			}
		#else
			if (pImageInfo->ImageID == OSLOADERID)
			{
				#if USE_SERIAL_DEBUG
				serial_outstr("uboot\n");
				#endif
			    pNextImageToTransferTo = pImageInfo;
			}
		#endif

	        // We read images from the flash and load them into the LOAD ADDRESS specified in the TIM for each image. 
	        // For now, we do NOT care if there is a conflict when an image needs to load where we are currently running from.
			// We assume that this will not happen. 
	        #if MMC_CODE
			if((pTIM_h->pConsTIM->VersionBind.Version) >= TIM_3_2_00)
            {
				SetPartition(pImageInfo->PartitionNumber, BOOT_FLASH);
			}
			#endif

		#if DECOMPRESS_SUPPORT
			if ((pImageInfo->ImageID != RELIABLEID) && 
					(pImageInfo->ImageID != LTGRELIABLEID) &&
					(pImageInfo->ImageID != LWGRESERVED1ID) &&
					(pImageInfo->ImageID != LTGRESERVED1ID))
			{
			#if SBOOT
				if (pImageInfo->ImageID != SBOOT_ID)
			#endif
				{
					CacheInvalidateMemory(DECOMPRESS_TEMP_ADDR, DECOMPRESS_LENGTH);
					start_read= GetOSCR0();
					start_read = GetOSCR0();
			        Retval = ReadFlash(pImageInfo->FlashEntryAddr, DECOMPRESS_TEMP_ADDR, pImageInfo->ImageSize, BOOT_FLASH);
					end_read= GetOSCR0();
					serial_outstr("read time\n");
					if (end_read > start_read)
						serial_outnum(OSCR0IntervalInMilli(start_read, end_read));
					else
						serial_outnum(OSCR0IntervalInMilli(end_read, start_read));
					serial_outstr("\n");
					// If ReadFlash function fails, then we fail and return NULL. 
			        if (Retval != NoError)
					{
						zimi_force_minisys(Retval);
						FatalError(Retval);
					}

					decompressLength = DECOMPRESS_LENGTH;

					#if ZIMI_PB05
					#if ZIMI_LED_MODE
					UINT8_T onkey_long_flag =0;
					onkey_long_flag = UsticaBaseRead(USTICA_STATUS);
					onkey_long_flag = UsticaBaseRead(USTICA_STATUS);//xyl:re-read for bug,fuck marvell
					if ((onkey_long_flag & USTICA_ONKEY_STATUS) == USTICA_ONKEY_STATUS)  //when onkey pressed > 3s, we need to tell CP
					{
						serial_outstr("onkey long press detected\n");
						*(VUINT_T *)ZIMI_ONKEY_LONG_PRESS_FLAG_ADDR= 0x4C4F4E47;//LONG
						LED_WIFI_G_ON();
					}
					#endif
					#endif
					
					start = GetOSCR0();
					start = GetOSCR0();
					#if QPRESS
					Retval = quickLZ_decompress(DECOMPRESS_TEMP_ADDR, (UINT8_T *)pImageInfo->LoadAddr);
					#else
					Retval = LZMA_Decompress((UINT_T *)pImageInfo->LoadAddr, &decompressLength, (UINT_T *)DECOMPRESS_TEMP_ADDR, pImageInfo->ImageSize);
					#endif
					end = GetOSCR0();

					if (pImageInfo->ImageID == OSLOADERID)
						nextImageSize = decompressLength;
					
					serial_outstr("Retval\n");
					serial_outnum(Retval);
					serial_outstr("\n");
					serial_outstr("time\n");
					if (end > start)
						serial_outnum(OSCR0IntervalInMilli(start, end));
					else
						serial_outnum(OSCR0IntervalInMilli(end, start));
					serial_outstr("\n");

					//CacheInvalidateMemory(DECOMPRESS_TEMP_ADDR, DECOMPRESS_LENGTH);
					
					if (Retval != NoError)
					{
						zimi_force_minisys(Retval);
						FatalError(Retval);
					}
				}
			#if SBOOT
				else
				{
					Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
					if (Retval != NoError)
					{
						zimi_force_minisys(Retval);
						FatalError(Retval);
					}
				}
			#endif
			}
			else
			{
			#if MRD_CHECK
				PlatformSetMRDAddr(pImageInfo->FlashEntryAddr, pImageInfo->ImageID);
				
				Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
				if (Retval != NoError)
				{
					zimi_force_minisys(Retval);
					FatalError(Retval);
				}

				CheckMRD(pImageInfo);
			
			#else
				Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
				if (Retval != NoError)
				{
					zimi_force_minisys(Retval);
					FatalError(Retval);
				}
			#endif
			}
		#else
			#if MRD_CHECK
				if ((pImageInfo->ImageID != RELIABLEID) && 
					(pImageInfo->ImageID != LTGRELIABLEID) &&
					(pImageInfo->ImageID != LWGRESERVED1ID) &&
					(pImageInfo->ImageID != LTGRESERVED1ID))
				{
					//start = GetOSCR0();
					Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
					//end = GetOSCR0();
					//serial_outstr("read singal image\n");
					//if (end > start)
					//	serial_outnum(OSCR0IntervalInMilli(start, end));
					//else
					//	serial_outnum(OSCR0IntervalInMilli(end, start));
					//serial_outstr("\n");
					if (Retval != NoError)
					{
						zimi_force_minisys(Retval);
						FatalError(Retval);
					}
				}
				else 
				{
					PlatformSetMRDAddr(pImageInfo->FlashEntryAddr, pImageInfo->ImageID);
									
					Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
					if (Retval != NoError)
					{
						zimi_force_minisys(Retval);
						FatalError(Retval);
					}

					CheckMRD(pImageInfo);
				}
			#else
				Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
				if (Retval != NoError)
				{
					zimi_force_minisys(Retval);
					FatalError(Retval);
				}
			#endif

		#endif
			
		}

		// Get a pointer to the next image we will load. 
		// For the last iteration (e.g. after the last image), we won't find an image.
		// However, this is OK because pImageInfo is not used again.   
		pImageInfo = (IMAGE_INFO_3_4_0*)((unsigned char*)pImageInfo + ImageSize);
    }

#if MRD_CHECK
#if SBOOT
	if ((pMRD_valid == NULL) && (sboot_flag != 0xaa))
	{
		zimi_force_minisys(NOVALIDMRD);
		FatalError(NOVALIDMRD);
	}
#else
	if (pMRD_valid == NULL)
	{
		zimi_force_minisys(NOVALIDMRD);
		FatalError(NOVALIDMRD);
	}
#endif

	FinalizeMRD();
#endif

#if DECOMPRESS_SUPPORT
	OBM_Flush();
	OBM_MPUCache_Disable();
#endif

#if SPI_CODE
	Giga_Disable4BytesMode();
#endif

   	return pNextImageToTransferTo;
}

pIMAGE_INFO_3_4_0 LoadSilentImages(pTIM pTIM_h)
{
	pIMAGE_INFO_3_4_0 pImageInfo = NULL;
	pIMAGE_INFO_3_4_0 pNextImageToTransferTo = NULL;
	UINT_T Retval = NoError;
	UINT_T Image_Index = 0;
	UINT_T Silent_Images[] = { LWGDSPID, LWGSKYLARKID, RELIABLEID, LTGOSLOADERID, 0x0 };

	while (Silent_Images[Image_Index] != 0x0)
	{
		pImageInfo = FindImageInTIM(pTIM_h, Silent_Images[Image_Index]);
		if (pImageInfo != NULL)
		{
		#if MRD_CHECK
			if ((pImageInfo->ImageID != RELIABLEID) && (pImageInfo->ImageID  != LTGOSLOADERID))
			{
				Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
		        if (Retval != NoError)
				{
					FatalError(Retval);
				}
			}
			else
			{
				PlatformSetMRDAddr(pImageInfo->FlashEntryAddr, pImageInfo->ImageID);
				
				Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
				if (Retval != NoError)
				{
					FatalError(Retval);
				}

				CheckMRD(pImageInfo);
			}
		#else
			Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
	        if (Retval != NoError)
			{
				FatalError(Retval);
			}
		#endif
		}

		Image_Index++;
	}

	serial_outstr("Silent Reset Boot\n");
	pNextImageToTransferTo = FindImageInTIM(pTIM_h, OSLOADERID);

	#if MRD_CHECK
	FinalizeMRD();
	#endif

	// Return the OSLO image.
	return pNextImageToTransferTo;
}

#if BACKUP_IMAGE
pIMAGE_INFO_3_4_0 LoadBackupImages(pTIM pTIM_h)
{
	pIMAGE_INFO_3_4_0 pImageInfo = NULL;
	pIMAGE_INFO_3_4_0 pNextImageToTransferTo = NULL;
	UINT_T Retval = NoError;
	UINT_T Image_Index = 0, decompressLength;
	UINT_T Backup_Images[] = { SBOOT_ID, LTGRELIABLEID,LTGRESERVED1ID,0x0 };

	
	#if ZIMI_PB05
	extern int isInMiniSys;
	isInMiniSys = 1;  //set flag
	LED_BAT_R_ON();
	LED_LTE_R_ON();
	LED_WIFI_R_ON();
	#endif
	
#if LWG_LTG_SUPPORT
	VUINT_T LWGLTGFlag = PlatformCheckLTGLWGFlag();
	pWTP_RESERVED_AREA_HEADER pWRAH = NULL;
	pLTG_LWG_Select pLL = NULL;
	UINT_T choice = 0;

	serial_outstr("LWG/LTG switch flag\n");
	serial_outnum(LWGLTGFlag);
	serial_outstr("\n");

	if ((LWGLTGFlag != PLATFORM_5MODE_LTG_VER) &&
		(LWGLTGFlag != PLATFORM_5MODE_LWG_VER) &&
		(LWGLTGFlag != PLATFORM_3MODE_LTG_VER) &&
		(LWGLTGFlag != PLATFORM_3MODE_LWG_VER))
	{
		pWRAH = FindPackageInReserved(&Retval, pTIM_h, LTGLWGSELECTID);
		if ((pWRAH == NULL) || (Retval != NoError))
		{
			PlatformSetLTGLWGFlag(PLATFORM_3MODE_LTG_VER);
			LWGLTGFlag = PLATFORM_3MODE_LTG_VER;
			serial_outstr("LTG/LWG select package not found, default to 3Mode LTG\n");
		}
		else
		{
			pLL = (pLTG_LWG_Select) pWRAH;
			choice	= pLL->Choice;
			switch (choice)
			{
				case 0: // 5Mdoe LTG
					PlatformSetLTGLWGFlag(PLATFORM_5MODE_LTG_VER);
					LWGLTGFlag = PLATFORM_5MODE_LTG_VER;
					serial_outstr("Select to 5Mode LTG\n");
					break;
					
				case 1: // 5Mode LWG
					PlatformSetLTGLWGFlag(PLATFORM_5MODE_LWG_VER);
					LWGLTGFlag = PLATFORM_5MODE_LWG_VER;
					serial_outstr("Select to 5Mode LWG\n");
					break;
					
				case 2: // 3Mode LTG
					PlatformSetLTGLWGFlag(PLATFORM_3MODE_LTG_VER);
					LWGLTGFlag = PLATFORM_3MODE_LTG_VER;
					serial_outstr("Select to 3Mode LTG\n");
					break;
					
				case 3: // 3Mode LWG
					PlatformSetLTGLWGFlag(PLATFORM_3MODE_LWG_VER);
					LWGLTGFlag = PLATFORM_3MODE_LWG_VER;
					serial_outstr("Select to 3Mode LWG\n");
					break;
					
				default:
					PlatformSetLTGLWGFlag(PLATFORM_3MODE_LTG_VER);
					LWGLTGFlag = PLATFORM_3MODE_LTG_VER;
					serial_outstr("Wrong package setting found, default to 3Mode LTG\n");
					break;
			}
		}
	}
#endif

#if DECOMPRESS_SUPPORT
	OBM_MPUCache_Init();
	MPUFlag = 1;
#endif

	while (Backup_Images[Image_Index] != 0x0)
	{
		pImageInfo = FindImageInTIM(pTIM_h, Backup_Images[Image_Index]);

if ((pImageInfo->ImageID != RELIABLEID) && 
					(pImageInfo->ImageID != LTGRELIABLEID) &&
					(pImageInfo->ImageID != LWGRESERVED1ID) &&
					(pImageInfo->ImageID != LTGRESERVED1ID))
{
		if (pImageInfo != NULL)
		{
			CacheInvalidateMemory(DECOMPRESS_TEMP_ADDR, DECOMPRESS_LENGTH);
			Retval = ReadFlash(pImageInfo->FlashEntryAddr, DECOMPRESS_TEMP_ADDR, pImageInfo->ImageSize, BOOT_FLASH);
			if (Retval != NoError)
			{
				FatalError(Retval);
			}

			decompressLength = DECOMPRESS_LENGTH;

			#if QPRESS
			Retval = quickLZ_decompress(DECOMPRESS_TEMP_ADDR, (UINT8_T *)pImageInfo->LoadAddr);
			#else
			Retval = LZMA_Decompress((UINT_T *)pImageInfo->LoadAddr, &decompressLength, (UINT_T *)DECOMPRESS_TEMP_ADDR, pImageInfo->ImageSize);
			#endif

			if (Retval != NoError)
			{
				FatalError(Retval);
			}
		}
}
else
{
	#if MRD_CHECK
				PlatformSetMRDAddr(pImageInfo->FlashEntryAddr, pImageInfo->ImageID);
				
				Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
				if (Retval != NoError)
				{
					FatalError(Retval);
				}

				CheckMRD(pImageInfo);
			
	#else
				Retval = ReadFlash(pImageInfo->FlashEntryAddr, pImageInfo->LoadAddr, pImageInfo->ImageSize, BOOT_FLASH);
				if (Retval != NoError)
				{
					FatalError(Retval);
				}
	#endif
}
		Image_Index++;
	}

	serial_outstr("Backup Boot\n");
	pNextImageToTransferTo = FindImageInTIM(pTIM_h, SBOOT_ID);


#if MRD_CHECK
#if SBOOT
	if ((pMRD_valid == NULL) && (sboot_flag != 0xaa))
		FatalError(NOVALIDMRD);
#else
	if (pMRD_valid == NULL)
		FatalError(NOVALIDMRD);
#endif

	FinalizeMRD();
#endif

	

#if DECOMPRESS_SUPPORT
	OBM_Flush();
	OBM_MPUCache_Disable();
#endif

#if SPI_CODE
	Giga_Disable4BytesMode();
#endif

	// Return the OSLO image.
	return pNextImageToTransferTo;
}
#endif

