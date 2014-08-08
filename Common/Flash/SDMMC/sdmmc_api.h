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

#ifndef __SDMMC_API_H__
#define __SDMMC_API_H__

#include "Typedef.h"
#include "Flash.h"
#include "xllp_defs.h"
#include "Partition.h"

// Globals
#define TRUE                		1
#define FALSE               		0
#define OSCR_OFFSET         		0x10
#define BOOL                		int
#define OCR_ARG             		0x80FF8000
#define SD_OCR_ARG          		0x40FF8000
#define OCR_ACCESS_MODE_MASK		0x60000000
#define RESPONSE_LENGTH     		8
#define R2_BYTE_LENGTH				16
#define MMC_BUSY_BIT        		0x80000000
#define SD_NO_CMD1          		0x300
#define LOW_ARG             		0x0000ffff
#define NO_FLAGS            		0
#define BLK_CNT             		1
#define BLK_LEN             		512
#define	SDVHS_2_7_TO_3_6	 		0x1
#define	SDVHS_LOW_VOLT		 		0x2
#define	SDVHSARGSHIFT				8
#define SDVHSCHECKPATTERN			0x33
#define HARD512BLOCKLENGTH			512
#define PARTITIONMASK				0xF
#define MMC_SD_BOOT_PARTITION		1
#define MMC_SD_BOOT_PARTITION2		2
#define MMC_SD_USER_PARTITION		0
#define SCRSD1BITMODE 				0x1
#define SCRSD4BITMODE 				0x5
#define SD_CMD6_4BITMODE 			2
#define R1_LOCKEDCARDMASK			0xFDFFFFFF	
#define R1_NOMASK					0xFFFFFFFF


// MMC Controller selections
typedef enum
{
	MMCNOTENABLED		 =  0,
	MMCSDHC0_1 			 =	1,
	MMCSDHC0_2 			 =	2,
	MMCSDHC1_1 			 =	3,
	MMCSDHC1_2 			 =	4
}CONTROLLER_TYPE;

enum 
{
	XLLP_MMC = 0, 
	XLLP_SD =1, 
	XLLP_eSD = 2,
    XLLP_SDIO =3, 
};

