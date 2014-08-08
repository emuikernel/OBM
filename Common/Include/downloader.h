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
 
 ******************************************************************************
**
**  FILENAME:       downloader.h
**
**  PURPOSE:        Intel Parallel Flasher engine header file.
**
**  LAST MODIFIED:
******************************************************************************/

//#include "global_types.h"
//#include "blockdevice.h"

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "Typedef.h"


/* Concentration of all per-build identifiers */
#define LASET_RAM_ADD      0x18080000

#define BLOCK_DEVICE_SECTOR_SIZE                        (4*1024)                // 32 KB
//#define BLOCK_DEVICE_SECTOR_SIZE 0x2000 //(8*1024)                // 32 KB
#define MAX_NUMBER_OF_IMAGE_STRUCTS_IN_DEVICE_HEADER    30
#define MAX_NUMBER_OF_FLASH_DEVICES_IN_MASTER_HEADER    4
#define MASTER_BLOCK_HEADER_SIZE_IN_BYTES               (sizeof(MasterBlockHeader))
#define DEVICE_HEADER_SIZE_IN_BYTES                     (sizeof(DeviceHeader))
#define DEVICE_HEADER_V2_SIZE_IN_BYTES                  (sizeof(DeviceHeader_V2))
#define MAX_NUMBER_OF_FLASH_SECTIONS                    3

/* end of per-build identifiers */

// downloader commands
#define DLCMD_DO_VERIFY_WRITE               1
#define DLCMD_DO_ERASE_BLOCKS               2
#define DLCMD_WRITE_IMAGE                   4
#define DLCMD_DUMP_IMAGE                    8
#define DLCMD_IMAGE_TYPE_FIELD_BIT          4
#define DLCMD_IMAGE_TYPE_FIELD_SIZE_BITS    4
#define DLCMD_IMAGE_SPARE_AREA_SZ_BIT        8          /* bit 8 through 16 */
#define DLCMD_IMAGE_SPARE_AREA_SZ_FIELD_BITS 8
#define DLCMD_IMAGE_DUMP_SPARE_BIT          16
#define DLCMD_IMAGE_ABSOLUTE_FLASH_OFFSET_ENABLE_BIT 17
#define DLCMD_PARTITION_NUM_FIELD_BIT_MSK   (1<<24)
#define DLCMD_PARTITION_NUM_FIELD_BIT_SIZE  (8)
#define DLCMD_PARTITION_NUM_FIELD_BIT       (24)

#define DLCMD_RAW_BINARY_IMAGE_TYPE                 (0)
#define DLCMD_WINDOWS_MOBILE_IMAGE_TYPE             (1)
#define DLCMD_YAFFS_IMAGE_TYPE	                    (2)
#define DLCMD_UBIFS_IMAGE_TYPE                      (3)
#define DLCMD_JFFS2_IMAGE_TYPE                      (4)
#define DLCMD_ABSOLUTE_FLASH_OFFSET_WITH_SPARE_TYPE (5)
#define DLCMD_SPARSE_IMAGE_TYPE						(6)
#define DLCMD_LZMA_IMAGE_TYPE						(7)
#define DLCMD_LZMA2_IMAGE_TYPE						(8)


/* general downloader commands */
#define FBF_GENERAL_CMD_INITIALIZE_FLASH_MSK         (1)
#define FBF_GENERAL_CMD_PRODUCTION_INIT_FLASH_MSK    (2)
#define FBF_GENERAL_CMD_ERASE_WM_USER_PARTITIONS_MSK (4)
#define FBF_GENERAL_CMD_ERASE_ALL_FLASH_MSK          (8)
#define FBF_GENERAL_CMD_PERFORM_TARGET_RESET_MSK     (16)

/* Flash parameters flags */
#define FLASH_PARAMETERS_ENABLE_BIT        1

#define BUF_FLAG_PRIMARY_VALID              1
#define BUF_FLAG_SECONDARY_VALID            2
#define BUF_FLAG_IS_FIRST_SWITCH            4


#define DSP_START_ADDRESS 0x84000000
#define DSP_FLASH_SIZE    0x80000

#define BOOTLOADER_START_ADDRESS 0x80000000
#define BOOTLOADER_END_ADDRESS   0x80010000

