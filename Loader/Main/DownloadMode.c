
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
 *  FILENAME:	DownloadMode.c
 *
 *  PURPOSE: 	Contains BootLoader's DownloadMode Code. Mainly handles the
 * 				Software Upgrade and Download Mode functions of the BootLoader
 * 				by calling either TIMDownload or FBFDownload functions.
 *
******************************************************************************/


//////////////////////////////////////////////////////////////////////
// Library Support for the DownloadMode.c
// Include any shared libraries in the DownloadMode.h file to keep this
// code clean.
//////////////////////////////////////////////////////////////////////
#include "DownloadMode.h"

#if I2C
#include "I2C.h"
#include "charger.h"
#endif


//////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////
UINT_T isDownload = FALSE;		// Used to differentiate from UPLOAD.
UINT_T time_count_enable = TRUE; // used for timeout
UINT_T upload_nand_spare = FALSE; // not upload nand spare areas when default upload
extern UINT8_T upload_times;

#if I2C
UINT8_T battery_connect = 0;
UINT8_T external_power = 0;
extern UINT_T Jig_status;
extern UINT_T ONKey_pressed;
#endif

extern UINT8_T ResetUE;
extern UINT8_T ResetDelay;

//////////////////////////////////////////////////////////////////////
// This is the entry point for the Download Mode.
//
// Inputs: Fuses (platformsettings) which contain info about which port to use
//         and the TIM which can contain port overrides.
// Outputs: Returns a pointer to the next image which we will transfer control to.
//
// It mainly functions as following:
//		1) Download the description file and determine if we have a
//		   TIM or FBF download
//		2) Download all the TIM or FBF images
//		3) If UPLOAD, we continue with booting. 
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 DownloadModeMain( pFUSE_SET pFuses, pTIM pTIM_h, OPERATING_MODE_T BootMode )
{
    pIMAGE_INFO_3_4_0 pBootImageInfo = NULL;

	// Download the description file. Depending on its identifier, select
	// TIMDownload or FBFDownload. Download all the images and return a
	// pointer to the next image which we will transfer control to.
	pBootImageInfo = DetermineModeAndDownload( pFuses, pTIM_h );

    // At this point, the images have been downloaded and burnt to flash.
    // Also note that the last image downloaded is still in ddr. Upon
    // return from this routine, that image will be copied to its base
    // location and executed.

	// If UPLOAD, we continue with booting.
    if( !isDownload )
	{
		if (ResetUE != 0)
		{
			if (ResetDelay != 0)
				Delay(1000 * 1000 * ResetDelay);

			PlatformSetForceUSBEnumFlag();

			do_wdt_reset();
		}
		else
		{
		#if I2C && !AUTO_BOOTUP
		//if ((!battery_connect) && (!external_power))
		if (((ONKey_pressed == 1) || 
			(Jig_status == 7)) || 
			((Jig_status == 6) && ((battery_connect == 1) || (external_power == 1))))
			serial_outstr("Allow to boot up\n");
		else
		{
			serial_outstr("Not allow to boot up\n");
			/*xyl del*/
			//battery_process_step2();
		}
		#endif
		#if ZIMI_PB05
		PB05_charging_loop();
		#endif
			// After the upload, we continue with regular boot mode. 
			pBootImageInfo = BootModeMain( pTIM_h, BootMode, pFuses );
		}
	}

    return pBootImageInfo;
}


