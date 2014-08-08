/******************************************************************************
 *
 *  (C)Copyright 2005 - 2012 Marvell. All Rights Reserved.
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

#include "I2C.h"

#if ZIMI_PB05
#include "zimi_bq24295.h"
#endif

UINT_T absolute_timer = 0; // absolute timer 
UINT_T resume_timer = 0; // resume-charging absolute timer
UINT_T resume_flag = 2;
UINT_T ONKey_pressed = 0;

extern UINT8_T battery_connect;
extern UINT8_T external_power;
extern UINT_T Jig_status;

UINT8_T GetBatteryPercent(UINT16_T v_bat)
{
	// battery_percent = 100 * ((v_bat - 3400) / (4200 - 3400))
	UINT8_T  battery_percent = (v_bat - Battery_Valid_Voltage) >> 3;

	if (v_bat < Battery_Valid_Voltage)
		battery_percent = 0;

	return battery_percent;
}

void I2cInit()
{
	UINT_T value1;
	UINT8_T i;
				  
	PlatformPI2CConfig();
	PlatformChargerConfig();

#if LED_DISPLAY
	PlatformLEDConfig();
	//testLED();
#endif

#if OLED_SUPPORT
	PlatformOLEDConfig();
#endif

#if RT8973
	PlatformUSBSwitchConfig();
#endif

	Delay_us(100);
	
	*(VUINT_T*)APBC_UART0_CLK_RST = 0x13;

	BBU_PI2C_Init();
	Delay_us(350);

	#if ZIMI_PB05
	PlatfromBqConfig();
	BBU_CI2C_Init();
	Delay_us(350);
	#endif

	Reset_Reg();

	// debug purpose
	//check_SanremoID();
	//check_Fan5405ID();

#if OLED_SUPPORT
	BBU_CI2C_Init();
	Delay_us(350);

	I2COLED_Init();

	Clear_Screen();

	//Test_I2C_OLED();
#endif

#if RT8973
	BBU_CI2C_Init();
	Delay_us(350);

	//check_RT8973ID();
	//check_attached();
#endif

	return;
}

System_Connect_Type_T check_ExternalPower(void)
{
	UINT16_T vbat_mv = 0, temp_bat = 0, current_bat = 0;
	UINT16_T base_voltage = 0;
	System_Connect_Type_T external, battery, usb;
	UINT8_T i, low = 0, high = 0;

	/*
	* The precondition: there is NO battery inserted! Only for test, NOT for end user.
	*
	* Both of USB/charger and external power will supply VBAT, if modify the charger
	* out to the lowest or the highest, but if there is no external power, the voltage
	* of VBAT will reduce or increase immediately.
	*/

	// step 1, get current voltage
	GetBatInstantVolt(&vbat_mv, USB_NotConnect);
	usb = check_USBConnect();

	if (vbat_mv < Battery_Valid_Voltage)
	{
		/*
		* VBAT is supplied by external power or USB, the voltage of VBAT shouldn't lower
		* than 3.4V, otherwise:
		* 1, there is no external power;
		* 2, charger/USB power is invalid, should check why!!!
		*/

		return USB_Invalide_Connect;
	}

	if (usb == USB_NotConnect)
	{
		// no battery, no USB, this is external power
		return External_Power_Connect;
	}

	if ((vbat_mv - Charger_Deafult_Voltage) > (Charger_Maximum_Voltage - vbat_mv))
	{
		// step 2, change charger out voltage to 3.54V
		for (i = 35; i > 0; i--)
			Charger_Voltage_Set(i << 0x2);

		// step 3, get current voltage
		GetBatInstantVolt(&current_bat, USB_NotConnect);
	 
		// step 4, check whether VBAT decrease immediately
		if (current_bat > Charger_Deafult_Voltage)
			temp_bat = current_bat - Charger_Deafult_Voltage;
		else
			temp_bat = Charger_Deafult_Voltage - current_bat;

		if (temp_bat < 60) // margin of error is 60mv
		{
			low = 0;
		}
		else
		{
			low = 1;
		}
	}
	else
	{
		// step 5, change charger out voltage to 4.20V
		for (i = 0; i < 35; i++)
			Charger_Voltage_Set(i << 0x2);

		// step 6, get current voltage
		GetBatInstantVolt(&current_bat, USB_NotConnect);

		// step 7, check whether VBAT decrease immediately
		if (current_bat > Charger_Maximum_Voltage)
			temp_bat = current_bat - Charger_Maximum_Voltage;
		else
			temp_bat = Charger_Maximum_Voltage - current_bat;

		if (temp_bat < 60) // margin of error is 60mv
		{
			high = 0;
		}
		else
		{
			high = 1;
		}
	}

	if (low || high)
	{
		external = External_Power_Connect;
	}
	else
	{
		external = External_Power_NotConnect;
	}

	// step 8, look up the voltage before decreasing
	if ((vbat_mv - Charger_Deafult_Voltage) > (Charger_Maximum_Voltage - vbat_mv))
	{
		for (i = 0; i < 35; i++)
			Charger_Voltage_Set(i << 0x2);
	}
	else
	{
		for (i = 35; i > 0; i--)
			Charger_Voltage_Set(i << 0x2);
	}

	return external;
}