#define INTERNAL_FLASH_START_ADDRESS 0x80000000
#define INTERNAL_FLASH_SIZE (64*0x10000)

#define CS0_BASE_ADDRESS 0x0
#define CS0_SIZE    0x04000000

typedef UINT16 CORE_ID;

#define MANITOBA_CORE_ID 0xABAB
#define HERMON_CORE_ID   0xBBAC
#define BULVERDE_CORE_ID 0xABAD
#define TAVOR_CORE_ID    0x1f34


typedef UINT8 FLASH_FAMILY;

#define TYAX_FLASH_FAMILY            0x00
#define SIBLEY_FLASH_FAMILY          0x01
#define SDRAM_FAMILY                 0x02
#define OEM_CAM_BITS_FLASH_FAMILY    0x03
#define RAW_NAND_FLASH_FAMILY        0x04
#define SAMSUNG_ONENAND              0x05
#define MDOC_NAND                    0x06
#define SDMMC                        0x07

/* customer reserved codes 0x80-0xff */
#define G4_NAND_FLASH_FAMILY      0x80

#if defined (INTEL_2CHIP_PLAT_BVD)
#if defined (TAVOR_PLAT)
#define MAX_NUMBER_OF_FLASH_DEVICES 1 //(BLOCK_DEVICE_BLOCK_SIZE - 4)
#define THIS_CORE_ID                TAVOR_CORE_ID
#else
#define MAX_NUMBER_OF_FLASH_DEVICES 2 //(BLOCK_DEVICE_BLOCK_SIZE - 4)
#define THIS_CORE_ID                BULVERDE_CORE_ID
#endif

#else // INTEL_2CHIP_PLAT_BVD

#define MAX_NUMBER_OF_FLASH_DEVICES 3 //(BLOCK_DEVICE_BLOCK_SIZE - 4)
#define THIS_CORE_ID  HERMON_CORE_ID

#endif // INTEL_2CHIP_PLAT_BVD

#define FLASHER_DL_FLASHER_IMAGE_REQ_MSG        0x06
#define FLASHER_FLASHER_IMAGE_INFO_MSG          0x07
#define FLASHER_FLASHER_IMAGE_START_DL_DATA_MSG 0x08
#define FLASHER_FLASHER_ADD_LOG_ENTRY_REQ_MSG   0x09

#define DUMP_COMPLETE           2
#define GET_BLOCK_ZERO_COMPLETE 1

/* This is the flasher Add Log Entry message */
typedef struct _FlasherProtocolAddLogEntryReqMsgStruct
{
    UINT32 opcode;
    char   log_entry[60];
} FlasherProtocolAddLogEntryReqMsgStruct, *PFlasherProtocolAddLogEntryReqMsgStruct;

#define FLASHER_PROTOCOL_PLATFORMID_MSG 0x5B2ABBCC


/* This is the flasher version message sent by the target to the PC */
typedef struct _FlasherProtocolRequestMsgStruct
{
    UINT32 opcode;
    UINT32 reserved0;
    UINT32 reserved1;
    UINT32 reserved2;
    UINT32 reserved3;
} FlasherProtocolRequestMsgStruct, *PFlasherProtocolRequestMsgStruct;

/* This is the flasher version message sent by the target to the PC */
typedef struct _FlasherFlasherImageInfoImageStruct
{
    UINT32 opcode;
    UINT32 OriginalImageSize;
    UINT32 reserved1;
    UINT32 reserved2;
    UINT32 reserved3;
} FlasherFlasherImageInfoImageStruct, *PFlasherFlasherImageInfoImageStruct;

typedef struct
{
    UINT32 tCH;   /*Enable Signal Hold Time*/
    UINT32 tCS;   /*Enable Signal Setup Time*/
    UINT32 tWH;   /*ND_nWE high duration*/
    UINT32 tWP;   /*ND_nWE pulse width*/
    UINT32 tRH;   /*ND_nRE high duration*/
    UINT32 tRP;   /*ND_nRE pulse width*/
    UINT32 tR;    /*ND_nWE high to ND_nRE Low for Read*/
    UINT32 tWHR;  /*ND_nWE High to ND_nRE Low for Read Status*/
    UINT32 tAR;   /*ND_ALE Low to ND_nRE Low Delay*/
}__attribute__((packed)) FlashTimingStruct;

