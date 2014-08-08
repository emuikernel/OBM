/******************************************************************************
 *
 *  (C)Copyright 2013 Marvell. All Rights Reserved.
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

#include "oled.h"
#include "sh1106.h"
#include "Typedef.h"

extern UINT8_T battery_connect;
extern const UINT8_T char611v_I2COLED[96][7][2];
extern const UINT8_T BatteryState[6][64][4];
extern const UINT8_T logo[1][64][4]; // add logo here
extern const UINT8_T USB_Status[1][48][3];
extern const UINT8_T Battery_Status[1][32][4];
extern const UINT8_T External_Power_Status[1][16][4];
extern const UINT8_T download_Status[2][32][4];

UINT8_T sh1106_DataBuf[DATABUF_SIZE_I2COLED] = {0};
UINT8_T sh1106_battery_base_percent = 0xff;
UINT8_T sh1106_battery_display_stage = 0;
UINT8_T sh1106_oled_timeout = 0;
UINT8_T sh1106_oled_unstable = 0;

/*
*
* Control byte
* Bit7	6	5	4	3	2	1	0
*   Co	D/C	0	0	0	0	0	0
* 
* Co - Continuation bit
*		If the Co is set as logic "0", the transmission of the following information
*		will contain data bytes only.
*
* D/C - Data / Command selection bit
*		The D/C# bit determines the next data byte is acted as a command or a data.
*		If the D/C# bit is set to logic "0", it defines the following data byte as a command.
*		If the D/C# bit is set to logic "1", it defines the following data byte as a data
*		which will be stored at the GDDRAM. The GDDRAM column address pointer will be increased
*		by one automatically after each data write.
*
*/

#define Control_Byte_Command		0x00 // Co - 0, D/C - 0
#define Control_Byte_Data			0x40 // Co - 0, D/C - 1

void SH1106_I2COLED_Write_I(UINT8_T value)
{
	UINT_T temp;
	
	// Set I2C address for the following i2c writing for ssd1306
#if AZW_OLED || NEZHA_MIFI_V4R1
	*(VUINT_T*)0xD401701c = 0x78;
#else
	*(VUINT_T*)0xD401701c = 0x7a;
#endif

	temp = BBU_putCI2C(Control_Byte_Command, value);
	
	if (temp == 0)
	{
		sh1106_oled_timeout += 1;
	}

	if (sh1106_oled_timeout > 10)
		sh1106_oled_unstable = 1;
}

void SH1106_I2COLED_Write_D(UINT8_T value)
{
	UINT_T temp;
	
	// Set I2C address for the following i2c writing for ssd1306
#if AZW_OLED || NEZHA_MIFI_V4R1
	*(VUINT_T*)0xD401701c = 0x78;
#else
	*(VUINT_T*)0xD401701c = 0x7a;
#endif

	temp = BBU_putCI2C(Control_Byte_Data, value);
}

