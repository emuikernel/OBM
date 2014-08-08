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
**
**  FILENAME:   OBM.c
**
**  PURPOSE:    OBM main routine
**
**  History:    Initial Creation 4/23/06
******************************************************************************/

//////////////////////////////////////////////////////////////////////
// Include the main BootLoader header file
// IMPORTANT: 
// 		DO NOT include anything from here. Include any header that is 
//		needed in the BootLoader.h file. 
//////////////////////////////////////////////////////////////////////
#include "BootLoader.h"

#if ZIMI_PB05
#include "zimi_bq24295.h"
#endif

#if I2C
#include "I2C.h"

extern UINT8_T external_power;
extern UINT_T ONKey_pressed;
extern UINT8_T battery_connect;
#endif

extern UINT_T isDownload;
extern UINT_T time_count_enable;
extern UINT_T Reset_flag;
extern UINT_T back_image_key;
UINT_T nextImageSize = 0;
UINT_T Jig_status = 0;

#if SBOOT
UINT_T sboot_flag = 0xff;
#endif


void test_led()
{

int start_time =0, cur_time=0;

	#if 0
	//GPIO26
	reg_write((GPIO0_BASE + GPIO_SDR), BIT26);
	reg_write((GPIO0_BASE + GPIO_PSR), BIT26);
	//GPIO27
	reg_write((GPIO0_BASE + GPIO_SDR), BIT27);
	reg_write((GPIO0_BASE + GPIO_PSR), BIT27);
	//GPIO28
	reg_write((GPIO0_BASE + GPIO_SDR), BIT28);
	reg_write((GPIO0_BASE + GPIO_PSR), BIT28);
	//GPIO36
	reg_write((GPIO1_BASE + GPIO_SDR), BIT4);
	reg_write((GPIO1_BASE + GPIO_PSR), BIT4);
	
	//GPIO34
	reg_write((GPIO1_BASE + GPIO_SDR),BIT2);
	reg_write((GPIO1_BASE + GPIO_PSR),BIT2);
	#endif
}

