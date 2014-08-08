#ifndef __ZIMI_BQ24295__
#define __ZIMI_BQ24295__


#define VENDER_REG 0x0A
#define BQ_SYS_STATUS 0x08
#define BQ_FAULT_STATUS 0x09

#define	PMUA_BASE	0xD4282800

#define ZIMI_POWER_UP_VOL_USB		3100   //usb case
#define ZIMI_POWER_UP_VOL_NOR		3300   //normal case

#define	PMUA_SD_ROT_WAKE_CLR		(PMUA_BASE+0x007C)	/* 32 bit	SDIO/Rotary
														 *			Wake
														 *			Clear
														 *			Register
														 */
#define PMUA_SD_ROT_WAKE_CLR_USB_VBUS_STS		BIT_15

typedef enum
{
	PB_CHIP_TYPE_UNKNOW,
	PB_CHIP_TYPE_BQ24195,
	PB_CHIP_TYPE_BQ24295
}PB_CHIP_TYPE;

extern PB_CHIP_TYPE gChipType;
extern int isBqDetected;

UINT8_T IIC_read(UINT8_T reg_addr);
void IIC_write(UINT8_T reg_addr,UINT8_T value);
char zimiInitBqChip();


#endif