void SH1106_I2COLED_Init(void)
{
#if NEZHA_MIFI_V4R1
	SH1106_I2COLED_Wcmd(0xAE);     //Set Display Off 

	SH1106_I2COLED_Wcmd(0xd5);     //display divide ratio/osc. freq. mode	
	SH1106_I2COLED_Wcmd(0x91);     //

	SH1106_I2COLED_Wcmd(0xA8);     //multiplex ration mode:63 
	SH1106_I2COLED_Wcmd(0x1F);

	SH1106_I2COLED_Wcmd(0xD3);     //Set Display Offset   
	SH1106_I2COLED_Wcmd(0x00);

	SH1106_I2COLED_Wcmd(0x40);     //Set Display Start Line 

	SH1106_I2COLED_Wcmd(0xAD);     //DC-DC Control Mode Set 
	SH1106_I2COLED_Wcmd(0x8b);     //DC-DC ON/OFF Mode Set 

	SH1106_I2COLED_Wcmd(0x32);     //Set Pump voltage value 

	SH1106_I2COLED_Wcmd(0xA1);     //Segment Remap	 

	//SH1106_I2COLED_Wcmd(0xC8);     //Sst COM Output Scan Direction	

	SH1106_I2COLED_Wcmd(0xDA);     //common pads hardware: alternative	
	SH1106_I2COLED_Wcmd(0x02);

	SH1106_I2COLED_Wcmd(0x81);     //contrast control 
	SH1106_I2COLED_Wcmd(0x33);		//33

	SH1106_I2COLED_Wcmd(0xD9);	    //set pre-charge period	  
	SH1106_I2COLED_Wcmd(0x1f);

	SH1106_I2COLED_Wcmd(0xDB);     //VCOM deselect level mode 
	SH1106_I2COLED_Wcmd(0x40);	    //

	SH1106_I2COLED_Wcmd(0xA4);     //Set Entire Display On/Off	

	SH1106_I2COLED_Wcmd(0xA6);     //Set Normal Display 

	//SH1106_I2COLED_Wcmd(0xAF);     //Set Display On 
#else
	SH1106_I2COLED_Wcmd(I2COLED_Display_OFF);

	SH1106_I2COLED_Wcmd(0x00);/* set lower column start address */
	SH1106_I2COLED_Wcmd(0x10);/* set higher column start address */

	SH1106_I2COLED_Wcmd(0x40);/* set display start line : 0x40+x_line*/	
	SH1106_I2COLED_Wcmd(0x81);/* set contrast control register */
	SH1106_I2COLED_Wcmd(0x7f);

	SH1106_I2COLED_Wcmd(0xa1);/* set column remap */

	SH1106_I2COLED_Wcmd(0xa6);/* set normal control */

	SH1106_I2COLED_Wcmd(0xa8);/* set multiplex ratio */
	SH1106_I2COLED_Wcmd(63);

	//SH1106_I2COLED_Wcmd(0xc8);/* set com scan direction */

	SH1106_I2COLED_Wcmd(0xd3);/* set display offset */
	SH1106_I2COLED_Wcmd(0x00);
	    
	SH1106_I2COLED_Wcmd(0xd5);/* set display clock divide ratio/oscillator frequancy */
	SH1106_I2COLED_Wcmd(0x80);
	       
	SH1106_I2COLED_Wcmd(0xda);/* set com configuration */
	SH1106_I2COLED_Wcmd(0x12);//0x12 is nomarl

	SH1106_I2COLED_Wcmd(0x8d);/* interal dc-dc */
	SH1106_I2COLED_Wcmd(0x14);
#endif
	
	if (sh1106_oled_unstable == 1)
		serial_outstr("OLED unstable or not exist\n");
	else
		serial_outstr("OLED stable\n");
}

void Clear_Screen(void)
{
	UINT_T page_number,column_number;
	UINT_T total_page = 0, total_column = 0;

#if NEZHA_MIFI_V4R1
	total_page = 4;
	total_column = 130;
#else
	total_page = 8;
	total_column = 128;
#endif

	if (sh1106_oled_unstable == 1)
		return;

	for(page_number = 0; page_number < total_page; page_number++)
	{
		SH1106_I2COLED_Wcmd(0xb0 + page_number);
		SH1106_I2COLED_Wcmd(0x10);

	#if NEZHA_MIFI_V4R1
		SH1106_I2COLED_Wcmd(0x00);
	#else
		SH1106_I2COLED_Wcmd(0x02);
	#endif
		
		for(column_number = 0; column_number < total_column; column_number++)
		{
			SH1106_I2COLED_Wdata(0x00);
		}
	}

	SH1106_I2COLED_Wcmd(I2COLED_Display_ON); // set display on

#if NEZHA_MIFI_V4R1
	SH1106_PowerON();
#endif
}

#if NEZHA_MIFI_V4R1
void SH1106_PowerON(void)
{
	PlatformOLEDPowerON();
}

void SH1106_PowerOFF(void)
{
	PlatformOLEDPowerOFF();
}
#endif

void Test_I2C_OLED(void)
{
	Clear_Screen();
	SH1106_I2COLED_Wcmd(I2COLED_Display_ON);/* set display on  */

	show_logo(0);
	while(1);
}

void DisplayBatstate(UINT8_T batpercent)
{
	UINT_T vbat_mv;
	UINT8_T percent;

	if (sh1106_oled_unstable == 1)
		return;

	GetBatInstantVolt(&vbat_mv, USB_Connect);

	percent = GetBatteryPercent(vbat_mv);
	Get_Dispaly_Percent(percent);
	Battery_Charing_Display(sh1106_battery_base_percent);
}