//////////////////////////////////////////////////////////////////////
// This is the entry point for the Boot Loader. 
//
// Inputs: A pointer to the Transfer Struct that is passed in from BootROM.
// Outputs: NONE. Transfers control to OS Loader. 
//
// It mainly functions as following:
//
//	1) Initial Setup
//	2) Validate TIM.
//	3) Perform TIM based setup.
//	4) Determine operating mode.
//		a) If Download Path (SW Upgrade or Download)
//			i.   Download Descriptor File
//			ii.  Validate TIM
//			iii. Download TIM OR FBF Images
//		b) If BOOT Path (Single TIM or Dual TIM boot)
//			i.  Single TIM Boot
//				Configure Flashes and FM
//				Load Images
//				Validate images
//			ii. Dual TIM Boot
//				Configure Flashes and FM
//				Load 2nd TIM
//				Set TIM Pointers
//				Validate 2nd TIM
//				Load Images
//				Validate images
//	5) BL Finalization setup
//	6) Transfer Control		
//////////////////////////////////////////////////////////////////////
void BootLoaderMain( P_TRANSFER_STRUCT pTS_h )
{
    FUSE_SET fuses;                  			// Fuse information passed in from BootROM
    IMAGE_INFO_3_4_0 *pBootImageInfo = NULL;
    UINT_T ErrorCode = NoError;					// Error Code variable
	OPERATING_MODE_T bootMode = UPGRADESW;   	// initalized so that coverity doesn't complain
    UINT_T force_platform_bind = 1;  			// WTM platform bind needs be done at DM stage; when it is set nonezero
                                                // platform lifecycle advance is performed so as to perform platform binding
    UINT_T perform_wtm_ver_adv = 0;  			// Instuct WTM to perform version advance                                    

	// Transfer struct related parameters
	UINT_T startOfDayTimerVal = 0; 				// pTS_h->SOD_ACCR0;
	UINT_T TIM_Address = 0;						// pTS_h->data_pairs[0].location;
	pTIM pTIM_h = GetTimPointer();
	UINT_T i;

	#if ZIMI_PB05
	extern int isInNormalSys;
	#endif
	// Initial setup
	SetupEnvironment(&TIM_Address, pTS_h, &startOfDayTimerVal, pTIM_h, &fuses, &perform_wtm_ver_adv);


	//test_led();
	#if 0
	UINT8_T xyl_val;
	xyl_val = UsticaBaseRead(USTICA_STATUS);
	/*
	if ((xyl & USTICA_ONKEY_STATUS) != USTICA_ONKEY_STATUS)
	{
		return 0;
	}
	*/
	serial_outstr("xyl ONKey status:\n");
	serial_outnum(xyl_val);
	serial_outstr("\n");
	#endif
	// Perform TIM based setup
    PerformTIMBasedSetup(pTIM_h, &fuses);	
	
	// Determine operating mode
	bootMode = DetermineOperatingMode(&fuses, pTIM_h);

	#if SBOOT
	sboot_flag = PlatformCheckSbootMode();
	#endif
	
	switch (bootMode) {
		// DOWNLOAD PATH		
		case UPGRADESW:
		case DOWNLOAD:
			// Download Descriptor File

			pBootImageInfo = DownloadModeMain( &fuses, pTIM_h, bootMode);
			
			break;
		// BOOT PATH
		case SINGLE_TIM_BOOT:
			// Configure Flashes and FM
			// SINGLE BOOT
			//    Load and Validate Images			
		case DUAL_TIM_BOOT:
			// DUAL BOOT
			// 	  Load 2nd TIM from flash
			//    Set TIM Pointers
			//    Validate 2nd TIM
			//    Load and Validate Images	
		#if I2C && !AUTO_BOOTUP
		//if ((!battery_connect) && (!external_power))
		if (((ONKey_pressed == 1) || 
			(Jig_status == 7)) || 
			((Jig_status == 6) && ((battery_connect == 1) || (external_power == 1))))
			serial_outstr("Allow to boot up\n");
		else
		{
			serial_outstr("Not allow to boot up\n");
			#if ZIMI_PB05
			//battery_process_step2();
			#endif
		}
		#endif
			pBootImageInfo = BootModeMain(pTIM_h, bootMode, &fuses);
			break;
		default:
			break;
	}

	// If there is an error or no image is found for booting, we FAIL. 
	if ((pBootImageInfo == NULL) && (!isDownload))
	{
		FatalError(NULLPointer);
    }

	if (isDownload)
	{
		#if I2C 
		#if OLED_SUPPORT || LED_DISPLAY
		serial_outstr("SoftwareUpgrade_Done\n");
		SoftwareUpgrade_Done();
		#endif
		#endif
	}
	// Finalization Setup
	FinalizeSetup(&fuses, pTIM_h);

	#if USE_SERIAL_DEBUG 
	serial_outstr("end...\n");
	#endif

	#if 0
	int start_time,cur_time;
	for(i=0;i<100;i++)
	{
		start_time = GetOSCR0();
			while(1)
			{
				cur_time = GetOSCR0();
				if(OSCR0IntervalInSec(start_time, cur_time) > 3) //wait for 4s
					break;
			}
			serial_outstr("xyl read status");
			int reg = *(volatile unsigned long*)0xd401e14c;
			serial_outnum(reg);
			serial_outstr("\n");
	}
	#endif
	//xyl:after download, we need switch USB to 9531
	//reg_write((GPIO2_BASE + GPIO_PCR), BIT7);
	
	// Transfer control to the next image.
	if ((bootMode == UPGRADESW) || (bootMode == DOWNLOAD))
	{
		// This function needs to be called in Download Mode
		if (isDownload)
		{
			if (Reset_flag == 1)
				do_wdt_reset(); // reset after download as default
			else
			{
				#if ZIMI_PB05
				while(1)  //case: when USB plig out, power off
				{
					if ((*(VUINT_T *)PMUA_SD_ROT_WAKE_CLR) & PMUA_SD_ROT_WAKE_CLR_USB_VBUS_STS)
						continue;
					else
						System_poweroff();
				}
				#endif
				
				ErrorCode = 1;
				
				// Infinite loop!
				while(ErrorCode&1)
				{
					// need debugger to step beyond this loop after inspecting input args...
					ErrorCode+=2;     
				}
			}
		}
		else
		{
		#if 0
			if (external_power)
			{
				battery_process_step4();
			}
		#endif

		#if 0
		if(isInNormalSys)  //case Normal sys, set WDT flag
		{
			zimi_do_wdt_reset();
			serial_outstr("zimi#enable WDT 10s\n");
		}
		#endif
		
		#if DECOMPRESS_SUPPORT
		    TransferControl(pBootImageInfo->LoadAddr, nextImageSize, pBootImageInfo->LoadAddr);	
		#else
			TransferControl(pBootImageInfo->LoadAddr, pBootImageInfo->ImageSize, pBootImageInfo->LoadAddr);
		#endif
		}
	}
	else
	{
		// As we are loading everything to their load addresses, we don't need
		// pass the DDR_DOWNLOAD_AREA_ADDR to the TransferControl function
		// in this case.

	#if 0
		if (external_power)
		{
			battery_process_step4();
		}
	#endif

	#if 0
	if(isInNormalSys)  //case Normal sys, set WDT flag
	{
		zimi_do_wdt_reset();
		serial_outstr("zimi#enable WDT 10s\n");
	}
	#endif

	
	#if DECOMPRESS_SUPPORT
	    TransferControl(pBootImageInfo->LoadAddr, nextImageSize, pBootImageInfo->LoadAddr);	
	#else
		TransferControl(pBootImageInfo->LoadAddr, pBootImageInfo->ImageSize, pBootImageInfo->LoadAddr);
	#endif
	}        																												
}


