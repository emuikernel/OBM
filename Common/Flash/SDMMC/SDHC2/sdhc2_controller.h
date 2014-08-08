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
 ******************************************************************************/


#ifndef __SDHC2_CONTROLLER_H
#define __SDHC2_CONTROLLER_H

#include "Typedef.h"

// MMC controller registers definition
typedef struct 
{
  VUINT32_T mm4_sysaddr;                  		// DMA system address				 		0x0
  VUINT32_T mm4_blk_cntl;                       // Block size control register		 		0x4
  VUINT32_T mm4_arg;                      		// Command argument					 		0x8
  VUINT32_T mm4_cmd_xfrmd;                      // Command and transfer mode		 		0xC
  VUINT32_T mm4_resp0;                          // cmd response 0					 		0x10
  VUINT32_T mm4_resp1;                          // cmd response 1					 		0x14
  VUINT32_T mm4_resp2;                          // cmd response 2					 		0x18
  VUINT32_T mm4_resp3;                          // cmd response 3					 		0x1C
  VUINT32_T mm4_dp;                             // buffer data port					 		0x20
  VUINT32_T mm4_state;                          // mm4 state						 		0x24
  VUINT32_T mm4_cntl1;                          // host control 1					 		0x28
  VUINT32_T mm4_cntl2;                          // host control 2					 		0x2C
  VUINT32_T mm4_i_stat;                         // status of current command 		 		0x30
  VUINT32_T mm4_i_stat_en;                      // interrupt status enable			 		0x34
  VUINT32_T mm4_i_sig_en;                       // interrupt signal enable			 		0x38
  VUINT32_T mm4_acmd12_er;                      // auto cmd12 error status			 		0x3C
  VUINT32_T mm4_cap1_2;                         // capabilities 1,2					 		0x40
  VUINT32_T mm4_cap3_4;                         // capabilities 3,4							0x44
  VUINT32_T mm4_sd_max_current1_2;              // max current capabilities					0x48
  VUINT32_T mm4_sd_max_current3_4;              // max current capabilities					0x4C
  VUINT32_T mm4_force_event;              		// force event for AutoCMD12 Error Status	0x50
  VUINT32_T mm4_adma_err_stat;                  // ADMA Error Status						0x54
  VUINT32_T mm4_adma_system_address[2];      	// ADMA Address 63:0						0x58
  VUINT32_T mm4_preset_value_for_init_ds; 		//											0x60
  VUINT32_T mm4_preset_value_for_hs_sdr12;		//											0x64  
  VUINT32_T mm4_preset_value_for_hs_sdr25_50;	//											0x68  
  VUINT32_T mm4_preset_value_for_hs_sdr104_50;	//											0x6C  
  VUINT32_T reserved_X1[28];                    // reserved fields							0x70
  VUINT32_T mm4_shared_bus_control;        		// Shared Bus Control Register				0xE0
  VUINT32_T reserved_X2[6];                   	// reserved									0xE4
  VUINT32_T mm4_sd_slot_int_stat_ctrl_ver;      // SD Interrupt Line and Version Support	0xFC
  VUINT32_T mm4_sd_cfg_fifo_param;				// SD Extra Parameters register				0x100
  VUINT32_T mm4_sd_fifo_param;					// SD_FIFO_PARAM register					0x104
  VUINT32_T mm4_sd_spi_mode_clk_brst_size;		// SD_SIP_MODE register						0x108
  VUINT32_T mm4_sd_ce_ata_1_2;					// SD_CE_ATA_1 and SD_CE_ATA_2				0x10C
  VUINT32_T mm4_sd_pad_io_setup;				// MM4_SD_PAD_IO_SETUP						0x110  	
} MM4_SDMMC_T, *P_MM4_SDMMC_T;

/*************************** Register Masks ********************************/
/**************************************************************************/
// ******************** MM4_BLK_CNTL **********************************
typedef struct
{
 unsigned int xfr_blksz       : 12;       // Transfer Block Size
 unsigned int dma_bufsz       : 4;        // Host DMA buffer size
 unsigned int blk_cnt         : 16;       // Block Count for Current Transfer
} MM4_BLK_CNTL, *P_MM4_BLK_CNTL;

#define MM4_512_HOST_DMA_BDRY  0x7

