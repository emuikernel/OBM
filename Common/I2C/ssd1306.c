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
#include "ssd1306.h"
#include "Typedef.h"

extern const UINT8_T char611v_I2COLED[96][7][2];
extern const UINT8_T BatteryState[6][64][4];
extern const UINT8_T logo[1][64][4]; // add logo here
extern const UINT8_T USB_Status[1][48][3];
extern const UINT8_T Battery_Status[1][32][4];
extern const UINT8_T External_Power_Status[1][16][4];
extern const UINT8_T download_Status[2][32][4];

UINT8_T ssd1306_DataBuf[DATABUF_SIZE_I2COLED] = {0};
UINT8_T ssd1306_battery_base_percent = 0xff;
UINT8_T ssd1306_battery_display_stage = 0;
UINT8_T ssd1306_oled_timeout = 0;
UINT8_T ssd136_oled_unstable = 0;

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

void SSD1306_I2COLED_Write_I(UINT8_T value)
{
	UINT_T temp;
	
	// Set I2C address for the following i2c writing for ssd1306
	*(VUINT_T*)0xD401701c = 0x7a;

	temp = BBU_putCI2C(Control_Byte_Command, value);
	
	if (temp == 0)
	{
		ssd1306_oled_timeout += 1;
	}

	if (ssd1306_oled_timeout > 10)
		ssd136_oled_unstable = 1;
}

void SSD1306_I2COLED_Write_D(UINT8_T value)
{
	UINT_T temp;
	
	// Set I2C address for the following i2c writing for ssd1306
	*(VUINT_T*)0xD401701c = 0x7a;

	temp = BBU_putCI2C(Control_Byte_Data, value);
}

void SSD1306_I2COLED_Init(void)
{
	SSD1306_I2COLED_Wcmd(I2COLED_Display_OFF);
	
	SSD1306_I2COLED_Wcmd(0x00);/* set lower column start address */
    SSD1306_I2COLED_Wcmd(0x10);/* set higher column start address */

    SSD1306_I2COLED_Wcmd(0x40);/* set display start line : 0x40+x_line*/	
    SSD1306_I2COLED_Wcmd(0x81);/* set contrast control register */
    SSD1306_I2COLED_Wcmd(0x7f);
    
    SSD1306_I2COLED_Wcmd(0xa1);/* set column remap */
    
    SSD1306_I2COLED_Wcmd(0xa6);/* set normal control */
    
    SSD1306_I2COLED_Wcmd(0xa8);/* set multiplex ratio */
	SSD1306_I2COLED_Wcmd(63);
   
    //SSD1306_I2COLED_Wcmd(0xc8);/* set com scan direction */
    
    SSD1306_I2COLED_Wcmd(0xd3);/* set display offset */
    SSD1306_I2COLED_Wcmd(0x00);
        
    SSD1306_I2COLED_Wcmd(0xd5);/* set display clock divide ratio/oscillator frequancy */
    SSD1306_I2COLED_Wcmd(0x80);
           
    SSD1306_I2COLED_Wcmd(0xda);/* set com configuration */
	SSD1306_I2COLED_Wcmd(0x12);//0x12 is nomarl
    
    SSD1306_I2COLED_Wcmd(0x8d);/* interal dc-dc */
    SSD1306_I2COLED_Wcmd(0x14);

	SSD1306_Cmd_1Para(ScanModeSet, SCAN_In_vertical);
	
	//SSD1306_I2COLED_Wcmd(I2COLED_Display_ON);/* set display on  */

	if (ssd136_oled_unstable == 1)
		serial_outstr("OLED unstable or not exist\n");
	else
		serial_outstr("OLED stable\n");
}

void SSD1306_SetCollomAddress(UINT8_T addr_start, UINT8_T addr_end)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	SSD1306_I2COLED_Wcmd(0x21);
	SSD1306_I2COLED_Wcmd(addr_start);
	SSD1306_I2COLED_Wcmd(addr_end);
}

void Clear_Screen(void)
{
	UINT_T i, bytes_number = 128 * 8;

	if (ssd136_oled_unstable == 1)
		return;

	SSD1306_SetCollomAddress(0x00, 0x7f);
	for (i = 0; i < bytes_number; i++)
	{
		SSD1306_I2COLED_Wdata(0x00);
	}

	SSD1306_I2COLED_Wcmd(I2COLED_Display_ON); // set display on
}