typedef struct
{
    UINT32 TotalNumOfBlocks; /*The total number of blocks in flash*/
    UINT32 FlashBlockSize; /*Block size in Flash*/
    UINT32 FlashOffset; /*Flash offset address*/
    UINT32 FlashBufferSize; /*Buffer size in Flash*/
    UINT32 NumberOfBlocksInBadBlockPool; /*What is the number of Blocks in the Bad Block Pool*/
    UINT32 SpareAreaSize; /* Flash Device spare area size */
    UINT32 ManufacturerBadBlockInfoOffset; /* Where manufacturer bad block information in flash*/
} __attribute__((packed)) FlashSectionStruct;

typedef struct
{
    FlashTimingStruct FlashTiming; /*Timing Parameters*/
    FlashSectionStruct FlashSection[MAX_NUMBER_OF_FLASH_SECTIONS];
} __attribute__((packed)) FlashParametersStruct;

typedef struct
{
    UINT32 flash_partition_size;    /* partition size of the image */
    UINT32 commands;                /* bit switches */
    UINT32 First_Sector;            /* First sector of the image in the block device */
    UINT32 length;                 /* Block length in bytes */
    UINT32 Flash_Start_Address;    /* start address in flash */
    UINT32 Flash_block_size;        /* flash device block size */
    UINT32 ChecksumFormatVersion2;  /* new format version image checksum (left for backwards compatibility) */
} __attribute__((packed)) ImageStruct;

#define MAX_RESEVERD_LEN	4
#define MAX_NUM_SKIP_BLOCKS      32


typedef struct
{
	UINT32 Image_ID;				/* image id*/
	UINT32 Image_In_TIM;			/* indicate this image is in TIM or not*/
    UINT32 Flash_partition;        /* partition number of the image */
    UINT32 Flash_erase_size;      /* erase size of the image */
    UINT32 commands;                /* bit switches */
    UINT32 First_Sector;            /* First sector of the image in the block device */
    UINT32 length;                 /* Block length in bytes */
    UINT32 Flash_Start_Address;    /* start address in flash */
    UINT32 reserved[MAX_RESEVERD_LEN];
    UINT32 ChecksumFormatVersion2;  /* new format version image checksum (left for backwards compatibility) */
} __attribute__((packed)) ImageStruct_11;

typedef ImageStruct *PImageStruct;
typedef ImageStruct_11 *PImageStruct_11;


typedef struct
{
	UINT32 Total_Number_Of_SkipBlocks; // total numbers of skip blocks
	UINT32 Skip_Blocks[MAX_NUM_SKIP_BLOCKS];
} SkipBlocksInfoStruct;


typedef struct
{
	UINT32 EraseAll; // erase all flag for user partition
    UINT32 ResetBBT; // indicate if reset BBT
    UINT32 NandID;	 // nand flash ID
	UINT32 Reserved[MAX_RESEVERD_LEN - 1];
	SkipBlocksInfoStruct  Skip_Blocks_Struct;
}FlashOptStruct;

typedef struct
{
    UINT32 DeviceFlags;
    FlashParametersStruct FlashParameters;        /*  Device Parameters */

    UINT8 nOfImages;        /* number of images */
    UINT8 BufferSizeLSB;        /* init to 0x00 */
    UINT16 AccessWidth;     /* init to 0x0000 */
    CORE_ID Core_ID;
    FLASH_FAMILY FlashFamily;           /*  Device Flags */
    UINT8 BufferSizeMSB;
    ImageStruct imageStruct[MAX_NUMBER_OF_IMAGE_STRUCTS_IN_DEVICE_HEADER]; /* array of image structs */
} __attribute__((packed)) DeviceHeader;