//////////////////////////////////////////////////////////////////////
// This is the Environment Setup function for the Boot Loader. 
//
// Inputs: Pointer to the address of TIM, Pointer the Transfer Struct 
// coming from the BootROM, Pointer to the Timer value, Pointer to the
// Fuses. 
// Outputs: None 
//
// It mainly functions as following:
//	1) If IPC read is required, gets the transfer struct from IPC. 
//  2) Parses the transfer struct. 		
//	3) Sets the TIM pointers.
//  4) Initializes the SOD Timer interface.
//	5) Enables default clocks.
//	6) Initializes the fuses using platform settings passed in from BootROM. 
//  7) Initializes the Keypad required for SW upgrade. 
//  8) Initializes the Platform Message Queue.
//  9) Init Security API. 
//////////////////////////////////////////////////////////////////////
void SetupEnvironment(UINT_T *TIM_Address, P_TRANSFER_STRUCT pTS_h, UINT_T *startOfDayTimerVal, TIM *pTIM_h, pFUSE_SET pFuses, UINT_T *perform_wtm_ver_adv)
{
	UINT_T PlatformSettings = 0, reg_value = 0;

	ParseTransferStruct(TIM_Address, pTS_h, &PlatformSettings, startOfDayTimerVal);
	
	//*TIM_Address = 0xd1100100;
    SetTIMPointers((UINT8_T*)*TIM_Address, pTIM_h);
    if( pTIM_h->pConsTIM->VersionBind.Identifier != TIMIDENTIFIER )
    {
		// Set error to indicate no TIM is found and make it a fatal error.
        FatalError(TIMNotFound);
    }

    // Initialize the SOD timer interface
    InitSODTimer();

	// Enable the clocks for the peripherals that will be used
    CheckDefaultClocks(&reg_value);

	// serial debug
    #if USE_SERIAL_DEBUG
    PlatformUARTConfig();
    serial_init();
	serial_outstr("start...\n");
	serial_outstr(SWD_VERSION_INFO);
	serial_outstr(OBM_VERSION_INFO);
	serial_outstr(OBM_COMPILE_INFO);
	#endif

	serial_outstr("APB_SPARE1_REG.VREG_DIG[13:12]\n");
	serial_outnum((reg_value >> 12) & 0x3);
	serial_outstr("\n");
	
	#if I2C
	battery_process_step1();
	#endif

	#if ZIMI_LED_SUPPORT
	zimi_led_init();  //xyl    we turn on LED here,because powerup may be called by battery insert , so we must make sure that
	#endif

	#if PRODUCTION_MODE_SUPPORT
	PlatformCheckProductionMode();
	#endif
   
    // Initialize any fuse information passed in by the BootROM
    pFuses->value = PlatformSettings;

	// Necassary fuse overwrites for enabling download. 
	FuseOverwriteForDownload(pFuses);

    // Keypad initialization: Required for detecting software upgrade request from the keypad.
    #if UPDATE_USE_KEY || BACKUP_IMAGE
    PlatformKeypadConfig();
    InitializeKeypad();
	#endif

	#if MIFI3
	PlatformUSBChargeConfig();
	#endif

	#if UPDATE_USE_GPIO
	PlatformGPIOConfig();
	#endif

    // Initialize structures required by the protocol / download manager.
    InitMessageQueue();

	#if ICASE
	PlatformUSBLimitSet();
	#endif
}