void SSD1306_DisDefaultLine(UINT_T page_start, UINT_T page_num, UINT_T p_coloum, UINT8_T *str)
{
	UINT_T i;
	UINT_T index,size;
	UINT_T char_index;

	UINT8_T * char_buf;

	if (ssd136_oled_unstable == 1)
		return;

	/**** (2)Map lib to Color Data ****/
	index = 0;
	while((index<DATABUF_SIZE_I2COLED)&&(*str!=0))
	{	
		char_index = *str -' ';
		char_buf=&char611v_I2COLED[char_index];
		for(i=0;i<char611v_CharSize;i++)
		{
			ssd1306_DataBuf[index++] = (*char_buf++);
		}
		
		//ssd1306_DataBuf[index++] =0x00; //add space
		//ssd1306_DataBuf[index++] =0x00; //add space
		str++;
	}

	/**** (3) write Data  ****/
	size = index;

	//(1)set page limit
	SSD1306_I2COLED_Wcmd(0x22);
	SSD1306_I2COLED_Wcmd(page_start);
	SSD1306_I2COLED_Wcmd((page_start + page_num-1));

	//(2)set column limit
	SSD1306_I2COLED_Wcmd(0x21);
	SSD1306_I2COLED_Wcmd(p_coloum);
	SSD1306_I2COLED_Wcmd((p_coloum + (size>>1)-1));

	//(3)send data
	for (i = 0; i < size; i++)
	{	
		SSD1306_I2COLED_Wdata(ssd1306_DataBuf[i]);
	}	
}

void Test_I2C_OLED(void)
{
	Clear_Screen();
	SSD1306_I2COLED_Wcmd(I2COLED_Display_ON);/* set display on  */

	SSD1306_DisDefaultLine(0, 2,0, "ABCDEFGHIJKLMNO");
	SSD1306_DisDefaultLine(2, 2,0, "abcdefghijklmnopq");
	SSD1306_DisDefaultLine(4, 2,0, "BCDEFGHIJKLMNO");
	SSD1306_DisDefaultLine(6, 2,0, "UVWXYZxyz!%");

	while(1);
}

void DisplayBatstate(UINT8_T batpercent)
{
	UINT_T vbat_mv;
	UINT8_T percent;

	if (ssd136_oled_unstable == 1)
		return;

	GetBatInstantVolt(&vbat_mv, USB_Connect);

	percent = GetBatteryPercent(vbat_mv);
	Get_Dispaly_Percent(percent);
	Battery_Charing_Display(ssd1306_battery_base_percent);
}

void No_Battery_Display(void)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	show_battery_status(0);
}