System_Connect_Type_T check_Battery_Status(void)
{
	UINT16_T vbat_mv = 0, temp_bat = 0, current_bat = 0;
	UINT16_T base_voltage = 0;
	System_Connect_Type_T external, battery;
	UINT8_T i, low = 0, high = 0;

	// step 1, get current voltage
	GetBatInstantVolt(&vbat_mv, USB_NotConnect);
	if ((vbat_mv < Charger_Deafult_Voltage) || (vbat_mv > Battery_Full_Voltage))
	{
		external = Battery_Connect;
		return external;
	}

	if ((vbat_mv - Charger_Deafult_Voltage) > (Charger_Maximum_Voltage - vbat_mv))
	{
		// step 2, change charger out voltage to 3.54V
		for (i = 35; i > 0; i--)
			Charger_Voltage_Set(i << 0x2);

		// step 3, get current voltage
		GetBatInstantVolt(&current_bat, USB_Connect);
	 
		// step 4, check whether VBAT decrease immediately
		if (current_bat > Charger_Deafult_Voltage)
			temp_bat = current_bat - Charger_Deafult_Voltage;
		else
			temp_bat = Charger_Deafult_Voltage - current_bat;

		if (temp_bat < 100) // margin of error is 60mv
		{
			low = 0;
		}
		else
		{
			low = 1;
		}
	}
	else
	{
		// step 5, change charger out voltage to 4.20V
		for (i = 0; i < 35; i++)
			Charger_Voltage_Set(i << 0x2);

		// step 6, get current voltage
		GetBatInstantVolt(&current_bat, USB_Connect);

		// step 7, check whether VBAT decrease immediately
		if (current_bat > Charger_Maximum_Voltage)
			temp_bat = current_bat - Charger_Maximum_Voltage;
		else
			temp_bat = Charger_Maximum_Voltage - current_bat;

		if (temp_bat < 100) // margin of error is 60mv
		{
			high = 0;
		}
		else
		{
			high = 1;
		}
	}

	if (low || high)
	{
		external = Battery_Connect;

		// step 8, look up the voltage before decreasing
		if ((vbat_mv - Charger_Deafult_Voltage) > (Charger_Maximum_Voltage - vbat_mv))
		{
			for (i = 0; i < 0x2b; i++)
				Charger_Voltage_Set(i << 0x2);
		}
		else
		{
			for (i = 35; i < 0x2b; i++)
				Charger_Voltage_Set(i << 0x2);
		}
	}
	else
	{
		external = Battery_NotConnect;

		// step 8, look up the voltage before decreasing
		if ((vbat_mv - Charger_Deafult_Voltage) > (Charger_Maximum_Voltage - vbat_mv))
		{
			for (i = 0; i < 35; i++)
				Charger_Voltage_Set(i << 0x2);
		}
		else
		{
			for (i = 35; i > 0; i--)
				Charger_Voltage_Set(i << 0x2);
		}
	}

	return external;
}

void battery_process_step4(void)
{
#if LED_DISPLAY
	UINT8_T i;
	
	for (i = 0; i < 3; i++)
	{
		External_Power_Display();
		Delay_us(1000 * 1000);
		External_Power_Off();
		Delay_us(1000 * 1000);
	}
#endif
}