//////////////////////////////////////////////////////////////////////
// This is the Parse Transfer Struct function for the Boot Loader. 
//
// Inputs: Pointer to the address of TIM, Pointer the Transfer Struct 
// coming from the BootROM, Pointer to the Timer value, Pointer to the
// Fuses. 
// Outputs: NONE.
//
// It mainly functions as following:
// 	1) If the passed in Transfer Struct is not NULL, parse it and initialize 
// 	   the platform settings, the timer value, and the TIM address. 
//	2) Else if it is NULL, look at a fixed location. 
//////////////////////////////////////////////////////////////////////
void ParseTransferStruct(UINT_T *TIM_Address, P_TRANSFER_STRUCT pTS_h, UINT_T *PlatformSettings, UINT_T *startOfDayTimerVal)
{
	UINT_T i;
	UINT_T *TransferStructArea = (UINT_T *)OBM1_TRANSFER_STRUCT_ADDR;

	// If the passed in Transfer Struct is not NULL, parse it and initialize 
	// the platform settings, the timer value, and the TIM address. 
	if( pTS_h )
	{
		*PlatformSettings = pTS_h->FuseVal;
		*startOfDayTimerVal = pTS_h->SOD_ACCR0;

		// For the case where the TIM is not in TS coming from the IPC, 
		// TS includes random values. In this case, this loop can take a 
		// long time. With MAX_NUMBER_OF_DATA_PAIRS, we put a limit on this. 
		for( i=0; i<pTS_h->num_data_pairs && i<MAX_NUMBER_OF_DATA_PAIRS; i++)
		{
			if(pTS_h->data_pairs[i].data_id == TIM_DATA )
			{
				*TIM_Address = pTS_h->data_pairs[i].location;
				return;
			}
		}
	}

	// If TIM is not found using the passed in Transfer Struct, this means 
	// that the previous image was DKB. In this case, DKB leaves the important 
	// TS variables in the DDR_TRANSFER_STRUCT_ADDR.
	// 
	// Note that: 
	//
	// At DDR_TRANSFER_STRUCT_ADDR --> | SOD Timer Value | 
	//								   |   Fuse Values   | 
	//								   |   TIM Location  |
    *startOfDayTimerVal = *(TransferStructArea++);
	*PlatformSettings = *(TransferStructArea++);		
	*TIM_Address = *TransferStructArea;
}


//////////////////////////////////////////////////////////////////////
// This is the Fatal Error function for the Boot Loader. 
//
// Inputs: Error code and a pointer to the TIM. 
// Outputs: NONE.   
//
// It mainly stays here in an infinite loop.
//////////////////////////////////////////////////////////////////////
void FatalError(UINT_T ErrorCode)
{
	UINT_T odd = 1;
	 
	// Print out why we failed if it is not a port error.
	if (ErrorCode != DownloadPortError)
	{
	   AddMessageError(REPORT_ERROR, ErrorCode);
	}

#if USE_SERIAL_DEBUG
	serial_outstr("fatal error code\n");
	serial_outnum(ErrorCode);
	serial_outstr("\n");
#endif

#if SPI_CODE
	Giga_Disable4BytesMode();
#endif


	#if 0
	extern int isInMiniSys;
	if(isInMiniSys == 0)
	{
		serial_outstr("zimi# force to mini system\n");
		*(VUINT_T *)BACKUP_IMAGES_ADDR = 0x4D494E53;
		do_wdt_reset();
	}
	else
	{
		serial_outstr("zimi# fatal error in mini sys\n");
	}
	#endif
	// Infinite loop!
	while(odd&1)
	{
		// need debugger to step beyond this loop after inspecting input args...
		odd+=2;     
	}

	return;
}