// ********************* MM4_CMD_XFRMD ********************************
typedef struct
{
 unsigned int dma_en          : 1;        // DMA enable							0
 unsigned int blkcbten        : 1;        // Block Count Enable					1
 unsigned int autocmd12       : 1;        // AutoCMD12      					2
 unsigned int reserved1       : 1;        //									3
 unsigned int dxfrdir         : 1;        // Data Transfer Direction Select 	4
 unsigned int ms_blksel       : 1;        // Multi Block Select					5
 unsigned int reserved2       : 10;        //									6
 unsigned int res_type        : 2;        // Response Type						16
 unsigned int reserved3       : 1;  	  //									18
 unsigned int crcchken        : 1;        // CRC check enable					19
 unsigned int idxchken        : 1;        // Command Index Check Enable			20
 unsigned int dpsel           : 1;        // Data present   select				21
 unsigned int cmd_type        : 2;        // Cmd Type							22
 unsigned int cmd_idx         : 6;        // Cmd Index							24
 unsigned int reserved4       : 2;        //									30
} MM4_CMD_XFRMD, *P_MM4_CMD_XFRMD;

typedef union
{
	MM4_CMD_XFRMD mm4_cmd_xfrmd_bits;
	UINT_T		  mm4_cmd_xfrmd_value;
} MM4_CMD_XFRMD_UNION, *P_MM4_CMD_XFRMD_UNION; 

#define NO_ARGUMENT					0xffffffff
#define MM4_CMD_TYPE_NORMAL 		0
#define MM4_CMD_TYPE_SUSPEND  		1
#define MM4_CMD_TYPE_RESUME 		2
#define MM4_CMD_TYPE_ABORT 			3
#define MM4_CMD_DATA 				1
#define MM4_CMD_NODATA				0
#define MM4_NO_RES					0
#define MM4_136_RES					1
#define MM4_48_RES					2
#define MM4_48_RES_WITH_BUSY		3

// this information will be included in the response type argument of relevant apis.
// it will occupy bits 15:8 of the RespType parameter.
#define MM4_RT_MASK					0x7f00
#define MM4_RT_NONE					0x0000
#define	MM4_RT_R1					0x0100
#define	MM4_RT_R2					0x0200
#define	MM4_RT_R3					0x0300
#define	MM4_RT_R4					0x0400
#define	MM4_RT_R5					0x0500
#define	MM4_RT_R6					0x0600
#define	MM4_RT_R7					0x0700		// sd card interface condition

#define MM4_RT_BUSYMASK				0x8000
#define MM4_RT_BUSY					0x8000


#define MM4_MULTI_BLOCK_TRAN		1
#define MM4_SINGLE_BLOCK_TRAN		0
#define MM4_HOST_TO_CARD_DATA		0
#define MM4_CARD_TO_HOST_DATA		1

// ********************* MM4_STATE ********************************
typedef struct
{
 unsigned int ccmdinhbt		: 1;
 unsigned int dcmdinhbt		: 1;
 unsigned int datactv		: 1;
 unsigned int retuning_req	: 1;
 unsigned int reserved0		: 4;
 unsigned int wractv		: 1;
 unsigned int rdactv		: 1;
 unsigned int bufwren		: 1;
 unsigned int bufrden		: 1;
 unsigned int reserved1		: 4;
 unsigned int cdinstd		: 1;
 unsigned int cdstbl		: 1;
 unsigned int cddetlvl		: 1;
 unsigned int wpswlvl		: 1;
 unsigned int lwrdatlvl		: 4;
 unsigned int cmdlvl		: 1;
 unsigned int uprdatlvl		: 4;
 unsigned int reserved2		: 3;
} MM4_STATE, *P_MM4_STATE;

// ********************* MM4_CNTL1 ********************************
typedef struct
{
 unsigned int ledcntl 		: 1;		// 0
 unsigned int datawidth		: 1;		// 1
 unsigned int hispeed		: 1;		// 2
 unsigned int dma_sel		: 2;		// 3
 unsigned int ex_data_width	: 1;		// 5
 unsigned int card_det_l	: 1;		// 6
 unsigned int card_det_s	: 1;		// 7
 unsigned int buspwr		: 1;		// 8
 unsigned int vltgsel		: 3;		// 9
 unsigned int reserved2		: 4;		// 12
 unsigned int bgreqstp		: 1;		// 16
 unsigned int contreq		: 1;
 unsigned int rdwcntl		: 1;
 unsigned int bgirqen		: 1;
 unsigned int reserved3		: 12;
} MM4_CNTL1, *P_MM4_CNTL1;