void battery_process_step3(void)
{
	#if ONLY_DISABLE_WD
	//serial_outstr("Notion PCIe\n");
	return;
	#endif

#if OLED_SUPPORT || LED_DISPLAY
	System_Connect_Type_T USB;

	if (ONKey_pressed == 0)
	{
		USB = check_USBConnect();
		if (USB == USB_Connect)
			USB_Connect_Display();

		if (USB == USB_NotConnect)
			USB_NotConnect_Display();
	}
#endif
}

void battery_process_step2(void)
{
	System_Connect_Type_T external, battery, USB;
	UINT8_T PWR_LED_blinking = 1, detect_boot = 0, no_battery = 1;
	UINT16_T vbat_mv, update_flag = 0;

	#if ONLY_DISABLE_WD
	//serial_outstr("Notion PCIe\n");
	return;
	#endif

	while (1)
	{
		resetTimer(); // reset charger 32s timer for this loop
		
		battery = check_BatteryConnect(); // if battery is connect, change PWR LED red solid
		if (battery == Battery_Connect)
		{
			PWR_LED_blinking = 0;

		#if OLED_SUPPORT
			no_battery = 0;
			update_flag++;
			DisplayBatstate(0);
		#endif
		}
		else
		{
			PWR_LED_blinking = 1;

		#if OLED_SUPPORT
			no_battery = 1;
			update_flag = 0;
		#endif

			if (external_power == 0)
			{
				external = check_ExternalPower();
				if (external == External_Power_Connect)
				{
				#if OLED_SUPPORT
					External_Power_Display();
				#endif

					external_power = 1;
				
					//return;
				}
			}
		}

		USB = check_USBConnect(); // if USB is plug out, power off
		if (USB == USB_NotConnect)
		{
			System_poweroff();
		}
		else
		{
		#if OLED_SUPPORT
			if ((battery == Battery_Connect) && (update_flag == 2))
			{
				// get current battery voltage
				GetBatInstantVolt(&vbat_mv, USB);
				Update_Battery_State(vbat_mv);
				update_flag = 0;
			}
		#endif
		}

		detect_boot = check_BootONKey(3, 1); // check 3s onkey
		if (detect_boot)
		{
			GetBatInstantVolt(&vbat_mv, USB_NotConnect);
			if ((vbat_mv > Battery_Low_Voltage) && ((battery == Battery_Connect) || (external_power == 1)))
			{
			#if OLED_SUPPORT
				ONKEY_Bootup();
			#endif
				return;
			}
			else
			{
			#if LED_DISPLAY
				ClosePowerOnBoot();
			#endif
				
				if (vbat_mv < Battery_Low_Voltage)
					serial_outstr("Not allow board to boot up: Low battery voltage\n");

				if ((battery == Battery_NotConnect) || (external_power == 0))
					serial_outstr("Not allow board to boot up: No battery or No external power\n");
			}
		}

	#if OLED_SUPPORT || LED_DISPLAY
		if (no_battery)
		{
			No_Battery_Display();
			Delay_us(1000 * 500);
		}

		if (PWR_LED_blinking)
		{
			No_Battery_Off();
			Delay_us(1000 * 1000);
		}
	#endif

	}
}

UINT8_T battery_full()
{
	UINT8_T detect_boot = 0;
	System_Connect_Type_T usb;
	
	serial_outstr("charging is done!\n");

	/*
	* 1, the charger/USB might not be plugged out after the battery is full charged,
	* and the charger/USB is wakeup source, if OBM power down the system after
	* full charged, it will wake up the system again...
	*
	* 2, if the charge function is disabled after full charged, the system power will
	* be supplied from battery, it's not acceptable since the battery power will be
	* exhausted...
	*
	* 3, the charger/USB can supply power to system too, and the charge function uses
	* the same HW line connection as power supply function on Nezha LET MIFI 2.0,
	* so, OBM will keep charge function, and let the charger/USB supply the system power.
	*/

	//DisableCharger();

	// PWR LED (green) to indicate charge full
	#if OLED_SUPPORT || LED_DISPLAY
	BatCharging_Full();
	#endif

	while (1)
	{
		/*
		* Normal power supply of USB/charger is controlled by the host with the t32s timer
		* running to ensure that the host is alive. When the timer times out, the power
		* supply is terminated.
		*/
		resetTimer();
		
		detect_boot = check_BootONKey(3, 1);
		if (detect_boot)
		{
			serial_outstr("onkey pressed, boot\n");
			return 2; // charge full
		}
		
		Delay_us(1000 * 1000);
		usb = check_USBConnect();

		if (usb == USB_NotConnect)
		{
			serial_outstr("charger is plugged out!\n");
        	System_poweroff( );
		}
	}
}