//////////////////////////////////////////////////////////////////////
// This is the function that performs TIM based setup in the Boot Loader. 
//
// Inputs: A pointer to the TIM and a pointer to the Fuses. 
// Outputs: NONE.
//
// It mainly functions as following:
//	1) Configures the operating mode. 
//  2) Configures the DDR. 
//	3) Enables the interrupts.
//  4) Enables the global interrupts. 
//  5) Checks the TIM for port over-writes. If this fails, initializes 
//     the default ports.  
//////////////////////////////////////////////////////////////////////
void PerformTIMBasedSetup(pTIM pTIM_h, pFUSE_SET pFuses)
{
	UINT_T Retval = NoError;

	// Configure DDR if BootROM doesn't configure it
	if (pFuses->bits.DDRInitialized == FALSE)
	{	
	    Retval = CheckAndConfigureDDR(pTIM_h, pFuses);
		if (Retval != NoError)
	    {
			FatalError(Retval);	
		}
	}

    // Interrupts may be required, so enable them.
    // For example, downloading over USB requires interrupts.
    SetInterruptVector( 0x0 );     // point the interrupt exception handler to our routine.
                                                // this allows us to run in-place out of isram
                                                // without having to enable the enable the mmu.
    INT_init();                 // turns off all interrupts...
    EnableIrqInterrupts();      // Enable Interrupts at the core
    IRQ_Glb_Ena();              // Enable Global Ints using the AP Global Interrupt Mask Register	

#if I2C
	PP_Switch(4); // for speeding up
#else
	PP_Switch(2); // for speeding up
#endif

	GetDDRSize();
}