typedef union
{
	MM4_CNTL1     mm4_cntl1_bits;
	UINT_T		  mm4_cntl1_value;
} MM4_CNTL1_UNION, *P_MM4_CNTL1_UNION; 

#define MM4_VLTGSEL_1_8		0x5
#define MM4_VLTGSEL_3_0		0x6
#define MM4_VLTGSEL_3_3		0x7

// ********************* MM4_CNTL2 ********************************
typedef struct
{
 unsigned int inter_clk_en		: 1;	// Internal Clock Enable
 unsigned int inter_clk_stable	: 1;	// Internal Clock Stable
 unsigned int mm4clken			: 1;	// Clock Enable
 unsigned int reserved1			: 2;	// bits 3,4
 unsigned int clk_gen_sel		: 1; 	// bit 5
 unsigned int sd_freq_sel_hi	: 2;	// 6								 
 unsigned int sd_freq_sel_lo	: 8;	// 8								  
 unsigned int dtocntr			: 4;	// bit 0 Data Timeout Counter Value
 unsigned int reserved2			: 4;	//
 unsigned int mswrst			: 1;	// bit 8 Software Reset for All
 unsigned int cmdswrst			: 1;	// bit 9 Software Reset for MM4CMD Line
 unsigned int datswrst			: 1;	// bit 10 Software Reset for MM4DATx Lines
 unsigned int reserved3			: 5;	// bits 11-15
} MM4_CNTL2, *P_MM4_CNTL2;

#define MM4CLKEN_EN_CLOCK    1
#define MM4CLOCK200KHZRATE	0x1FF	// Set also additional SD_FREQ_HI bit.	So SD_FREQ_SEL = 0x1FF = 511 * 2 = 1022 (clock divider)
#define MM4CLOCK50MHZRATE	2
#define MM4CLOCK25MHZRATE	4
#define MM4CLOCK12_5MHZRATE	8
#define MM4CLOCK6MHZRATE	16

#define CLOCK_27_MULT		0xE

typedef union
{
	MM4_CNTL2     mm4_cntl2_bits;
	UINT_T		  mm4_cntl2_value;
} MM4_CNTL2_UNION, *P_MM4_CNTL2_UNION; 

// ********************* MM4_I_STAT, MM4_I_STAT_EN, MM4_I_SIGN_EN ************
typedef struct
{
 unsigned int cmdcomp 		: 1;   //0
 unsigned int xfrcomp		: 1;   //1
 unsigned int bgevnt		: 1;   //2
 unsigned int dmaint		: 1;   //3
 unsigned int bufwrrdy		: 1;   //4
 unsigned int bufrdrdy		: 1;   //5
 unsigned int cdins			: 1;   //6
 unsigned int cdrem			: 1;   //7
 unsigned int cdint			: 1;   //8
 unsigned int int_a			: 1;   //9
 unsigned int int_b			: 1;   //10
 unsigned int int_c			: 1;   //11
 unsigned int retuninig_int	: 1;   //12
 unsigned int reserved0		: 2;   //13
 unsigned int errint		: 1;   //15
 unsigned int ctoerr		: 1;   //16
 unsigned int ccrcerr		: 1;   //17
 unsigned int cenderr		: 1;   //18
 unsigned int cidxerr		: 1;   //19
 unsigned int dtoerr		: 1;   //20
 unsigned int dcrcerr		: 1;   //21
 unsigned int denderr		: 1;   //22
 unsigned int ilmterr		: 1;   //23
 unsigned int ac12err		: 1;   //24
 unsigned int admaerr		: 1;   //25
 unsigned int tune_err		: 1;   //26
 unsigned int reserved1		: 1;   //27
 unsigned int spierr		: 1;   //28
 unsigned int axi_resp_err	: 1;   //29
 unsigned int cpl_tout_err  : 1;   //30
 unsigned int crc_stat_err	: 1;   //31
} MM4_I_STAT, *P_MM4_I_STAT, MM4_I_STAT_EN, *P_MM4_I_STAT_EN, MM4_I_SIGN_EN, *P_MM4_I_SIGN_EN;

#define DISABLE_INTS 	0
#define ENABLE_INTS		1

typedef union
{
	MM4_I_STAT 	  mm4_i_stat_bits;
	UINT_T		  mm4_i_stat_value;
} MM4_I_STAT_UNION, *P_MM4_I_STAT_UNION; 
//#define CLEAR_INTS_MASK		0xFFFF7FFD
#define CLEAR_INTS_MASK		0xFFFF7FCD