typedef struct
{
    UINT32 DeviceFlags;
    UINT32 DeviceParameters[16];        /*  Device Parameters, reserve 16 U32 here, will be defined depending on different devices */
	FlashOptStruct  FlashOpt;
	UINT32 ProductMode; // product mode
	UINT8 OptValue;	// choice: 0 - Not reset after download, 1 - Reset after download
	UINT8 ChipID;
	UINT8 reserved[2];
	UINT32 Reserved[MAX_RESEVERD_LEN - 2];
    UINT32 nOfImages;        /* number of images */
    ImageStruct_11 imageStruct_11[MAX_NUMBER_OF_IMAGE_STRUCTS_IN_DEVICE_HEADER]; /* array of image structs */
} __attribute__((packed)) DeviceHeader_11;

typedef DeviceHeader *PDeviceHeader;
typedef DeviceHeader_11 *PDeviceHeader_11;

/* define DeviceHeader from FBF make version V2 (and down) to support backward compatibility for older versions */
typedef struct
{
    UINT8 nOfImages;        /* number of images */
    UINT8 BufferSizeLSB;        /* init to 0x00 */
    UINT16 AccessWidth;     /* init to 0x0000 */
    CORE_ID Core_ID;
    FLASH_FAMILY FlashFamily;           /*  Device Flags */
    UINT8 BufferSizeMSB;
    ImageStruct imageStruct[MAX_NUMBER_OF_IMAGE_STRUCTS_IN_DEVICE_HEADER]; /* array of image structs */
} __attribute__((packed)) DeviceHeader_V2;

#define NUM_OF_SUPPORTED_FLASH_DEVS    4

#define UNIQUE_SIZE 24
typedef struct
{
    char Unique[UNIQUE_SIZE];
    UINT16 Flash_Device_Spare_Area_Size[NUM_OF_SUPPORTED_FLASH_DEVS];

    UINT16 Format_Version;                                /* FBF format version */
    UINT16 Size_of_Block;                                 /* un-used */
    UINT32 Bytes_To_Program;                              /* Total number of bytes to program */
    UINT32 Bytes_To_Verify;                               /* Total number of bytes to verify */
    UINT32 Number_of_Bytes_To_Erase;                      /* Total number of bytes to erase */
    UINT32 Main_Commands;

    UINT32 nOfDevices;                                    /* number of devices to burn in parallel */
    UINT32 DLerVersion;                                   /* Version of downloader current 1          */
    UINT32 deviceHeaderOffset[MAX_NUMBER_OF_FLASH_DEVICES_IN_MASTER_HEADER]; /* offset in Sector 0 for each flash device header  */
} __attribute__((packed)) MasterBlockHeader;
typedef MasterBlockHeader *PMasterBlockHeader;


typedef enum
{
    STATE_COMPARE_BLOCK_CHECKSUM = 0,
    STATE_VERIFY_BLOCK_DATA,
    STATE_FLASH_ERASING_BLOCK_AND_SETUP_BLOCK_WRITE,
    STATE_FLASH_PROGRAM,
    STATE_READ_NEXT_FBF_SECTOR,
    STATE_FLASH_PROGRAM_BINARY_COMPLETE,
    STATE_DEVICE_FLASH_PROGRAMMING_COMPLETE,
    STATE_COMPARE_DATA_TO_BE_BURNED_WITH_DATA_ON_FLASH,
    STATE_WAIT_PENDING_AND_ERASE,
    STATE_WAIT_PENDING_AND_SKIP_BLOCK,
    STATE_IS_PROGRAMMING_COMPLETE,
    WAIT_FOR_FLASH_TO_BECOME_READ_ONLY
} FlasherState, *PFlasherState;

typedef enum
{
    BUF_STATE_ISSUED_REQUEST_FOR_FIRST_BLOCK,
    BUF_STATE_ISSUED_REQUEST_FOR_FIRST_AND_SECOND_BLOCK,
    BUF_STATE_NULL

} BufferState, *PBufferState;

/*- Functions declaration */
void FinalizeFlashProgramming(void);

void DLMakeMasterBlock(MasterBlockHeader* pMasterBlock, UINT32 nOfDevices, UINT32 DLerVersion);
void DLmakeDeviceHeader(DeviceHeader* devHeader, UINT32 numOfImages, UINT32 Blocksize);
UINT16 DLgetMasterBlockHeaderChksum(MasterBlockHeader *mBlockHeader);
void DLaddDeviceToMasterHeader(PMasterBlockHeader mbHeader);
void DLaddImageToDeviceHeader(PDeviceHeader pDev, PImageStruct pImageStruct);
void startFlashProgramming(void);