//////////////////////////////////////////////////////////////////////
// This is the Mode Determining function for the Boot Loader. 
//
// Inputs: A pointer to the Fuses and a pointer to the TIM. 
// Outputs: Returns one of the following operationg modes: 
//		UPGRADESW, DOWNLOAD, SINGLE_TIM_BOOT, or DUAL_TIM_BOOT			
//
// It mainly functions as following:
//	1) Sets running as identifier. 
//	2) Check if the HW upgrade request button is pressed. If so, mode = UPGRADESW.
//	3) Else if there is a DKB ID in the TIM, then mode = DOWNLOAD.
//	4) Else if there is a DUAL TIM ID in the TIM, then mode = DUAL_TIM_BOOT.
//	5) Else by default, we have mode = SINGLE_TIM_BOOT.
//////////////////////////////////////////////////////////////////////
OPERATING_MODE_T DetermineOperatingMode(FUSE_SET *pFuses, pTIM pTIM_h)
{
  	OPERATING_MODE_T mode = SINGLE_TIM_BOOT;
  	UINT_T loops, value;
  	UINT gotkey=0, k;

	#if ZIMI_PB05
		//onkey, battery voltage ,charger , reset
	#if BACKUP_IMAGE
	unsigned int Temp;
	*(VUINT_T *)(GPIO0_BASE + GPIO_CDR) |= BIT25;
	Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_CDR);
	Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_PLR);
    for (loops = 0; loops < 100; loops++)
    {
		if(((Temp>>25)&1) == 0)
		{
			gotkey =1;
			break;
		}
	Temp = *(VUINT_T *)(GPIO0_BASE + GPIO_PLR);
    }
	
	#if I2C
	if (gotkey) // Got key pressed
	{
		gotkey = 0; // clear
	
		for (loops = 0; loops < 10; loops++)
		{
			value = UsticaBaseRead(USTICA_STATUS);
			if ((value & USTICA_ONKEY_STATUS) == USTICA_ONKEY_STATUS)
			{
				gotkey = 1; // Got Onkey pressed
				break;
			}
		}
	}
	#endif

    if( gotkey)
    {
        back_image_key = 1;
		serial_outstr("Backup Image Boot Key Detected\n");

		#if PRODUCTION_MODE_SUPPORT
		PlatformClearProductionMode();
		#endif
    }
	else
	{
		serial_outstr("Backup Image Boot Key Not Detected\n");
	}

	//xyl add for early on led
	UINT_T Retval;

	UINT8_T FlashNumber = (pTIM_h->pConsTIM->FlashInfo.BootFlashSign) & 0xFF;
	
	Retval = Configure_Flashes (FlashNumber, BOOT_FLASH);
	if( Retval != NoError)
	   FatalError(Retval);
	
	Retval = firmware_upgrade(pTIM_h);
	serial_outstr("TR069 return\n");
	serial_outnum(Retval);
	serial_outstr("\n");

	if (Retval == NoError)
	{
		PlatformSetTR069Flag();
		do_wdt_reset();
	}

	UINT16_T vbat_mv;
	UINT8_T m_temp = IIC_read(0x07) & 0xff;

	GetBatInstantVolt(&vbat_mv,USB_NotConnect);

	extern UINT16_T gBatVol;
	//global battery volume here
	if(3500<vbat_mv && vbat_mv<4050)
	{
		 m_temp |= 0x20;
		 IIC_write(0x07,m_temp);
		 Delay_us(300*1000);
		GetBatInstantVolt(&gBatVol,USB_NotConnect);
		//ReadBatVolt(&vbat_mv);
		 m_temp &= ~0x20;
		 IIC_write(0x07,m_temp);
	}
	else
	{
		GetBatInstantVolt(&gBatVol,USB_NotConnect);
		//ReadBatVolt(&vbat_mv);
	}
	serial_outstr("zimi# global vbat\n");
	serial_outnum(gBatVol);
	serial_outstr("\n");
	*(VUINT_T *)ZIMI_BATVOL_INIT_FLAG_ADDR = gBatVol;

	extern PowerUPType_t powerup;  
	extern UINT_T upgrade_failed_flag;
	extern int isTempDetect;
	UINT8 isUsbIn =0;
	
	if ((*(VUINT_T *)PMUA_SD_ROT_WAKE_CLR) & PMUA_SD_ROT_WAKE_CLR_USB_VBUS_STS)
		isUsbIn = 1;