void External_Power_Display(void)
{
	if (ssd136_oled_unstable == 1)
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
	if (ssd136_oled_unstable == 1)
		return;
	
	SSD1306_FillArea(0x2, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
	show_Downlaod(0);
}

void SoftwareUpgrade_Done()
{
	if (ssd136_oled_unstable == 1)
		return;
	
	show_Downlaod(1);
}

void SSD1306_FillArea(UINT_T page_start,
									UINT_T page_num,
									UINT_T p_coloum,
									UINT_T p_width,
									UINT8_T fill_data,
									UINT_T byte_num)
{
	int i;

	if (ssd136_oled_unstable == 1)
		return;

	//(1)set page limit
	SSD1306_I2COLED_Wcmd(0x22);
	SSD1306_I2COLED_Wcmd(page_start);
	SSD1306_I2COLED_Wcmd((page_start+page_num-1));

	//(2)set column limit
	SSD1306_I2COLED_Wcmd(0x21);
	SSD1306_I2COLED_Wcmd(p_coloum);
	SSD1306_I2COLED_Wcmd((p_coloum+p_width-1));

	//(3)send data
	for(i=0;i<byte_num;i++)
	{	
		SSD1306_I2COLED_Wdata(fill_data);
	}	
}

void SSD1306_PicDisplay( UINT_T page_start,
										UINT_T page_num,
										UINT_T p_coloum,
										UINT_T p_width,
										UINT8_T *buf,
										UINT_T byte_num)
{
	UINT_T i;

	if (ssd136_oled_unstable == 1)
		return;

	//(1)set page limit
	SSD1306_I2COLED_Wcmd(0x22);
	SSD1306_I2COLED_Wcmd(page_start);
	SSD1306_I2COLED_Wcmd((page_start+page_num-1));

	//(2)set column limit
	SSD1306_I2COLED_Wcmd(0x21);
	SSD1306_I2COLED_Wcmd(p_coloum);
	SSD1306_I2COLED_Wcmd((p_coloum+p_width-1));

	//(3)send data
	for (i = 0; i < byte_num; i++)
	{	
		SSD1306_I2COLED_Wdata((*buf++));
	}	
}

void Battery_Charing_Display(UINT8_T battery_stage)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	SSD1306_PicDisplay( ssd136_Battery_Charging_Page_Start,
								 ssd136_Battery_Charging_Page_Num,
								 ssd136_Battery_Charging_Coloum,
								 ssd136_Battery_Charging_Width,
								 &BatteryState[battery_stage],
								 ssd136_Battery_Charging_Bytes_Num
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

	if (ssd1306_battery_base_percent == 0xff)
	{
		ssd1306_battery_base_percent = stage;
		ssd1306_battery_display_stage = ssd1306_battery_base_percent;
	}

	return stage;
}

void BatCharging_On(void)
{
	UINT_T vbat_mv;
	UINT8_T percent;

	if (ssd136_oled_unstable == 1)
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

	Battery_Charing_Display(ssd1306_battery_base_percent);
}

void Update_Battery_State(UINT16_T vbat_mv)
{
	UINT8_T percent, update_stage;

	if (ssd136_oled_unstable == 1)
		return;

	percent = GetBatteryPercent(vbat_mv);
	update_stage = Get_Dispaly_Percent(percent);

	if (ssd1306_battery_display_stage == 5)
		ssd1306_battery_display_stage = ssd1306_battery_base_percent;
	else
		ssd1306_battery_display_stage++;

	Battery_Charing_Display(ssd1306_battery_display_stage);

	if (update_stage > ssd1306_battery_base_percent)
		ssd1306_battery_base_percent = update_stage;
}

void ONKEY_Bootup(void)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	SSD1306_PicDisplay( ssd136_LOGO0_Page_Start,
								 ssd136_LOGO0_Page_Num,
								 ssd136_LOGO0_Coloum,
								 ssd136_LOGO0_Width,
								 &logo[0],
								 ssd136_LOGO0_Bytes_Num
								 );
}

void show_logo(UINT8_T choice)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	SSD1306_PicDisplay( ssd136_LOGO0_Page_Start,
								 ssd136_LOGO0_Page_Num,
								 ssd136_LOGO0_Coloum,
								 ssd136_LOGO0_Width,
								 &logo[choice],
								 ssd136_LOGO0_Bytes_Num
								 );
}

void show_battery_status(UINT8_T choice)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	SSD1306_FillArea(0x2, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
	SSD1306_PicDisplay( ssd136_Battery_status0_Page_Start,
								 ssd136_Battery_status0_Page_Num,
								 ssd136_Battery_status0_Coloum,
								 ssd136_Battery_status0_Width,
								 &Battery_Status[choice],
								 ssd136_Battery_status0_Bytes_Num
								 );
}

void show_external_power(UINT8_T choice)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	//SSD1306_FillArea(0x2, 0x4, 0x20, 0x40, 0x00, (0x4*0x40));
	SSD1306_PicDisplay( ssd136_External_Power_status0_Page_Start,
								 ssd136_External_Power_status0_Page_Num,
								 ssd136_External_Power_status0_Coloum,
								 ssd136_External_Power_status0_Width,
								 &External_Power_Status[choice],
								 ssd136_External_Power_status0_Bytes_Num
								 );
}

void show_USB_status(UINT8_T choice)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	SSD1306_PicDisplay( ssd136_USB_status0_Page_Start,
								 ssd136_USB_status0_Page_Num,
								 ssd136_USB_status0_Coloum,
								 ssd136_USB_status0_Width,
								 &USB_Status[choice],
								 ssd136_USB_status0_Bytes_Num
								 );
}

void show_Downlaod(UINT8_T choice)
{
	if (ssd136_oled_unstable == 1)
		return;
	
	SSD1306_PicDisplay( ssd136_Download_Page_Start,
								 ssd136_Download_Page_Num,
								 ssd136_Download_Coloum,
								 ssd136_Download_Width,
								 &download_Status[choice],
								 ssd136_Download_Bytes_Num
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