void DLmakeImageStruct( PImageStruct pImageStruct,
                        UINT32 commands,    /* bit switches */
                        UINT32 BlockDevice_Start_Block, /* start block in Block device */
                        UINT32 length,             /* Block length in bytes */
                        UINT32 Flash_Start_Address /* start address in flash */
                        );


typedef struct
{
    UINT16* startAddr;
    UINT32  nOfBlocksToErase;
    UINT32 Flash_block_size;    /* flash device block size */
} BlockRange;



typedef enum
{
   INVALID = 0,
   READ_REQ,
   READ_PENDING,
   READ_COMPLETE,
   FLASH_WRITING
} BlockStatus;

typedef struct
{
    UINT32      sectorNumber;
    UINT16      *ptrBuffer;
    BlockStatus status;
} BlockDeviceHandle;

typedef enum
{
	PLAT_PXA92x_Old=0,
	PLAT_PXA92x ,
	PLAT_MMP2 ,
	PLAT_WUKONG,
	PLAT_NEVO,
	PLAT_MMP3,
	PLAT_NEZHA,
	PLAT_NEVO_OTA,
	PLAT_EMEI,
	PLAT_HELAN,
	PLAT_EDEN,
	PLAT_HELANLTE,
	PLAT_UNKNOWN = 0xff
} Platform_ID;

/* FLASH related functions and vTable*/
#define FLASH_ACCESS_8BIT 8
#define FLASH_ACCESS_16BIT 16
#define FLASH_ACCESS_32BIT 32
#define RAM_ACCESS_WIDTH 16
#define RAM_ACCESS 0xff

typedef struct
{
    void (*VTsetFlashToreadStatusMode)(UINT16*);
    void (*VTsetFlashToReadOnlyMode)(UINT16*);
    int (*VTIsFlashBusy)(UINT16* vAddress);
    int (*VTBlockErase)(UINT16* addressToBlock, UINT32 Flash_block_size, UINT32 commands);
    UINT32 (*VTFlashWriteData)(UINT16* vAddress, UINT16* ptrToBuf ,  UINT32 BufferSize);
    void (*VTFlashWriteWord)(UINT16* vAddress, UINT16 word);
    void (*VTFlashWriteBuffer)(UINT16* vAddress, UINT16* ptrToBuf , UINT32 BufferSize);
    UINT32 (*VTcalculateBlockChecksum)(UINT32* vAddress, UINT32 length);
    UINT32 (*VTcompareData)(UINT32* ptr1, UINT32* ptr2, UINT32 len, UINT32 device_num);
    void (*VTreadFlashMemory)(UINT32* flashPtr, UINT32* bufferPtr, UINT32 lengthToRead);
    void (*VTChangeFlashPartition)(UINT32 new_partition);
    void (*VTFinalizeBurning)(void);
    void (*VTInitializeImage)(UINT32 image_commands, UINT32 flash_start_address);
}Flash_vtable, *PFlash_vtable;



typedef struct
{
    UINT8 buffer0[BLOCK_DEVICE_SECTOR_SIZE];
    UINT8 buffer1[BLOCK_DEVICE_SECTOR_SIZE];
} DoubleBuffer;

#define DUMP_ADDRESS_RANGE_CMD 0xFFFFABCD

typedef struct
{
    UINT32 DumpAddressRangeCmd;
    UINT32 FlashStartAddress;
    UINT32 length;
    UINT32 Core_ID;
} FilePreamble;


UINT32 BuffersFindDeltaFromEndOfBuffer( UINT32 *pBuffer );
void updateStatus(void);
void Initialize_Flash_Status_Update( UINT32 num_of_blocks_erased, UINT32 total_num_of_blocks, UINT32 block_size );

/* Windows Mobile Sector info */
typedef struct _SectorInfoWM
{
  DWORD dwReserved1;
  BYTE bOEMReserved;
  BYTE bBadBlock;
  WORD wReserved2;
} __attribute__((packed)) SectorInfoWM, *PSectorInfoWM;


void DLPrintLogEntryMsg(char *pMsg);

void DLPrintBBT(void *BBT_ptr);

#endif // DOWNLOADER_H