if((isUsbIn ==1  && gBatVol>=ZIMI_POWER_UP_VOL_USB) || gBatVol>=ZIMI_POWER_UP_VOL_NOR)   //xyl: early light led depend on
{
	if(powerup == PowerUP_Reset)
	{
		if(isTempDetect)
		{
			bqTempDetect();
		}
		if(back_image_key || upgrade_failed_flag || (zimiCheckBackupImagesFlag() == 0x4D494E53))
		{
			LED_BAT_R_ON();
			LED_WIFI_R_ON();
			LED_LTE_R_ON();
		}
		else
		{
			#if ZIMI_LAST_LED_MODE
			LED_ALL_ON();
			#else
			LED_BAT_G_ON();
			LED_WIFI_G_ON();
			LED_LTE_G_ON();
			#endif
		}
	}
	else if(powerup == PowerUP_USB)
	{
		if(back_image_key || upgrade_failed_flag || (zimiCheckBackupImagesFlag() == 0x4D494E53))	
		{
			LED_BAT_R_ON();
			LED_WIFI_R_ON();
			LED_LTE_R_ON();
		}
		else
		{
			if(gBatVol<3600)
			{
				serial_outstr("zimi# bat led low\n");
				LED_BAT_R_ON();
			}
			else if(gBatVol<3750)
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
	}
}
#endif

	if (FindImageInTIM( pTIM_h, DKBIDENTIFIER ))  //this is empty flash case
    	{
    		serial_outstr("zimi# this is empty flash case\n");
       	 mode = DOWNLOAD;
		 time_count_enable = FALSE;
		 return mode;
	}
	
	UINT8 bqStatus;
	bqStatus = IIC_read(BQ_SYS_STATUS);
	extern int isBqDetected;
	
	if(isUsbIn== 0)	//no usb connect
	{
		serial_outstr("zimi# USB is not conneted\n");
		extern PowerUPType_t powerup;
		if(powerup == PowerUP_ONKEY || powerup == PowerUP_Reset || powerup == PowerUP_USB)
		{
			//GetBatInstantVolt(&vbat_mv, USB_NotConnect);
			if(gBatVol < ZIMI_POWER_UP_VOL_NOR)
			{
				LED_ALL_OFF();
				serial_outstr("zimi# battery too low, pls connect USB\n");
				//notify low batt stat
				LED_BAT_R_ON();
				Delay_us(500000);
				LED_BAT_R_OFF();
				Delay_us(500000);
				LED_BAT_R_ON();
				Delay_us(500000);
				LED_BAT_R_OFF();
				//poweroff
				System_poweroff();
			}
			//no USB connect&& battery level is OK, normal boot case
			return SINGLE_TIM_BOOT;;
		}
		else  //other wake up reason, like battery insert, we need power off
		{
			serial_outstr("zimi# illegal powerup\n");
			System_poweroff();
		}
	}
	serial_outstr("zimi# USB  conneted\n");
	#if 0  //once detect USB, enter download mode
	if(back_image_key)
	#endif
		time_count_enable = TRUE;  //xyl enable time out
		return	UPGRADESW; //USB is connected && back_image_key pressed
	return SINGLE_TIM_BOOT;
	#endif




  	// Wait for a key to be hit. This will take precedence over TIM based operations.
    // Experimentation has shown that a key won't register immediately -
    // - even if it was held down during power up. after about 0x13 loops
    // the key is detected. (that turns out to be about 0x6d3 oscr0 ticks).
    // Use that information to set a limit on the amount of checking.
    // About 100 loops should be fine.
#if UPDATE_USE_KEY || UPDATE_USE_GPIO || UPDATE_USE_ATCMD || UPDATE_USE_DETECT_USB
	mode = SINGLE_TIM_BOOT;
#else
	mode = UPGRADESW;
#endif

	// No user request for software update.
	// Deterimine from TIM headers what to do.
	// If DKB Identifier is found then we have download mode.
    if (FindImageInTIM( pTIM_h, DKBIDENTIFIER ))
    {
        mode = DOWNLOAD;
		time_count_enable = FALSE;
    }
	else
	{
#if UPDATE_USE_KEY
	    for (loops = 0; loops < 100; loops++)
	    {
	        gotkey = ReadKeypad(&k);
	        if( gotkey ) break;
	        if( k != 0xff ) break;
	    }
		
		#if I2C && MIFI_V3R1
		if (gotkey) // Got WPS key pressed
		{
			gotkey = 0; // clear WPS
		
			for (loops = 0; loops < 10; loops++)
			{
				value = UsticaBaseRead(USTICA_STATUS);
				if ((value & USTICA_ONKEY_STATUS) == USTICA_ONKEY_STATUS)
				{
					gotkey = 1; // Got Onkey pressed
					break;
				}
			}
		}
		#endif
		
	    if( gotkey && ( k == KEYID_SOFTWAREUPGRADEREQUEST ) )
	    {
	        mode = UPGRADESW;
			time_count_enable = FALSE;
	    }
	    else
#endif
#if UPDATE_USE_GPIO
		for (loops = 0; loops < 100; loops++)
		{
			gotkey = CheckGPIO();
			if (gotkey)
				break;
		}

		if (gotkey)
		{
			mode = UPGRADESW;
			time_count_enable = FALSE;
		}
		else
#endif
#if UPDATE_USE_ATCMD
		if (*(VUINT_T *)ATCMD_FLAG_ADDR == ATCMD_FLAG_VALID_VALUE)
		{
			*(VUINT_T *)ATCMD_FLAG_ADDR = 0; // clear it
			mode = UPGRADESW;
			time_count_enable = FALSE;
			serial_outstr("Wait for download\n");
		}
		else
#endif
#if UPDATE_USE_DETECT_USB
		if ((*(VUINT_T *)PMUA_SD_ROT_WAKE_CLR) & PMUA_SD_ROT_WAKE_CLR_USB_VBUS_STS)
		{
			mode = UPGRADESW;
			time_count_enable = TRUE; // this is active enumeration by OBM if USB is connect, need to set timeout
			serial_outstr("USB connect\n");
		}
		else
#endif
		if (PlatformCheckForceUSBEnumFlag())
		{
			mode = UPGRADESW;
			time_count_enable = FALSE;
			PlatformClearForceUSBEnumFlag();
			serial_outstr("Force USB download\n");
		}
	}
//onkey, battery voltage ,charger , reset
#if BACKUP_IMAGE
    for (loops = 0; loops < 100; loops++)
    {
        gotkey = ReadKeypad(&k);
        if( gotkey ) break;
        if( k != 0xff ) break;
    }
	
	#if I2C
	if (gotkey) // Got key pressed
	{
		gotkey = 0; // clear
	
		for (loops = 0; loops < 10; loops++)
		{
			value = UsticaBaseRead(USTICA_STATUS);
			if ((value & USTICA_ONKEY_STATUS) == USTICA_ONKEY_STATUS)
			{
				gotkey = 1; // Got Onkey pressed
				break;
			}
		}
	}
	#endif

	serial_outstr("k\n");
	serial_outnum(k);
	serial_outstr("\n");
    if( gotkey && ( k == 0x8 ) )
    {
        back_image_key = 1;
		serial_outstr("Backup Image Boot Key Detected\n");

		#if PRODUCTION_MODE_SUPPORT
		PlatformClearProductionMode();
		#endif
    }
	else
	{
		serial_outstr("Backup Image Boot Key Not Detected\n");
	}
#endif

	
	return mode;
}