//////////////////////////////////////////////////////////////////////
// This is the high level download function for the Download Mode.
//
// Inputs: Fuses (platformsettings) and the TIM.
// Outputs: Returns a pointer to the next image which we will transfer
//          control to.
//
// It mainly determines the download mode (TIM or FBF) and calls the
// appropriate download routine.
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 DetermineModeAndDownload( pFUSE_SET pFuses, pTIM pTIM_h )
{
    pIMAGE_INFO_3_4_0 pBootImageInfo = NULL;
    FUNC_STATUS fs_Retval;
    volatile pProtocolISR pPortInterrupt;
    volatile pProtocolCmd pCommand;

	int start_time =0, cur_time=0;
	fs_Retval.ErrorCode = GeneralError;

	#if ZIMI_PB05
	int mTimeOut;

	extern UINT_T back_image_key;
	if(back_image_key)  //Onkey + reset
		mTimeOut =4;
	else
		mTimeOut =1;
	#endif
	
	InitDefaultPort( pFuses );
	//time_count_enable = 0;
    // The download port is opened. Get all the images and burn them to flash.
    // Start by trying to get a TIM followed by individual files.
    // If that fails, then try getting an FBF bundle.
    // If that fails as well, then treat this a a fatal error.
	start_time = GetOSCR0();
    do
    {
        pPortInterrupt = getProtocolISR();
		if(time_count_enable)
		{
			cur_time = GetOSCR0();
			if(OSCR0IntervalInSec(start_time, cur_time) >mTimeOut)  //xyl define
			{
				pBootImageInfo = NULL;
				goto shutdown;
			}
		}

		if (upload_times == 0)
		{
			goto disconnect; // no upload any more
		}

        if(pPortInterrupt->PreambleReceived == FALSE)
        {
        #if I2C
			resetTimer(); // reset charger 32s timer for key pressed download
		#endif
			fs_Retval.ErrorCode = SeqError;
            continue;
        }
		time_count_enable= 0;

		start_time = GetOSCR0();
		while (pPortInterrupt->CommandReceived == FALSE)
		{
			cur_time = GetOSCR0();
			if(OSCR0IntervalInSec(start_time, cur_time) > PROTOCOL_WAITTIME) //wait for command
			{
				pBootImageInfo = NULL;
				goto shutdown;
			}
		}

	    pCommand = getProtocolCmd();
	    switch(pCommand->Command)
	    {
	    case GetVersionCmd:
			// Used to differentiate from UPLOAD.
	        isDownload =  TRUE;
			#if I2C
			#if OLED_SUPPORT || LED_DISPLAY
			SoftwareUpgrade_Start();
			#endif
			#endif

	    	// Request the TIM first as originally implemented.
			// The new TIM is stored at pTIM_h->pConstTIM.

			// USB DMA can not access SQU, so OBM download DKB_NTIM to DDR, and then copy it to SQU
			// it will be fixed on Y0 board
	        fs_Retval = HandleRequest( DDR_DOWNLOAD_AREA_ADDR, TIMIDENTIFIER );
	        //fs_Retval = HandleRequest( (UINT_T)pTIM_h->pConsTIM, TIMIDENTIFIER );
	        if (fs_Retval.ErrorCode == NoError)
	        {
				// Before we call the TIM Download function which actually does the downloading,
				// copy the tim from its download location to its load address. 
				memcpy(pTIM_h->pConsTIM, DDR_DOWNLOAD_AREA_ADDR, 1024);

	            // pTIM_h is the newly downloaded TIM.
	            // It has a list of all the images that need to be downloaded and burned.
	            // The DownloadImages function will use DDR to hold the images while writing them to flash.
				pBootImageInfo = TIMDownloadMain( pTIM_h );
	        }
			else // download failed
			{
				FatalError(fs_Retval.ErrorCode);
			}
	        break;
			
	    case UploadDataHeaderCmd:
			// UPLOAD_DATA_AREA is in DDR
			// upload size limitted 0x1d00000 - 0x1000
	        fs_Retval = HandleRequest( (UINT_T)UPLOAD_DATA_AREA, NULL);
			if (fs_Retval.ErrorCode == NoError) // upload successfully
        	{
        		upload_times--;
        		fs_Retval.ErrorCode = GeneralError; // wait for next upload
        		upload_nand_spare = FALSE; // clear it
        	}
			else // upload failed
			{
				FatalError(fs_Retval.ErrorCode);
			}
			
	        // Does not matter if we had an error. Boot anyway.
	        break;

		default:
			unknown_protocol_command(getProtocolCmd());
			FatalError(UnknownProtocolCmd);
	    }
	}while (fs_Retval.ErrorCode != NoError);

disconnect:
	HandleDisconnect();

shutdown:
    ShutdownPort(CI2_USB_D);

	return pBootImageInfo;
}