void battery_timer(vod)
{
	UINT_T current_timer1, current_timer2;
	UINT_T timer1, timer2;

	if (resume_flag == 0)
	{
		current_timer1 = GetOSCR0();
		timer1 = OSCR0IntervalInSec(absolute_timer, current_timer1);
		if (timer1 == Hours_6)
		{
			serial_outstr("6H absolute timer\n");
			DisableCharger();

			resume_flag = 1; // mark
			resume_timer = GetOSCR0(); // resume-charging absolute timer starts to time
		}
	}
	else if (resume_flag == 1)
	{
		current_timer2 = GetOSCR0();
		timer2 = OSCR0IntervalInSec(resume_timer, current_timer2);
		if (timer2 == Hours_2)
		{
			serial_outstr("2H resume-charging absolute timer\n");
			EnableCharger();

			resume_flag = 0; // clear
			absolute_timer = GetOSCR0(); // absolute timer starts to time
		}
	}
}

void check_battery(void)
{
	/*
	* check whether the battery is valid or not
	*/

	System_Connect_Type_T battery;

	battery = check_Battery_Status();
	if (battery == Battery_Connect) // it's a valid battery
	{
		serial_outstr("Valid battery connect\n");
		return;
	}
	else
	{
		serial_outstr("Invalid battery connect\n");

		while (1)
		{
		#if OLED_SUPPORT || LED_DISPLAY
			External_Power_Display();
			Delay_us(1000 * 1000);
			External_Power_Off();
			Delay_us(1000 * 1000);
		#endif
		}
	}
}

void battery_charge(void)
{
	UINT8_T charger_skipped = 0;
	UINT16_T vbat_mv;
	System_Connect_Type_T battery, usb;
	UINT_T i = 0;

	check_battery();

	// enable charger, and charge start...
	EnableCharger();

	absolute_timer = GetOSCR0(); // absolute timer starts to time
	resume_flag = 0; // init
		
	while (charger_skipped == 0)
	{
		// charge interval is 1s
		Delay_us(1000 * 1000);

		battery_timer();

	#if MIFI_V2R1 || MIFI_V3R0 || MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
		HandleTempVoltageThreshold();
	#endif

		// check whether USB is still connectted or not
		usb = check_USBConnect();
		
		// get current battery voltage
		GetBatInstantVolt(&vbat_mv, usb);

		#if OLED_SUPPORT
		if (i == 2)
		{
			Update_Battery_State(vbat_mv);
			i = 0;
		}
		else
			i++;
		#endif

		// check whether battery is still connectted or not
		battery = check_BatteryConnect();

		// if USB is not connectted when charge is in process, power down system
		if (usb == USB_Connect)
		{
			/*
			* At the beginning of charging, the IC starts a 15 minutes timer. When this timer is
			* times out, charging is terminated. Writing to any register through I2C stops and
			* resets the timer, which in turn starts a 32 seconds timer. Setting the TMR_RST bit
			* (REG0[7]) resets the new timer. If the new timer times out, charging is terminated,
			* the registers are set to their default values, and charging resumes using the default
			* value with the 15 minutes timer running.
			*
			* Normal charging is controlled by the host with the 32s timer running to ensure that
			* the host is alive.
			*/
			resetTimer();

			// battery is not present, blink PWR LED (red)
			if (battery == Battery_NotConnect)
			{
				charger_skipped = 1;
				battery_connect = 0;
			}
			else
			{
				if (vbat_mv < Battery_Low_Voltage)
				{
					/*
					* System is forbidden to boot up if battery voltage is lower than power up voltage,
					* it has to wait for OBM charging battery to enough voltage
					*/
					continue;
				}
				else if ((vbat_mv < Battery_Full_Voltage) && (vbat_mv > Battery_Low_Voltage))
				{
					/*
					* Battery voltage is enough for system requirement, and system is permitted to boot up
					* if ONKey press longer than 3s
					*/
					charger_skipped = check_BootONKey(3, 1);
				}
				else if (vbat_mv > Battery_Full_Voltage)
				{
					// Once the battery is charged full, no charge any more
					charger_skipped = battery_full();
				}

				if ((charger_skipped == 1) || (charger_skipped == 2))
				{
					GetBatInstantVolt(&vbat_mv, usb);
					usb = check_USBConnect();

				#if 0 // OLED_SUPPORT || LED_DISPLAY
					if (usb == USB_Connect)
					{
						if (charger_skipped == 2) // battery full and ONKey
						{
							//BatCharging_Full();
							ONKEY_Bootup();
						}
						else
						{
							//BatCharging_In_Process();
							ONKEY_Bootup();
						}
					}
					else
					{
						DisplayBatstate(GetBatteryPercent(vbat_mv));
					}
				#endif
				}
			}
		}
		else
		{
			serial_outstr("charger is plugged out!\n");
        	System_poweroff( );
		}
	}
}