// ********************* MM4_ACMD12_ER *******************************************
typedef struct
{
 unsigned int ac12nexe			: 1;   // 0
 unsigned int ac12toer			: 1;   // 1
 unsigned int ac12crcer			: 1;   // 2
 unsigned int ac12ender			: 1;   // 3
 unsigned int ac12idxer			: 1;   // 4
 unsigned int reserved0			: 2;   // 5
 unsigned int cmdnisud			: 1;   // 7
 unsigned int reserved1			: 8;   // 8
 unsigned int uhs_mode_sel		: 3;   // 16	
 unsigned int sgh_v18_en		: 1;   // 19
 unsigned int drv_strength_sel 	: 2;   // 20 
 unsigned int exe_tuning		: 1;   // 22
 unsigned int sampling_clk_sel	: 1;   // 23
 unsigned int reserved2			: 6;   // 24
 unsigned int async_int_en		: 1;   // 30
 unsigned int pre_val_en		: 1;   // 31					
} MM4_ACMD12_ER, *P_MM4_ACMD12_ER;

// ********************* MM4_CAP0 *******************************************
typedef struct
{
 unsigned int toclkfreq				: 6;
 unsigned int reserved0				: 1;
 unsigned int toclkunit				: 1;
 unsigned int bsclkfreq				: 8;
 unsigned int max_blk_len			: 2;
 unsigned int ex_data_width_support	: 1; 
 unsigned int reserved1				: 1;
 unsigned int adma2_support			: 1;
 unsigned int adma1_support			: 1;
 unsigned int hi_speed_support		: 1;
 unsigned int sdma_support			: 1;
 unsigned int sus_res_support		: 1;
 unsigned int vlg_33_support		: 1;
 unsigned int vlg_30_support		: 1;
 unsigned int vlg_18_support		: 1;
 unsigned int reserved2				: 1;
 unsigned int sys_bus_64_support	: 1;
 unsigned int async_int_support		: 1;
 unsigned int cfg_slot_type			: 1;
} MM4_CAP1_2, *P_MM4_CAP1_2;

typedef union
{
	MM4_CAP1_2 	  mm4_cap1_2_bits;
	UINT_T		  mm4_cap1_2_value;
} MM4_CAP1_2_UNION, *P_MM4_CAP1_2_UNION; 

// ********************* MM4_SD_MAX_CURRENT1_2 *******************************************
typedef struct
{
 unsigned int v3_3vmaxi		: 8;
 unsigned int v3_0vmaxi		: 8;
 unsigned int v1_8vmaxi		: 8;
 unsigned int reserved0 	: 8;
} MM4_SD_MAX_CURRENT1_2, *P_MM4_SD_MAX_CURRENT1_2;

typedef union
{
	MM4_SD_MAX_CURRENT1_2 	  	mm4_sd_max_current1_2_bits;
	UINT_T		  				mm4_sd_max_current1_2_value;
} MM4_SD_MAX_CURRENT1_2_UNION, *P_MM4_SD_MAX_CURRENT1_2_UNION; 

// ********************* MM4_SD_CFG_FIFO_PARAM *******************************************

typedef struct
{
 unsigned int reserved0				: 3;	// Bits 0-2
 unsigned int boot_ack				: 1; 	// Bit 3
 unsigned int squ_empty_check		: 1;	// Bit 4 
 unsigned int squ_full_check		: 1;	// Bit 5
 unsigned int gen_pad_clk_on		: 1;	// Bit 6
 unsigned int reserved2				: 17;	// Bits 7-23
 unsigned int gen_pad_clk_cnt		: 8;	// Bits	24-31
} MM4_SD_CFG_FIFO_PARAM, *P_MM4_SD_CFG_FIFO_PARAM;

typedef union
{
	MM4_SD_CFG_FIFO_PARAM 	MM4_SD_CFG_FIFO_PARAM_bits;
	UINT_T		  		MM4_SD_CFG_FIFO_PARAM_value;
} MM4_SD_CFG_FIFO_PARAM_UNION, *P_MM4_SD_CFG_FIFO_PARAM_UNION; 