typedef	enum
{
    XLLP_MMC_CMD0               = 0x0,	// go idle state
    XLLP_MMC_CMD1               = 0x1,	// send op command
    XLLP_MMC_CMD2               = 0x2,	// all send cid
    XLLP_MMC_CMD3               = 0x3,	// set relative addr
    XLLP_SD_CMD3                = 0x3,	// set relative addr
    XLLP_SDIO_CMD3                = 0x3,	// set relative addr
    XLLP_MMC_CMD4               = 0x4,	// set dsr
    XLLP_SDIO_CMD5                = 0x5,	// SDIO 
    XLLP_SDIO_CMD7              = 0x7,	// select/deselect card for SDIO
    XLLP_MMC_CMD7               = 0x7,	// select/deselect card
    XLLP_SD_CMD6				= 0x6, 	// SD Switch Function Command
    XLLP_MMC_CMD6				= 0x6, 	// MMC Switch Function Command
	XLLP_SD_ACMD6				= 0x6,	// SD ACMD Command for SET_BUS_WIDTH
    XLLP_SD_CMD8               	= 0x8,	// SD Card Interface Condition
	XLLP_MMC_CMD8				= 0x8,	// MMC request to read EXT CSD
    XLLP_MMC_CMD9             	= 0x9,	// send csd
    XLLP_MMC_CMD10            	= 0xa,	// send cid
    XLLP_MMC_CMD11              = 0xb,	//  read data until stop
    XLLP_MMC_CMD12              = 0xc,	// stop transmission
    XLLP_MMC_CMD13              = 0xd,	// send status
    XLLP_MMC_CMD15              = 0xf,	// go inactive state
    XLLP_MMC_CMD16              = 0x10,	//  set block length
    XLLP_MMC_CMD17              = 0x11,	// read single block
    XLLP_MMC_CMD18              = 0x12,	//  read multiple block
    XLLP_MMC_CMD20              = 0x14,	// write data until stop
	XLLP_MMC_CMD23            	= 0x17,	// Set Block Count
    XLLP_MMC_CMD24            	= 0x18,	// write block
    XLLP_MMC_CMD25              = 0x19,	// write multiple block
    XLLP_MMC_CMD26              = 0x1a,	// program CID
    XLLP_MMC_CMD27            	= 0x1b,	// program CSD
    XLLP_MMC_CMD28            	= 0x1c,	// set write prot
    XLLP_MMC_CMD29            	= 0x1d, // clr write prot
    XLLP_MMC_CMD30            	= 0x1e,	// send write prot
    XLLP_SD_CMD32            	= 0x20,	// tag sector start
    XLLP_SD_CMD33            	= 0x21,	// tag sector end
    XLLP_MMC_CMD34            	= 0x22,	// untag sector
    XLLP_MMC_CMD35            	= 0x23,	// tag erase group start
    XLLP_MMC_CMD36            	= 0x24,	//  tag erase group end
    XLLP_MMC_CMD37            	= 0x25,	// untag erase group
	XLLP_eSD_CMD37				= 0x25, // SD PartitionManagement group Command 
    XLLP_eSD_CMD57				= 0x39,
    XLLP_MMC_CMD38            	= 0x26,	// erase
    XLLP_MMC_CMD39              = 0x27,	//fast IO
    XLLP_MMC_CMD40              = 0x28,	// go irq state
    XLLP_MMC_CMD42            	= 0x2a,	// lock-unlock
    XLLP_eSD_CMD43				= 0x2b, // Select Partition
	XLLP_SDIO_CMD52             = 0x34, // SDIO read/write single register
    XLLP_SDIO_CMD53             = 0x35, // SDIO read/write blocks
    XLLP_SD_CMD55              	= 0x37,	// app cmd
    XLLP_MMC_CMD56            	= 0x38,	//  gen cmd
    XLLP_SPI_CMD58              = 0x3a,	// read ocr
    XLLP_SPI_CMD59              = 0x3b,	// crc on-off
    XLLP_SD_ACMD41	            = 0x29,
    XLLP_SD_ACMD51      	    = 0x33	// Read SD Configuration Register (SCR)
} XLLP_MMC_CMD;

typedef struct
{
	UINT_T		CID_VALUE[4];
	UINT_T		SerialNum;	
} CID_LAYOUT;


// note: 
//   in the mmc spec, the first bits of the csd are numbered 127:126, etc. 
//   the csd_structure field has bits labelled "127:126" in the mmc spec. 
//   => the csd_structure field appears first on the logic analyzer.
//
//   within a byte, the high order bits of a byte correspond to the 
//   higher numbers in the mmc spec. for example, the two high order bits
//   of a byte are the ones that contain the two bit "csd_structure" field.
//
//
// CSD structure, as layed out in our SD/MMC controller's response buffer:
// 
//   the 128 bit response is stored backwards:
//
//   when looking at a logic analyzer trace, the first eight bits (bits 127-120) of the csd response 
//   are stored in the sixteenth byte of the reponse buffer. (ie. respbuf[15] has csd_structure field).
//
//   then the next eight bits of the response are stored in the fifteenth byte of the response buffer.
//   (ie. respbuf[14] has taac field).
//   the next eight bits of the response are stored in the fourteenth byte of the response buffer.
//   (ie. respbuf[13] has nsac field).
//
//   the process continues until the last eight bits of the response 
//   are stored in the first byte of the response buffer.
//   (ie. respbuf[0] has file_format_grp, copy, perm... ecc fields)
// 