PowerUPType_t powerup;
void battery_process_step1(void)
{
	System_Connect_Type_T bat_connect_state, usb_connect_state;
	UINT8_T bootup = 0;
	UINT16_T vbat_mv;
	//xyl del
	//PowerUPType_t powerup;
	System_Connect_Type_T external;
	UINT_T poweroff = 0;

	// I2C init
	I2cInit();
	
#if ONLY_DISABLE_WD
	//serial_outstr("Notion PCIe\n");
	return;
#endif

	poweroff = Enable_SMPL();

	// configrations
	Charger_init();

	#if ZIMI_PB05
	extern int isTempDetect;
	char ret_val;
	*(VUINT *)0xD401E140 = 0xd000;		//GPIO25 pull up
	ret_val  = zimiInitBqChip();
	if(ret_val)
	{
		isBqDetected = 0;
	}
	if(isBqDetected)
	{
		if(gChipType == PB_CHIP_TYPE_BQ24295)
			*(VUINT_T *)ZIMI_BQ_HARDWARE_VER = 0x02040209;
		else
			*(VUINT_T *)ZIMI_BQ_HARDWARE_VER = 0x02040109;

		if ((*(VUINT_T *)PMUA_SD_ROT_WAKE_CLR) & PMUA_SD_ROT_WAKE_CLR_USB_VBUS_STS)
		{
			isTempDetect =0;
		}
		else
		{
			isTempDetect =1;
		}
	}
	else
	{
		isTempDetect =0;
		*(VUINT_T *)ZIMI_BQ_HARDWARE_VER = 0x4E4F4251;  //NOBQ
	}
	#endif

	bat_connect_state = Battery_Connect;	 //xyl we force batt_connect state
	usb_connect_state = check_USBConnect();
	if (bat_connect_state == Battery_Connect)
	{
		battery_connect = 1;
	}
	else
	{
		battery_connect = 0;
	}

	// confige chager according to USB connect
	/*xyl:we do NOT need this*/
	//ConfigCharger(battery_connect);

	// get why we are here
	powerup = check_wakeup(battery_connect);
	
	#if ZIMI_PB05

	if(powerup == PowerUP_USB)
	{
		UINT8_T onkey_short_detect;
	
		onkey_short_detect = check_BootONKey(10, 0);
		if(onkey_short_detect)
		{
			serial_outstr("zimi# short onkey detect\n");
			powerup = PowerUP_ONKEY;
		}
	}
	
	switch(powerup)
	{
		case PowerUP_USB:
			*(VUINT_T *)ZIMI_ONKEY_LONG_PRESS_FLAG_ADDR= 0x55534241;//USBA
			break;
		case PowerUP_ONKEY:
			*(VUINT_T *)ZIMI_ONKEY_LONG_PRESS_FLAG_ADDR= 0x4F4E4B45;//ONKE
			break;
		case PowerUP_Reset:
			*(VUINT_T *)ZIMI_ONKEY_LONG_PRESS_FLAG_ADDR= 0x52455354;//REST
			break;
		default:
			*(VUINT_T *)ZIMI_ONKEY_LONG_PRESS_FLAG_ADDR= 0x4F4E4B45;//ONKE
			break;
	}
	*(VUINT_T *)ZIMI_ONKEY_LONG_PRESS_FLAG_ADDR= 0x4F4E4B45;//ONKE
	*(VUINT_T *)ZIMI_OBM_VERSION_FLAG_ADDR = ZIMI_OBM_VER_INFO;          //xyl:pass OBM version to cp
	serial_outstr("zimi OBM ver: \n");
	serial_outnum(ZIMI_OBM_VER_INFO);
	serial_outstr("\n");
	return;
	#endif
	
	if (powerup == PowerUP_JIG)
		Jig_status = 7;

	//if ((powerup == PowerUP_Reset) && (PlatformCheckResetFlag() == 0xfefefefe))
	if (powerup == PowerUP_Reset)
		Jig_status = 6;
#if 0

	// if there is no battery connect, it's for debug purpose
	if (bat_connect_state == Battery_NotConnect)
	{
		if ((powerup != PowerUP_JIG) && (powerup != PowerUP_External)) // Jig is one kind of external power
		{
			external = check_ExternalPower();
			if (external == External_Power_Connect)
			{
				serial_outstr("external power\n");
				external_power = 1;

			#if OLED_SUPPORT || LED_DISPLAY
				External_Power_Display();
			#endif

				//DisableCharger();
				//return;
			}
			else
				serial_outstr("NO external power\n");
		}
		else
		{
			if (powerup == PowerUP_JIG)
				serial_outstr("Jig power\n");

			if (powerup == PowerUP_External)
				serial_outstr("external power\n");
		}
	}

	// battery wakeup is forbidden to boot up
	if ((powerup == PowerUP_Battery) && (bat_connect_state == Battery_Connect))
	{
		//if ((poweroff == 1) && (external_power == 0))
		if (poweroff == 1)
			System_poweroff();
		else
		{
			// if it comes to here, it means SMPL
			serial_outstr("SMPL battery wakeup\n");
		}
	}

	if (powerup == PowerUP_ONKEY)
	{
		// if battery is low voltage, no allow to boot up even ONKey pressed
		GetBatInstantVolt(&vbat_mv, usb_connect_state);
		if (vbat_mv < Battery_Low_Voltage)
		{
			System_poweroff();
		}

		#if 0 //xyl: once we detect Onkey pressed, just power on for time-consuming
		bootup = check_BootONKey(3, 1);
		#else
		bootup = 1;
		ONKey_pressed = 1;
		#endif
		
		if (bootup)
		{
			// in case of battery in low voltage
			GetBatInstantVolt(&vbat_mv, USB_NotConnect);
			if (vbat_mv < Battery_Low_Voltage)
			{
				System_poweroff();
			}

		#if OLED_SUPPORT || LED_DISPLAY
			if (ONKey_pressed == 0)
				DisplayBatstate(GetBatteryPercent(vbat_mv));
		#endif
			return;
		}
		else
		{
			System_poweroff();
		}
	}

	// battery flow
	if (bat_connect_state == Battery_Connect)
	{
		usb_connect_state = check_USBConnect();
		GetBatInstantVolt(&vbat_mv, usb_connect_state);

		// if battery voltage is not enough to boot up, power down
		if ((usb_connect_state == USB_NotConnect) && (vbat_mv < Battery_Low_Voltage))
		{
			serial_outstr("Baterry in Low_voltage,plese insert Charger!\n");
			System_poweroff();
		}

	#if OLED_SUPPORT || LED_DISPLAY
		if ((usb_connect_state == USB_Connect) && (ONKey_pressed == 0))
		{
			BatCharging_In_Process();
		}
	#endif

		// charge flow
		if ((powerup == PowerUP_USB) || (powerup == PowerUP_Charger) || (vbat_mv < Battery_Low_Voltage))
		{
			/*xyl del: we do NOT need this*/
			/*battery_charge();*/
		}
	}
	else
	{
	#if OLED_SUPPORT || LED_DISPLAY
		serial_outstr("Battery Display Off for No battery\n");
		BatState_Disoff();
	#endif
	}
	
	#if OLED_SUPPORT
	show_logo(0);
	#endif
#endif
}

