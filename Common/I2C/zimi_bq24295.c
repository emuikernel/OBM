#if ZIMI_PB05
#include "typedef.h"
#include "zimi_bq24295.h"
#include "USB1.h"
#include "gpio.h"
#include "ustica.h"
#include "i2c.h"

int isBqDetected = 1;
int isTempDetect;
int isInMiniSys = 0;
int isInNormalSys =0;
UINT16_T gBatVol;
UINT_T gChipID;
UINT_T isUniformSector = 0;

extern UINT8_T BBU_getCI2C(UINT8_T regAddr);
extern UINT_T BBU_putCI2C(UINT8_T regAddr, UINT8_T data);
extern UINT_T BBU_CI2C_Init();

UINT8_T IIC_read(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for BQ24295
	*(VUINT_T*)0xD401701c = 0xD6;

	return BBU_getCI2C(reg_addr);
}

void IIC_write(UINT8_T reg_addr,UINT8_T value)
{
	// Set I2C address for the following i2c writing for Bq24295
	*(VUINT_T*)0xD401701c = 0xD6;

	BBU_putCI2C(reg_addr, value);
}

PB_CHIP_TYPE gChipType = PB_CHIP_TYPE_UNKNOW;
char zimiInitBqChip()
{
	UINT8 chipid;
	int trytime = 0;
	do
	{
		chipid = IIC_read(VENDER_REG);
		trytime++;
		if(chipid == 0xC0)
		{
			gChipType = PB_CHIP_TYPE_BQ24295;
		}
		else if(chipid == 0x23)
		{
			gChipType = PB_CHIP_TYPE_BQ24195;
		}
	}while((gChipType != PB_CHIP_TYPE_UNKNOW) && trytime < 10);
	if(gChipType == PB_CHIP_TYPE_UNKNOW)
	{
		serial_outstr("zimi# bq not detected\n");
		return 1;
	}
	switch(gChipType)
	{
		case PB_CHIP_TYPE_BQ24295:
			serial_outstr("zimi# bq24295 connect\n");
			IIC_write(0x05, 0x80);
			IIC_write(0x07, 0x4B);
			IIC_write(0x00, 0x57);
			IIC_write(0x01, 0x1B);
			IIC_write(0x02, 0x9E);	
			IIC_write(0x03, 0x31);
			IIC_write(0x04, 0xB2);	
			break;
		case PB_CHIP_TYPE_BQ24195:
			serial_outstr("zimi# bq24195 connect\n");
			IIC_write(0x05, 0x80);
			IIC_write(0x07, 0x4B);
			IIC_write(0x00, 0x52);
			IIC_write(0x01, 0x1B);
			IIC_write(0x02, 0xA0);			
			IIC_write(0x03, 0x31);		
			IIC_write(0x04, 0xB2);				
			break;
		default:
			serial_outstr("zimi# bq not detected\n");
			return 1;
	}

	return 0;
}