// ********************* MM4_SD_CE_ATA_1_2 *******************************************
typedef struct
{
 unsigned int cpl_timeout			: 14;	// Bits 0-13
 unsigned int reserved0				: 2;	// Bit 14,15
 unsigned int misc_int_int_en	 	: 1;	// Bit 0
 unsigned int misc_int_en	 		: 1;	// Bit 1
 unsigned int misc_int		 		: 1;	// Bit 2
 unsigned int reserved1				: 1;	// Bit 3
 unsigned int cpl_complete_int_en	: 1;	// Bit 4
 unsigned int cpl_complete_en		: 1;	// Bit 5
 unsigned int cpl_complete			: 1;	// Bit 6
 unsigned int reserved2		 		: 5;	// Bit 7
 unsigned int mmc_card				: 1;	// Bit 12
 unsigned int ceata_card			: 1;	// Bit 13
 unsigned int snd_cpl				: 1;	// Bit 14
 unsigned int chk_cpl				: 1;	// Bit 15
} MM4_SD_CE_ATA_1_2, *P_MM4_SD_CE_ATA_1_2;

typedef union
{
	MM4_SD_CE_ATA_1_2 	MM4_SD_CE_ATA_1_2_bits;
	UINT_T		  		MM4_SD_CE_ATA_1_2_value;
} MM4_SD_CE_ATA_1_2_UNION, *PMM4_SD_CE_ATA_1_2_UNION; 


/*********** End MM4 Register Def's **************************************/
// response types
enum {
  MMC_RESPONSE_NONE = 1L<<8,
  MMC_RESPONSE_R1 = 2L<<8,
  MMC_RESPONSE_R1B = 3L<<8,
  MMC_RESPONSE_R2 = 4L<<8,
  MMC_RESPONSE_R3 = 5L<<8,
  MMC_RESPONSE_R4 = 6L<<8,
  MMC_RESPONSE_R5 = 7L<<8,
  MMC_RESPONSE_R5B = 8L<<8,
  MMC_RESPONSE_R6 = 9L<<8,
  MMC_RESPONSE_R7 = 0xAL<<8,
  MMC_RESPONSE_MASK = 0x0000FF00
};

#define SD_OCR_VOLTAGE_3_3_TO_3_6 	0xE00000
#define SD_OCR_VOLTAGE_1_8_TO_3_3 	0x1F8000
#define SD_OCR_VOLTAGE_1_8		 	0x80
#define MMC_OCR_VOLTAGE_ALL			0xFF8000
#define	MM4FIFOWORDSIZE				0x80

// device context for MMC API, containing anything needed to operate on 
// this API. It is always first parameter for all APIs.
typedef struct 
{
  P_MM4_SDMMC_T            	pMMC4Reg;				// MMC1 register base
} MM4_SDMMC_CONTEXT_T, *P_MM4_SDMMC_CONTEXT_T;


//Function Prototypes
void MMC4StopInternalBusClock(P_MM4_SDMMC_CONTEXT_T pContext);
void MMC4StartInternalBusClock(P_MM4_SDMMC_CONTEXT_T pContext);
void MMC4StartBusClock(P_MM4_SDMMC_CONTEXT_T pContext);
void MMC4StopBusClock (P_MM4_SDMMC_CONTEXT_T pContext);
void MMC4SetBusRate(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T rate);
void MMC4EnableDisableIntSources(P_MM4_SDMMC_CONTEXT_T pContext, UINT8_T Desire);
void MMC4SetDataTimeout(P_MM4_SDMMC_CONTEXT_T pContext, UINT8_T CounterValue);
void MMC4CMDSWReset(P_MM4_SDMMC_CONTEXT_T pContext);
void MMC4DataSWReset(P_MM4_SDMMC_CONTEXT_T pContext);
void MMC4FullSWReset(P_MM4_SDMMC_CONTEXT_T pContext);
void MMC4SendDataCommand(P_MM4_SDMMC_CONTEXT_T pContext, 
                  UINT_T Cmd,
                  UINT_T  Argument, 
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType, 
                  UINT_T SDMAMode);

void MMC4SendDataCommandNoAuto12(P_MM4_SDMMC_CONTEXT_T pContext, 
                  UINT_T Cmd,
                  UINT_T  Argument, 
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType, 
                  UINT_T SDMAMode);

void MMC4SendSetupCommand(P_MM4_SDMMC_CONTEXT_T pContext, 
                  UINT_T Cmd,
                  UINT_T CmdType,  
                  UINT_T  Argument, 
                  UINT_T ResType);
#endif