void No_Battery_Display(void)
{
	if (sh1106_oled_unstable == 1)
		return;
	
	show_battery_status(0);
}

void External_Power_Display(void)
{
	if (sh1106_oled_unstable == 1)
		return;
	
	show_external_power(0);
}

void USB_NotConnect_Display(void)
{
}

void USB_Connect_Display(void)
{
/*
* for OLED at this stage, it's not proper to show USB
*/
	//show_USB_status(0);
}

void SoftwareUpgrade_Start()
{
	if (sh1106_oled_unstable == 1)
		return;

#if NEZHA_MIFI_V4R1
	SH1106_FillArea(0x0, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
#else
	SH1106_FillArea(0x2, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
#endif

	show_Downlaod(0);
}

void SoftwareUpgrade_Done()
{
	if (sh1106_oled_unstable == 1)
		return;
	
	show_Downlaod(1);
}

void SH1106_FillArea(UINT_T page_start,
									UINT_T page_num,
									UINT_T p_coloum,
									UINT_T p_width,
									UINT8_T fill_data,
									UINT_T byte_num)
{
	UINT_T i, j = 0;
	UINT8_T page_st = page_start + 0xb0;
	UINT8_T coloum_st = p_coloum;

	if (sh1106_oled_unstable == 1)
		return;

	for (i = 0; i < page_num; i++)
	{
		SH1106_I2COLED_Wcmd(page_st + i);
		SH1106_I2COLED_Wcmd(0x10 +(((0x02 + coloum_st) >> 0x04) & 0x0F));
		SH1106_I2COLED_Wcmd((0x02 + coloum_st)&0x0F);   

		for (j = 0; j < p_width; j++)
		{
			SH1106_I2COLED_Wdata(fill_data);
		}

	}         
}

void SH1106_PicDisplay(UINT_T page_start,
										UINT_T page_num,
										UINT_T p_coloum,
										UINT_T p_width, 
										UINT8_T *buf,
										UINT_T byte_num)
{                                                                          
	UINT_T i, j = 0;
	UINT8_T page_st = page_start + 0xb0;
	UINT8_T coloum_st = p_coloum;

	if (sh1106_oled_unstable == 1)
		return;

	for (i = 0; i < page_num; i++)
	{
		SH1106_I2COLED_Wcmd(page_st + i);
		SH1106_I2COLED_Wcmd(0x10 +(((0x02 + coloum_st) >> 0x04) & 0x0F));
		SH1106_I2COLED_Wcmd((0x02 + coloum_st)&0x0F);   

		for (j = 0; j < p_width; j++)
		{
			SH1106_I2COLED_Wdata(*(buf + i + j*page_num));
		}
	}
}

void Battery_Charing_Display(UINT8_T battery_stage)
{
	if (sh1106_oled_unstable == 1)
		return;
	
	SH1106_PicDisplay( sh1106_Battery_Charging_Page_Start,
								 sh1106_Battery_Charging_Page_Num,
								 sh1106_Battery_Charging_Coloum,
								 sh1106_Battery_Charging_Width,
								 &BatteryState[battery_stage],
								 sh1106_Battery_Charging_Bytes_Num
								 );
}

UINT8_T Get_Dispaly_Percent(UINT8_T percent)
{
	UINT8_T stage;
	
	if (percent < 5)
	{
		stage = 0;
	}
	else if (percent < 26)
	{
		stage = 1;
	}
	else if (percent < 46)
	{
		stage = 2;
	}
	else if (percent < 66)
	{
		stage = 3;
	}
	else if (percent < 95)
	{
		stage = 4;
	}
	else
	{
		stage = 5;
	}

	if (sh1106_battery_base_percent == 0xff)
	{
		sh1106_battery_base_percent = stage;
		sh1106_battery_display_stage = sh1106_battery_base_percent;
	}

	return stage;
}

void BatCharging_On(void)
{
	UINT_T vbat_mv;
	UINT8_T percent;

	if (sh1106_oled_unstable == 1)
		return;

	GetBatInstantVolt(&vbat_mv, USB_Connect);

	percent = GetBatteryPercent(vbat_mv);
	Get_Dispaly_Percent(percent);
	serial_outstr("vbat_mv\n");
	serial_outnum(vbat_mv);
	serial_outstr("\n");
	serial_outstr("percent\n");
	serial_outnum(percent);
	serial_outstr("\n");

	Battery_Charing_Display(sh1106_battery_base_percent);
}

void Update_Battery_State(UINT16_T vbat_mv)
{
	UINT8_T percent, update_stage;

	if (sh1106_oled_unstable == 1)
		return;

	percent = GetBatteryPercent(vbat_mv);
	update_stage = Get_Dispaly_Percent(percent);

	if (sh1106_battery_display_stage == 5)
		sh1106_battery_display_stage = sh1106_battery_base_percent;
	else
		sh1106_battery_display_stage++;

	Battery_Charing_Display(sh1106_battery_display_stage);

	if (update_stage > sh1106_battery_base_percent)
		sh1106_battery_base_percent = update_stage;
}

void ONKEY_Bootup(void)
{
	if (sh1106_oled_unstable == 1)
		return;

#if NEZHA_MIFI_V4R1
	Clear_Screen();
#endif
	
	SH1106_PicDisplay( sh1106_LOGO0_Page_Start,
								 sh1106_LOGO0_Page_Num,
								 sh1106_LOGO0_Coloum,
								 sh1106_LOGO0_Width,
								 &logo[0],
								 sh1106_LOGO0_Bytes_Num
								 );
}

void show_logo(UINT8_T choice)
{
	if (sh1106_oled_unstable == 1)
		return;

	SH1106_PicDisplay( sh1106_LOGO0_Page_Start,
								 sh1106_LOGO0_Page_Num,
								 sh1106_LOGO0_Coloum,
								 sh1106_LOGO0_Width,
								 &logo[choice],
								 sh1106_LOGO0_Bytes_Num
								 );
}

void show_battery_status(UINT8_T choice)
{
	if (sh1106_oled_unstable == 1)
		return;

#if NEZHA_MIFI_V4R1
	SH1106_FillArea(0x0, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
#else
	SH1106_FillArea(0x2, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
#endif

	SH1106_PicDisplay( sh1106_Battery_status0_Page_Start,
								 sh1106_Battery_status0_Page_Num,
								 sh1106_Battery_status0_Coloum,
								 sh1106_Battery_status0_Width,
								 &Battery_Status[choice],
								 sh1106_Battery_status0_Bytes_Num
								 );
}

void show_external_power(UINT8_T choice)
{
	if (sh1106_oled_unstable == 1)
		return;

#if NEZHA_MIFI_V4R1
	SH1106_FillArea(0x0, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
#else
	SH1106_FillArea(0x2, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
#endif

	SH1106_PicDisplay( sh1106_External_Power_status0_Page_Start,
								 sh1106_External_Power_status0_Page_Num,
								 sh1106_External_Power_status0_Coloum,
								 sh1106_External_Power_status0_Width,
								 &External_Power_Status[choice],
								 sh1106_External_Power_status0_Bytes_Num
								 );
}

void show_USB_status(UINT8_T choice)
{
	if (sh1106_oled_unstable == 1)
		return;

#if NEZHA_MIFI_V4R1
	Clear_Screen();
#endif
	
	SH1106_PicDisplay( sh1106_USB_status0_Page_Start,
								 sh1106_USB_status0_Page_Num,
								 sh1106_USB_status0_Coloum,
								 sh1106_USB_status0_Width,
								 &USB_Status[choice],
								 sh1106_USB_status0_Bytes_Num
								 );
}

void show_Downlaod(UINT8_T choice)
{
	if (sh1106_oled_unstable == 1)
		return;

#if NEZHA_MIFI_V4R1
	Clear_Screen();
#endif
	
	SH1106_PicDisplay( sh1106_Download_Page_Start,
								 sh1106_Download_Page_Num,
								 sh1106_Download_Coloum,
								 sh1106_Download_Width,
								 &download_Status[choice],
								 sh1106_Download_Bytes_Num
								 );
}

void Firmware_Upgrade_Start(void)
{
	SoftwareUpgrade_Start();
}

void Firmware_Upgrade_Done(void)
{
	SoftwareUpgrade_Done();
}