UINT8_T get_charge_type()
{
	UINT8_T charge_type =0;
	UINT_T value;
	
	PlatformCI2Init();

	Delay_us(2000);

	value = *(VUINT_T *)USB_PORTSC;
	serial_outstr("USB_PORTSC\n");
	serial_outnum(value);
	serial_outstr("\n");
	if ((value & 0xC00) == 0xC00)
	{
		serial_outstr("zimi# Wall Charger\n");
		charge_type = 0;
	}
	else if ((value & 0xC00) == 0x800)
	{
		serial_outstr("zimi# USB Charger\n");
		charge_type = 1;
	}
	else
	{
		serial_outstr("zimi# No Charger\n");
		charge_type = 1;   //we force USB charger
	}

	PlatformCI2Shutdown();	
	return charge_type;
}
void PB05_charging_loop()
{
	UINT16_T vbat_mv;
	UINT8 bqStatus,charge_type;
	int ischargingerror = 0 ;
	int m_loop = 1;
	
	extern int isBqDetected;
	
	//GetBatInstantVolt(&vbat_mv, USB_Connect);
	if(gBatVol>= ZIMI_POWER_UP_VOL_USB)
	{
		serial_outstr("zimi# battery vol is OK\n");
		return;
	}

	extern PowerUPType_t powerup;
	powerup = PowerUP_USB;   //force powerup
	LED_ALL_OFF();

	//set OUT_EN to output
	UINT_T temp;
	reg_write((GPIO1_BASE + GPIO_SDR), BIT18); //set to output
	reg_write((GPIO1_BASE + GPIO_PCR), BIT18); //set logic to 0
	
	if(isBqDetected == 0)
	{
		if ((*(VUINT_T *)PMUA_SD_ROT_WAKE_CLR) & PMUA_SD_ROT_WAKE_CLR_USB_VBUS_STS)  // usb connect && i2c communication fail
		{
			char ret_val;
			serial_outstr("zimi#enter special state\n");
			while(1)
			{
				LED_BAT_R_ON();
				ret_val = zimiInitBqChip();
				if(ret_val == 0)  //i2c success, we break to set large charging current
					break;
				zimi_delay_ms_chg(5000);
				GetBatInstantVolt(&vbat_mv, USB_NotConnect);
				if(vbat_mv >ZIMI_POWER_UP_VOL_USB)  //the bat vol is ok, just return
				{
					*(VUINT_T *)ZIMI_BATVOL_INIT_FLAG_ADDR = vbat_mv;
					//LED_BAT_R_OFF();
					return;
				}
				LED_BAT_R_OFF();
				zimi_delay_ms(500);  //to notify user this special state, we off led per 5s
				serial_outstr("zimi#special state loop\n");
			}
		}
		else
		{
			serial_outstr("zimi# bq not detected!!!\n");
			System_poweroff();
		}
	}
	LED_BAT_R_ON();
	charge_type = get_charge_type();

	//xyl: force adapter type
	//charge_type = 0;  

	//force  turn ON Q4
	UINT8_T mm;
	mm = IIC_read(0x07) & 0xff;
	mm &= ~0x20;
	IIC_write(0x07,mm);
	
	if(charge_type == 1)
	{
		IIC_write(0x00, 0x42);
	}
	else
	{
		IIC_write(0x00, 0x57);
	}

	if(gChipType == PB_CHIP_TYPE_BQ24295)
		IIC_write(0x02, 0x9E);
	else if(gChipType == PB_CHIP_TYPE_BQ24195)
		IIC_write(0x02,0xA0);
	IIC_write(0x03, 0x31);		
	Delay_us(3000*1000);


	
	UINT8 regt;
	regt = IIC_read(BQ_FAULT_STATUS);
	do
	{
		bqStatus = IIC_read(BQ_SYS_STATUS);

		if((bqStatus & 0x4) == 0)
		{
			serial_outstr("zimi# charger out\n");
			System_poweroff();
		}
		
		//caution here: we choose USB_NotConnect beacuse Marvell will minus 100mv default
		GetBatInstantVolt(&vbat_mv, USB_NotConnect);
		if(vbat_mv >ZIMI_POWER_UP_VOL_USB)
		{
			*(VUINT_T *)ZIMI_BATVOL_INIT_FLAG_ADDR = vbat_mv;
			//LED_BAT_R_OFF();
			break;
		}

		regt = IIC_read(BQ_FAULT_STATUS);
		if((regt & 0x3f) == 0)
		{
			ischargingerror = 0;
		}
		else
		{
			ischargingerror = 1;
		}

		if(ischargingerror)
		{
			while(1)
			{
				UINT8_T m_temp;

				m_temp = IIC_read(0x07) & 0xff;
				 m_temp &= ~0x20;
				 IIC_write(0x07,m_temp);
				
				serial_outstr("zimi# charging error\n");
				LED_BAT_R_OFF();
				Delay_us(200*1000);
				LED_BAT_R_ON();
				regt = IIC_read(BQ_FAULT_STATUS);
				if((regt & 0x3f) != 0)
				{
					ischargingerror = 0;
					break;
				}
				Delay_us(200*1000);

				 m_temp = IIC_read(0x07) & 0xff;
				 m_temp &= ~0x20;
				 IIC_write(0x07,m_temp);
				
				regt = IIC_read(BQ_FAULT_STATUS);
				if((regt & 0x3f) != 0)
				{
					LED_BAT_R_ON();
					ischargingerror = 0;
					break;
				}

				bqStatus = IIC_read(BQ_SYS_STATUS);
				if((bqStatus & 0x4) == 0)
				{
					serial_outstr("zimi# charger out\n");
					System_poweroff();
				}
			}
		}

		Delay_us(1000*1000);
		m_loop++;
		if(m_loop/5)
		{
			m_loop = 1;
			serial_outstr("charging loop\n");
		}
		
	}while(1);
	
}