typedef struct __attribute__ ((__packed__)) SDMMC_CSD_S
{
//	unsigned	rsv4:1;					//   0:  0	// stripped by our controller
//	unsigned	crc:7;					//   7:  1	// stripped by our conroller. if not, it would have been the first byte in the response buffer.
	unsigned	ecc:2;					//   9:  8  // low order bit of first byte in response buffer, response buffer[0]
	unsigned	file_format:2;			//  11: 10
	unsigned	tmp_write_protect:1;	//  12: 12
	unsigned	perm_write_protect:1;	//  13: 13
	unsigned	copy:1;					//  14: 14
	unsigned	file_format_grp:1;		//  15: 15	// high order bit of first byte in response buffer, response buffer[0]
	unsigned	content_prot_app:1;		//  16: 16
	unsigned	rsv3:4;					//  20: 17
	unsigned	write_bl_partial:1;		//  21: 21
	unsigned	write_bl_len:4;			//  25: 22
	unsigned	r2w_factor:3;			//  28: 26
	unsigned	default_ecc:2;			//  30: 29
	unsigned	wp_grp_enable:1;		//  31: 31
	unsigned	wp_grp_size:5;			//  36: 32
	unsigned	erase_grp_mult:5;		//  41: 37
	unsigned	erase_grp_size:5;		//  46: 42
	unsigned	c_size_mult:3;			//  49: 47
	unsigned	vdd_w_curr_max:3;		//  52: 50
	unsigned	vdd_w_curr_min:3;		//  55: 53
	unsigned	vdd_r_curr_max:3;		//  58: 56
	unsigned	vdd_r_curr_min:3;		//  61: 59
	unsigned	c_size:12;				//  73: 62
	unsigned	rsv2:2;					//  75: 74
	unsigned	dsr_imp:1;				//  76: 76
	unsigned	read_blk_misalign:1;	//  77: 77
	unsigned	write_blk_misalign:1;	//  78: 78
	unsigned	read_bl_partial:1;		//  79: 79
	unsigned	read_bl_len:4;			//  83: 80
	unsigned	ccc:12;					//  95: 84
	unsigned	tran_speed:8;			// 103: 96
	unsigned	nsac:8;					// 111:104	// response buffer[13]
	unsigned	taac:8;					// 119:112	// response buffer[14]
	unsigned	rsv1:2;					// 121:120  // low order bit of 15th byte in the response buffer. 
	unsigned	spec_vers:4;			// 125:122
	unsigned	csd_structure:2;		// 127:126	// this first bit in the response package is stored as the high order bit of 15th byte in the response buffer.
} SDMMC_CSD_T;



typedef struct
{
	UINT_T		CSD_VALUE[4];
} CSD_LAYOUT;

typedef struct
{
	UINT_T		SCR_VALUE[2];
} SCR_LAYOUT;

// SD CMD 6 Related
typedef struct
{
	unsigned int   	FunctionGroup		:24;
	unsigned int	Reserved 			:7;
	unsigned int	Mode 				:1;
} SD_CMD6_LAYOUT;

// EXT_CSD
#define BOOT_BUS_WIDTH_MMC_EXT_CSD_OFFSET	177
#define BUS_WIDTH_8_WITH_SDR                0x2      // 8 bit data bus

#define PARTITION_CONFIG_MMC_EXT_CSD_OFFSET	179
#define BOOT_FROM_PARTITION_1_WITH_BOOTACK  0x49     // boot from partition 1 with boot ack enable

#define BUS_WIDTH_MMC_EXT_CSD_OFFSET		183
#define HS_TIMING_MMC_EXT_CSD_OFFSET		185
#define EXT_CSD_ACCESS_CMD_SET				0
#define EXT_CSD_ACCESS_SET_BITS				1
#define EXT_CSD_ACCESS_CLEAR_BITS			2
#define EXT_CSD_ACCESS_WRITE_BYTE			3
#define PARTITION_ACCESS_BITS				0x7


#define PARTITION_MANAGEMENT_CMD_SET 	0xFFFF2F
#define DEFAULT_CMD_SET					0xFFFFFF
#define CHECK_FUNCTION_MODE 			0
#define SWITCH_FUNCTION_MODE			1
#define PARTITION_MANAGEMENT_FUNCTION 	2
#define MMC_ALTERNATE_BOOT_ARGUMENT 	0xFFFFFFFA
#define MMC_CMD0_PRE_IDLE_ARGUMENT		0xF0F0F0F0


typedef union
{
	UINT_T				SD_CMD6_Bits;
	SD_CMD6_LAYOUT		SD_CMD6_Layout;
} SD_CMD6_OVERLAY;


// MMC CMD 6 Related
typedef struct
{
	unsigned int	CmdSet 				:3;
	unsigned int	Reserved0 			:5;
	unsigned int   	Value				:8;
	unsigned int   	Index				:8;
	unsigned int   	Access				:2;
	unsigned int	Reserved1 			:5;
} MMC_CMD6_LAYOUT;