//////////////////////////////////////////////////////////////////////
// This is the Finalization Setup function for the Boot Loader. 
//
// Inputs: A pointer to the fuses and a pointer to the TIM.  
// Outputs: NONE. 
//
// It mainly functions as following:
//  1) Shut down all the ports.
//	2) Shuts down the security API. 
//	3) Finalize Flashes. 
//	4) Disable interrupts.
//	5) Restore platform default configuration.  
//////////////////////////////////////////////////////////////////////
void FinalizeSetup(pFUSE_SET pFuses, pTIM pTIM_h )
{
	PlatformProtectBootBlocks();
	
	// Finalize the Flashes. 
    Finalize_Flashes(BOOT_FLASH);    // this will flush the BBT information if necessary.

	// Disable interrupts. 
    DisableIrqInterrupts();

	// Restore the DEFAULT configuration.
	RestoreDefaultConfig();

	PlatformPrepareOBMVersion();
	
	PP_Switch(0); // set PP back
}


//////////////////////////////////////////////////////////////////////
// This is the Setup Transfer Struct function for the Boot Loader. 
//
// Inputs: A pointer to the TIM and Fuse values. 
// Outputs: NONE. 
//
// When transferring from DKB to OBM we need to setup the transfer 
// structure corrrectly. May need to become a platform specific routine 
// if the transfer data is different. 
//////////////////////////////////////////////////////////////////////
void SetupTransferStruct(pTIM pTIM_h, UINT_T FusesValue)
{																	
	// Leave the following in the DDR_TRANSFER_STRUCT_ADDR location 
	// for OBM to read from. 
	//
	// At DDR_TRANSFER_STRUCT_ADDR --> | SOD Timer Value | 
	//								   |   Fuse Values   | 
	//								   |   TIM Location  | 

    UINT_T *TransferStructArea = (UINT_T *)DDR_TRANSFER_STRUCT_ADDR;

    *TransferStructArea++ = GetSODTimerValue();
    *TransferStructArea++ = FusesValue;
    *TransferStructArea = (UINT_T) pTIM_h->pConsTIM;
}