void LED_BAT_R_ON()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO0_BASE + GPIO_PSR), BIT27);
	}
	else
	{
		reg_write((GPIO0_BASE + GPIO_PSR), BIT26);
	}
}
void LED_BAT_R_OFF()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{	
		reg_write((GPIO0_BASE + GPIO_PCR), BIT27);
	}
	else
	{
		reg_write((GPIO0_BASE + GPIO_PCR), BIT26);
	}
}

void LED_BAT_G_ON()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO0_BASE + GPIO_PSR), BIT26);
	}
	else
	{
		reg_write((GPIO0_BASE + GPIO_PSR), BIT27);
	}
}
								
void LED_BAT_G_OFF()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO0_BASE + GPIO_PCR), BIT26);
	}
	else
	{
		reg_write((GPIO0_BASE + GPIO_PCR), BIT27);
	}
}

void LED_LTE_R_ON()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO0_BASE + GPIO_PSR), BIT28);
	}
	else
	{
		reg_write((GPIO1_BASE + GPIO_PSR), BIT4);
	}
}
void LED_LTE_R_OFF()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO0_BASE + GPIO_PCR), BIT28);
	}
	else
	{
		reg_write((GPIO1_BASE + GPIO_PCR), BIT4);
	}
}
void LED_LTE_G_ON()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO1_BASE + GPIO_PSR), BIT4);
	}
	else
	{
		reg_write((GPIO0_BASE + GPIO_PSR), BIT28);
	}
}
void LED_LTE_G_OFF()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO1_BASE + GPIO_PCR), BIT4);
	}
	else
	{
		reg_write((GPIO0_BASE + GPIO_PCR), BIT28);
	}
}

void LED_WIFI_G_ON()	
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO1_BASE + GPIO_PSR),BIT2);
	}
	else
	{
		reg_write((GPIO1_BASE + GPIO_PSR),BIT1);
	}
}
void LED_WIFI_G_OFF() 
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO1_BASE + GPIO_PCR),BIT2);
	}
	else
	{
		reg_write((GPIO1_BASE + GPIO_PCR),BIT1);
	}
}
void LED_WIFI_R_ON()   
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO1_BASE + GPIO_PSR),BIT1);
	}
	else
	{
		reg_write((GPIO1_BASE + GPIO_PSR),BIT2);
	}
}
void LED_WIFI_R_OFF()
{
	if(gChipType == PB_CHIP_TYPE_BQ24295)
	{
		reg_write((GPIO1_BASE + GPIO_PCR),BIT1);
	}
	else
	{
		reg_write((GPIO1_BASE + GPIO_PCR),BIT2);
	}
}

void LED_ALL_ON()
{
	reg_write((GPIO1_BASE + GPIO_PSR),BIT1);
	reg_write((GPIO1_BASE + GPIO_PSR),BIT2);
	reg_write((GPIO1_BASE + GPIO_PSR), BIT4);
	reg_write((GPIO0_BASE + GPIO_PSR), BIT28);
	reg_write((GPIO0_BASE + GPIO_PSR), BIT26);
	reg_write((GPIO0_BASE + GPIO_PSR), BIT27);
}