typedef union
{
	UINT_T				MMC_CMD6_Bits;
	MMC_CMD6_LAYOUT		MMC_CMD6_Layout;
} MMC_CMD6_OVERLAY;



typedef enum
{
	UNINITIALIZED,			// Controller and Card are uninitialized
	INITIALIZE,				// Controller and Card are being ninitialized
	READ,					// Multiple Block Read State
	ERASE,					// Erase State
	WRITE,					// Multiple Block Write State
	READY,					// The Card is ready for Data Transfer 
	DATATRAN,				// The controller has finished data transfer but card may be busy
	FAULT					// Fault
} SDMMC_IO_TRANSFERS;

// Vital information used in Data Transfers by the ISR and driver routines.
typedef struct
{
	UINT_T					CardAddress;		// Starting Card Address
	UINT_T					TransWordSize; 		// Total Number of Bytes involved in this transaction
	UINT_T					NumBlocks; 			// Total Number of Blocks involved in this transaction
	UINT_T					LocalAddr;			// Destination Address Pointer for reads, and source addr for writes
	UINT_T 					StartDiscardWords;	// Words - from the first block that caller doesn't want
	UINT_T					EndDiscardWords;	// Words - from the last block that caller doesn't want
	UINT_T					WordIndex;			// Words - Word index to the progress in this transfer request
	UINT8_T					Cmd;				// The command that started the transaction
	UINT8_T					ACmdFlag;			// Indicates if the executing command is standard vs. application specific (ACMD)
	UINT8_T					AutoCMD12Mode;		// When "true", the controller is operating in auto command 12 mode.
	UINT8_T					RespType;			// Info about the response type and potential for busy state.
} SDMMC_TRANSFER;

typedef struct
{
	UINT_T					RCA;			// RCA of the card	
  	UINT_T					OCR;			// OCR Register Contents
	CID_LAYOUT				CID;			// CID Register
  	CSD_LAYOUT			  	CSD;			// CSD Register Contents
  	SCR_LAYOUT				SCR;			// SCR Register Contents	
}	SDMMC_CARD_REGISTERS; 
#define SD_SPEC_MASK	0x0F000000
#define SD_SPEC_OFFSET	24

// Legacy controller related driver functions
typedef struct
{
	UINT_T (*Init_F) ();					// Internal Initialization function
	UINT_T (*SwitchPartition_F) ();			// Internal Switch Partition function
	UINT_T (*Read_F) ();					// Internal Read function
	UINT_T (*Write_F) ();					// Internal Write Function
	UINT_T (*Erase_F) ();					// Internal Erase function
	UINT_T (*Shut_F) ();					// Internal Shutdown function
} SDMMC_FUNCTIONS;

typedef struct 
{
  UINT_T pBuffer[4];							// Buffer to read values in to
  UINT_T CommandComplete;			
// TBD  UINT_T CommandError;	
  UINT_T SendStopCommand;					// If some read/write transactions require stop command	
  UINT_T R1_RESP;							// Capture the R1 Response of the Card for most commands
} SDMMC_RESPONSE;

typedef enum
{
	BYTE_ACCESS = 0, 
	SECTOR_ACCESS = 1
}ACCESS_MODE;