void LED_ALL_OFF()
{
	reg_write((GPIO1_BASE + GPIO_PCR),BIT1);
	reg_write((GPIO1_BASE + GPIO_PCR),BIT2);
	reg_write((GPIO1_BASE + GPIO_PCR), BIT4);
	reg_write((GPIO0_BASE + GPIO_PCR), BIT28);
	reg_write((GPIO0_BASE + GPIO_PCR), BIT26);
	reg_write((GPIO0_BASE + GPIO_PCR), BIT27);
}

void zimi_delay_ms(UINT_T timeout)
{
	UINT8_T val;
	UINT_T startTime, endTime, timeWaited = 0;
	startTime = GetOSCR0();
	
	do
	{
		endTime = GetOSCR0();

		if (endTime < startTime)
			endTime += (0x0 - startTime);
		
		timeWaited = OSCR0IntervalInMilli(startTime, endTime);
	} while (timeWaited < timeout );
}

void zimi_delay_ms_chg(UINT_T timeout)   //for charge, we detect USB here
{
	UINT8_T val;
	UINT_T startTime, endTime, timeWaited = 0;
	startTime = GetOSCR0();
	
	do
	{
		if ((*(VUINT_T *)PMUA_SD_ROT_WAKE_CLR) & PMUA_SD_ROT_WAKE_CLR_USB_VBUS_STS)  // usb connect && i2c communication fail
		{
		}
		else  //USB out
		{
			serial_outstr("zimi# charger out in sepcial state\n");
			System_poweroff();
		}
		endTime = GetOSCR0();

		if (endTime < startTime)
			endTime += (0x0 - startTime);
		
		timeWaited = OSCR0IntervalInMilli(startTime, endTime);
	} while (timeWaited < timeout );
}
void zimi_blink_led()
{
	UINT_T m_time=0;
	for(m_time=0;m_time<2;m_time++)
	{
		LED_ALL_ON();
		zimi_delay_ms(200);
		LED_ALL_OFF();
		zimi_delay_ms(200);
	}

	LED_ALL_ON();
}
//do NOT clear flag
UINT_T zimiCheckBackupImagesFlag(void)
{
	serial_outstr("0x07D7F040\n");
	serial_outnum(*(VUINT_T *)BACKUP_IMAGES_ADDR);
	serial_outstr("\n");

	return *(VUINT_T *)BACKUP_IMAGES_ADDR;
}

void OUTPUT_BOOST_ENABLE(void)
{
	IIC_write(0x07, 0x4B);
	zimi_delay_ms(5);
	IIC_write(0x01, 0x2B);
}

#define OUTPUT_BOOST_DISABLE()	IIC_write(0x01, 0x1B)