// This Properties structure is shared between MM4 and legacy type drivers. The pContext field
// is specific to the variant. 
typedef struct 
{
	CONTROLLER_TYPE			ControllerType;		// See CONTROLLER_TYPE platformconfig.h
	void				 	*pContext; 			// Pointer to MMC control registers
	UINT8_T					SD; 				// Indicates if the card is SD, eSD or MMC
  	UINT8_T					Slot; 				// Indicates which slot used
	SDMMC_CARD_REGISTERS	CardReg;			// Card Registers 
	SDMMC_RESPONSE			CardReponse;		// Card Response Related 
	UINT_T 					SD_VHS; 			// SD Voltage Acceptance Return Value (SD only)
	volatile SDMMC_IO_TRANSFERS	State;			// Indicate State of the card
	ACCESS_MODE				AccessMode;			// High Density Card
	SDMMC_TRANSFER			Trans;				// Transfer State of the Card
	UINT_T					ReadBlockSize;		// Bytes - Block Size Used for Reads
	UINT_T					WriteBlockSize;		// Bytes - Block Size Used for Writes
	UINT_T					EraseSize;			// Bytes - Minimum Size of an erasable unit
	UINT_T					CardCapacity; 		// Bytes - Maximum Capacity of the card	
	P_FlashProperties_T 	pFlashP; 			// A pointer to generic FlashProperties structure
	SDMMC_FUNCTIONS			Funcs;				// Pointer to some internal driver functions
	UINT_T					SDMA_Mode; 			// Enable SDMA Mode or Not.
	UINT_T					StrictErrorCheck;	// Relax error checking during card init. Not all cards are strictly compliant.
} 
SDMMC_Properties_T, *P_SDMMC_Properties_T;


/*********************************************************************
*	SDMMC R1 Response Bits
**********************************************************************/
typedef enum
{
	R1_SWITCH_ERROR	 			= XLLP_BIT_0,
	R1_ERASE_RESET_ERROR	 	= XLLP_BIT_13,
	R1_CIDCSD_OVERWRITE_ERROR	= XLLP_BIT_16,
	R1_OVERRUN_ERROR 			= XLLP_BIT_17,
	R1_UNDERUN_ERROR 			= XLLP_BIT_18,
	R1_GENERAL_ERROR 			= XLLP_BIT_19,
	R1_CC_ERROR 				= XLLP_BIT_20,
	R1_ECC_ERROR 				= XLLP_BIT_21,
	R1_ILL_CMD_ERROR 			= XLLP_BIT_22,
	R1_COM_CRC_ERROR			= XLLP_BIT_23,
	R1_LOCK_ULOCK_ERRROR		= XLLP_BIT_24,
	R1_LOCK_ERROR 				= XLLP_BIT_25,
	R1_WP_ERROR 				= XLLP_BIT_26,
	R1_ERASE_PARAM_ERROR		= XLLP_BIT_27,
	R1_ERASE_SEQ_ERROR			= XLLP_BIT_28,
	R1_BLK_LEN_ERROR 			= XLLP_BIT_29,
	R1_ADDR_MISALIGN_ERROR 		= XLLP_BIT_30,
	R1_ADDR_RANGE_ERROR 		= XLLP_BIT_31
} R1_SDMMC_RESPONSE_CODES;

typedef struct
{
	unsigned int Reserved0	 		: 1;
	unsigned int BootAck			: 1;
	unsigned int BootPartitionEn	: 3;
	unsigned int PartitionAccess	: 3;
} PARTITION_CONFIG_EXT_CSD;

/*********************************************************************
*	SDMMC Interface API's
**********************************************************************/
UINT_T InitializeSDMMCDevice(UINT8_T FlashNum, UINT8_T* P_DefaultPartitionNum);
UINT_T SDMMC_SHUTDOWN();
UINT_T SDMMC_READ (UINT_T FlashOffset, UINT_T LocalBuffer, UINT_T Size);
UINT_T SDMMC_WRITE (UINT_T FlashOffset, UINT_T LocalBuffer, UINT_T Size);
UINT_T SDMMC_ERASE (UINT_T FlashOffset, UINT_T Size);
void SDMMC_SETPARTITION (UINT_T PartitionNum);

P_SDMMC_Properties_T GetSDMMCProperties(void);
UINT_T MM4SwitchPartitionForAlternateBootMode(void);                 // expose for eMMC alternate boot configuration
UINT_T MM4_MMCReadEXTCSD (UINT_T *pBuffer);                          // expose for eMMC alternate boot configuration

void SDMMC_ISR();
UINT_T SDMMCGetCardErrorState(P_SDMMC_Properties_T pSDMMCP);

// SDMMC partition validation function prototype. 
UINT_T SDMMC_ValidateFlashPartitions (P_PartitionTable_T PartitionTable);
// SDMMC partition creation function prototype. 
UINT_T SDMMC_CreateFlashPartitions (P_PartitionTable_T PartitionTable);

#endif // __SDMMC_API_H__