void bqTempDetect(void)
{
	UINT8 bqStatus;
	UINT8 mm;
	UINT16_T mTmp;
		
	if(gChipType == PB_CHIP_TYPE_BQ24195)
	{
		OUTPUT_BOOST_ENABLE();
		zimi_delay_ms(500);
		bqStatus = IIC_read(BQ_SYS_STATUS);
		if((bqStatus & 0xC0) != 0xC0)  //fail to open burst, just return
		{
			OUTPUT_BOOST_DISABLE();
			return;
		}
		ReadBatTemp(&mTmp);
		serial_outstr("zimi# bat tmp:\n");
		serial_outnum(mTmp);
		serial_outstr("\n");

		OUTPUT_BOOST_DISABLE();
			
		if(mTmp < 1492)  //temp too high
		{
				serial_outstr("zimi# battery temp too high\n");
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

	}
}

void zimi_force_minisys(UINT_T fatalCode)
{
	#if ZIMI_PB05
	extern int isInMiniSys;

	#if USE_SERIAL_DEBUG
	serial_outstr("zimi# fatal error code\n");
	serial_outnum(fatalCode);
	serial_outstr("\n");
	#endif
	
	if(isInMiniSys == 0)
	{
		serial_outstr("zimi# force to mini system\n");
		*(VUINT_T *)BACKUP_IMAGES_ADDR = 0x4D494E53;
		do_wdt_reset();
	}
	else
	{
		serial_outstr("zimi# fatal error in mini sys\n");

		UINT_T odd = 1;
		while(odd&1)
		{
			// need debugger to step beyond this loop after inspecting input args...
			odd+=2;     
		}
	}
	#endif

}

#define FAT_FLASH_ADDR 0x00830000
#define FAT_DATA_SIZE	0x00150000

#define FAT_TEST_ADDR_1  0x00830000
#define FAT_TEST_ADDR_2  0x00840000

#define PSM_TEST_ADDR_1 0x00990000
#define PSM_TEST_ADDR_2 0x00990000

#define TEST_64KB	0x10000

#define PSM_FLASH_ADDR 0x00980000
#define PSM_DATA_SIZE    0x00060000

#define BUFFER_ADDR	DDR_DOWNLOAD_AREA_ADDR
#define READ_BUFFER		0x01B00000

int checkResult(UINT_T addr,UINT_T type)
{
	UINT_T ii;

	UINT_T data;

	if(type)
		data = 0xaaaaaaaa;
	else
		data = 0x55555555;

	for(ii = 0; ii < 16384; ii++)
	{
		if(*(VUINT_T *)addr != data)
			return -1;
		addr += 4;
	}

	return 0;
}
static int failTime =0 ;
UINT32 trytime = 0;

void ddr_data_init()
{
	UINT32 size = FAT_DATA_SIZE,ii;

	UINT_T data;

		data = 0xaaaaaaaa;
	
	UINT_T addr = BUFFER_ADDR;
	for(ii=0;ii< TEST_64KB/4;ii++)
	{
		*(VUINT_T *)addr = data;
		addr +=4;
	}

	data = 0x55555555;

	for(ii=0;ii< TEST_64KB/4;ii++)
	{
		*(VUINT_T *)addr = data;
		addr +=4;
	}
}

void exec_flash(UINT_T flash_addr, UINT_T size, UINT_T type)
{

	UINT_T addr;

	if(type)
		addr = BUFFER_ADDR;
	else
		addr = BUFFER_ADDR + TEST_64KB;

		EraseFlash(flash_addr, size, BOOT_FLASH);
		WriteFlash(flash_addr, addr, size, BOOT_FLASH);
		ReadFlash(flash_addr,READ_BUFFER , 64*1024, BOOT_FLASH);
		if(checkResult(READ_BUFFER,type))
		{
			failTime++;
			LED_WIFI_G_ON();
			while(1)
			{
				UINT32 loop=0;
				if(loop == 30)
				{
					loop =0;
					serial_outstr("trytime till fail:\n");
					serial_outnum(trytime);
					serial_outstr("\n");
					serial_outnum(flash_addr);
					serial_outstr("\n");
				}
				loop++;
			}
		}
		
	
}


void flash_test(void)
{
	LED_ALL_OFF();
	LED_BAT_R_ON();
	LED_WIFI_R_ON();
	LED_LTE_G_ON();


	UINT32 ii;
	UINT loop = 0;

	ddr_data_init();
	
	while(1)
	{
		exec_flash(FAT_TEST_ADDR_1,TEST_64KB,0);
		exec_flash(FAT_TEST_ADDR_2,TEST_64KB,0);
		exec_flash(PSM_TEST_ADDR_1, TEST_64KB,0);
		exec_flash(PSM_TEST_ADDR_2, TEST_64KB,0);
		trytime++;
		loop++;
		if(loop == 5)
		{
			loop = 0;
			serial_outstr("trytime:\n");
			serial_outnum(trytime);
			serial_outstr("\n\n");
		}


		exec_flash(FAT_TEST_ADDR_1,TEST_64KB,1);
		exec_flash(FAT_TEST_ADDR_2,TEST_64KB,1);
		exec_flash(PSM_TEST_ADDR_1, TEST_64KB,1);
		exec_flash(PSM_TEST_ADDR_2, TEST_64KB,1);
		trytime++;
		loop++;
		if(loop == 5)
		{
			loop = 0;
			serial_outstr("trytime:\n");
			serial_outnum(trytime);
			serial_outstr("\n\n");
		}

		
	}
}


#endif
