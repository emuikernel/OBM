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
**  FILENAME:	TIM.h
**
**  PURPOSE: 	Defines the Trusted/Non-Trusted Image Module
**
**
******************************************************************************/

#ifndef __TIM_H__
#define __TIM_H__

#include "general.h"
#include "security.h"


/******************* TIM Layout ************************************/
/* Below Presents the Maxiumum Size TIM layout in memory

VERSION_I      	VersionBind;         			//
FLASH_I        	FlashInfo;           			//
UINT_T         	NumImages;           			//
UINT_T         	NumKeys;						//
UINT_T         	SizeOfReserved;					//
IMAGE_INFO 		IMG[MAX_IMAGES];                //
KEY_MOD        	Key[MAX_KEYS];                  //
UINT_T         	Reserved[0x4E8];       			//
PLAT_DS        	TBTIM_DS;                       //
                                                //
Below Presents the Maxiumum Size NTIM layout in memory

VERSION_I   	VersionBind;         			//
FLASH_I     	FlashInfo;           			//
UINT_T      	NumImages;           			//
UINT_T         	NumKeys;						//
UINT_T      	SizeOfReserved;					//
IMAGE_INFO 		IMG[MAX_IMAGES];                //
UINT_T      	Reserved[0xD80];       			//
NTIM, *pNTIM;									//
/******************* TIM Layout ************************************/

// TIM Versions
#define TIM_3_2_00			0x30200			// Support for Partitioning
#define TIM_3_3_00			0x30300			// Support for ECDSA-256
#define TIM_3_4_00			0x30400			// Support for ECDSA-521

// B1 TIM
#include "Typedef.h"
#define MAX_IMAGES				20
#define MAX_KEYS	 			3
#define MAX_TIM_SIZE 			0x1000
#define TIMBUFFER 				4096    		//4k for a Tim structure size

// Predefined Image Identifiers
#define TIMIDENTIFIER		0x54494D48		// "TIMH"
#define TIMDUALBOOTID		0x54494D44		// "TIMD"
#define WTMIDENTIFIER		0x57544D49		// "WTMI"
#define OBMIDENTIFIER		0x4F424D49		// "OBMI"
#define OBM2IDENTIFIER		0x4F424D32		// "OBM2"
#define MONITORIDENTIFIER	0x4D4F4E49		// "MONI"
#define TZSWIDENTIFIER		0x545A5349		// "TZSI"    This is also used as consumer ID
#define TBRIDENTIFIER		0x54425249		// "TBRI"	 This is also used as consumer ID
#define DKBIDENTIFIER		0x444B4249		// "DKBI"
#define JTAGIDENTIFIER		0x4A544147		// "JTAG"
#define PATCHIDENTIFIER		0x50415443		// "PATC"
#define TCAIDENTIFIER		0x5443414B		// "TCAK"

#define PARTIONIDENTIFIER	0x50415254      // "PART"
#define FBFIDENTIFIER       0x46424649      // "FBFI" Should never appear in actual TIM file
#define HSIBOOTID			0x48533939		// "HSII"
#define FBFIDENTIFIER2      0x46424644      // "FBFD"
#define FBFIDENTIFIER0      0x46424600      // "FBFx"

#define PRODUCTION_MODE		0x50524d44 		// "PRMD"
#define SBOOT_ID			0x53424c49 		// "SBLI"

// Image Identifiers for LWG
#define OSLOADERID			0x4F534C4F		// "OSLO", re-use for LWG CP
#define RELIABLEID			0x52424C49		// "RBLI", re-use for LWG reliable data
#define LWGDSPID		    0x47524249      // "GRBI", for LWG DSP
#define LWGSKYLARKID 		0x5246424E      // "RFBN", for LWG skylark
#define LWGRESERVED1ID		0x4C574731		// "LWG1", reserved1 for LWG
#define LWGRESERVED2ID		0x4C574732		// "LWG2", reserved2 for LWG
#define LWGRESERVED3ID		0x4C574733		// "LWG3", reserved3 for LWG

// Image Identifiers for LTG
#define LTGOSLOADERID		0x4F534C52		// "OSLR", for LTG CP
#define LTGRELIABLEID		0x52424C52		// "RBLR", for LTG reliable data
#define LTGDSPID		    0x47524252      // "GRBR", for LTG DSP
#define LTGSKYLARKID 		0x52464252      // "RFBR", for LTG skylark
#define LTGRESERVED1ID		0x4C544731		// "LTG1", reserved1 for LTG
#define LTGRESERVED2ID		0x4C544732		// "LTG2", reserved2 for LTG
#define LTGRESERVED3ID		0x4C544733		// "LTG3", reserved3 for LTG

#define PLATFORM_3MODE_LTG_VER		0x334c5447	// LTG only
#define PLATFORM_3MODE_LWG_VER		0x334c5747	// LWG only
#define PLATFORM_5MODE_LTG_VER		0x354c5447	// LTG default
#define PLATFORM_5MODE_LWG_VER		0x354c5747	// LWG default


//Define ID types
//  Use upper 3 bytes as a type identifier and allow up to 256 of a particular type
//
#define TYPEMASK			0xFFFFFF00
#define DDRTYPE				0x444452	 	// "DDR"
#define TIMTYPE				0x54494D		// "TIM"
#define TZRTYPE				0x545A52		// "TZR"

// WTP Format Recognized Reserved Area Indicator
#define WTPRESERVEDAREAID	0x4F505448      // "OPTH"

// Reserved Area Package Headers
#define TERMINATORID		0x5465726D		// "Term"
#define GPIOID				0x4750494F		// "GPIO"
#define UARTID				0x55415254		// "UART"
#define USBID				0x00555342		// "USB"
#define FORCEUSBID			0x46555342		// "FUSB"
#define RESUMEID			0x5265736D		// "Resm"
#define USBVENDORREQ		0x56524551		// "VREQ"
#define TBR_XFER			0x54425258		// "TBRX"
#define RESUMEBLID			0x52736D32		// "Rsm2"
#define ESCAPESEQID			0x45534353		// "ESCS"
#define OEMCUSTOMID			0x43555354		// "CUST"
#define NOMONITORID			0x4E4F4D43		// "NOMC"
#define COREID				0x434F5245		// "CORE"
#define SOCPROFILEID		0x534F4350		// "SOCP"
#define BBMTYPEID			0x42424D54		// "BBMT"
#define TR069INFOID			0x54524655      // "TRFU"
#define LTGLWGSELECTID		0x4C545753      // "LTWS"

// DDR Related ID's
#define DDRID				0x44447248		// "DDRH"
#define DDRGID				0x44445247		// "DDRG"
#define DDRTID				0x44445254		// "DDRT"
#define DDRCID				0x44445243		// "DDRC"
#define CMCCID				0x434d4343		// "CMCC"

// new DDR configuration related ID's
#define CIDPID              0x43494450      // "CIDP"
#define TZRI				0x545A5249		// "TZRI" Trustzone
#define MAX_SCRATCH_MEMORY_ID	0x3

// Operating mode related ID's
#define FREQID				0x46524551		// "FREQ"
#define VOLTID				0x564f4c54		// "VOLT"
#define OPMODEID			0x4d4f4445		// "MODE"
#define OPDIVID 			0x4f504456		// "OPDV"
#define CLKEID				0x434c4b45		// "CLKE"

//Trustzone
#define TZID                0x545A4944      // "TZID" Field based - Legacy
#define TZON                0x545A4f4E      // "TZON" Register based - Legacy

// USB
#define DESCRIPTOR_RES_ID	0x55534200      // "USB"
#define NUM_USB_DESCRIPTORS			12
#define MAX_USB_STRINGS 			7

typedef enum
{
 USB_DEVICE_DESCRIPTOR   				= (DESCRIPTOR_RES_ID | 0),
 USB_QUALIFIER_DESCRIPTOR   		= (DESCRIPTOR_RES_ID | 1),	// added for Qualifier Descriptor - xxliu
 USB_CONFIG_DESCRIPTOR   				= (DESCRIPTOR_RES_ID | 2),
 USB_OTHERSPEED_DESCRIPTOR   		= (DESCRIPTOR_RES_ID | 3),	// added for Other Speed Descriptor - xxliu
 USB_INTERFACE_DESCRIPTOR 				= (DESCRIPTOR_RES_ID | 4),
 USB_LANGUAGE_STRING_DESCRIPTOR   		= (DESCRIPTOR_RES_ID | 5),
 USB_MANUFACTURER_STRING_DESCRIPTOR   	= (DESCRIPTOR_RES_ID | 6),
 USB_PRODUCT_STRING_DESCRIPTOR   	  	= (DESCRIPTOR_RES_ID | 7),
 USB_SERIAL_STRING_DESCRIPTOR   	  	= (DESCRIPTOR_RES_ID | 8),
 USB_INTERFACE_STRING_DESCRIPTOR   		= (DESCRIPTOR_RES_ID | 9),
 USB_DEFAULT_STRING_DESCRIPTOR			= (DESCRIPTOR_RES_ID | 10),
 USB_ENDPOINT_DESCRIPTOR 				= (DESCRIPTOR_RES_ID | 11)
} USB_DESCRIPTORS;

typedef enum
{
      COREID_MP1 = 0,
      COREID_MP2 = 1,
      COREID_MM  = 2,
	  COREID_NONE = 0x4E4F4E45				// "NONE"
}
COREID_T;


// Global Identifiers
#define FFIDENTIFIER		0x00004646		// "FF"
#define ALTIDENTIFIER		0x00414C54		// "ALT"
#define DIFFIDENTIFIER		0x44696666		// "Diff"
#define SEIDENTIFIER		0x00005345		// "SE"
#define U2DIDENTIFIER		0x55534232		// "USB2"
#define PINSIDENTIFIER		0x50696E73		// "Pins"
#define MDOCFORMATID        0x6d444f43      // "mDOC"
#define MDOCBINID           0x6d42944E      // "mBIN"
#define MDOCBDTLID          0x6d424454	    // "mBDT"
#define CI2IDENTIFIER       0x00434932     // "CI2"
#define OTGIDENTIFIER		0x554F5447		// "UOTG"
#define INVALIDID           0x21212121      // "!!!!" Should never appear in actual TIM file

// Miscellanous
#define BINDKEYSIZE			129				// (MAXKEYSIZEWORDS * 2) + 1 for crypto scheme algo
#define FLASHSIGMASK		0x0000000F		// mask off the 4 bits of platform state

/********** WTP Recognized Reserved Area Layout ********************************
*
*	WTPTP_Defined_Reserved_Format_ID    \	  This clues BR, OBM and DKB that the reserved area is in a known format
*	Number of Reserved Area Packages    /	  For each package there is a header, payload size and payload
*
*	Header	 	 		\	  Indicates what type of a Reserved Area Package
*	Size		  		 \	  Size Comprises a single Reserved Area Package
*	Payload		  		 /	  There may be any number of Packages so long as TIM/NTIM < 4KB
*			 			/
*
*	Header		 		\	  The Last one should be a Package with a Terminator Header
*	Size		 		/	  The size should be 8 bytes (the size of this package)
*
**********************************************************************************/
typedef struct
{
 UINT_T WTPTP_Reserved_Area_ID;	  	// This clues BR, OBM and DKB that the reserved area is in a known format
 UINT_T	NumReservedPackages;	  	// For each package there is a header, payload size and payload
}WTP_RESERVED_AREA, *pWTP_RESERVED_AREA;

typedef struct
{
 UINT_T Identifier;					// Identification of this reserved area entry
 UINT_T Size;						// Size  = Payload Size + 2 words (8 bytes).
}WTP_RESERVED_AREA_HEADER, *pWTP_RESERVED_AREA_HEADER;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T 	bmRequestType;
 UINT_T 	bRequest;
 UINT_T		wValue;
 UINT_T		wIndex;
 UINT_T		wLength;
 UINT_T		wData; 							// First word of the proceeding Data. Note, there may be more traialing data
} USB_VENDOR_REQ, *pUSB_VENDOR_REQ;			// There is no restriction that data need be 32 bit aligned.

typedef struct
{
 volatile int *Addr;
 UINT_T Value;
}GPIO_DEF, *pGPIO_DEF;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T XferLoc;						//location to put the XFER table
 UINT_T NumPairs;
 XFER_DATA_PAIR_T pXferPairs[1];
}OPT_XFER_SET, *pOPT_XFER_SET;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T Version;					// 0 = not supported, 1 = MBBT v1, 2 = MBBT v2
}BBM_VERSION_REQ, *pBBM_VERSION_REQ;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T Port;
 UINT_T Enabled; 					// 1 - Enabled
}OPT_PROTOCOL_SET, *pOPT_PROTCOL_SET;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T EscSeqTimeOutMS;  				// Value is interpreted in milli secs.
}OPT_ESCAPE_SEQUENCE, *P_OPT_ESCAPE_SEQUENCE;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T NumGpios;
 pGPIO_DEF	GPIO;
}OPT_GPIO_SET, *pOPT_GPIO_SET;

typedef struct
{
 UINT_T ResumeAddr;
 UINT_T ResumeParam;
 UINT_T ResumeFlag;
}OPT_RESUME_DDR_INFO, *pOPT_RESUME_DDR_INFO;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 OPT_RESUME_DDR_INFO ResumeDDRInfo;
}OPT_RESUME_SET, *pOPT_RESUME_SET;

// Do not remove - used by TBR and BootLoader
#define RESUME_FLAG_MASK 0x55AA55AA

typedef struct
{
 UINT_T DDRResumeRecordAddr;
 void	*DDRScratchAreaAddr;
 UINT_T DDRScratchAreaLength;
}OPT_TIM_RESUME_DDR_INFO, *pOPT_TIM_RESUME_DDR_INFO;

typedef struct
{
 WTP_RESERVED_AREA_HEADER	WRAH;
 OPT_TIM_RESUME_DDR_INFO	TimResumeDDRInfo;
}OPT_TIM_RESUME_SET, *pOPT_TIM_RESUME_SET;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T Enable;			// enable TR069 firmware upgrade or not
 UINT_T FlashAddr;		// flash address to store FBF header
 UINT_T Magic;			// Magic number of FBF header
}TR069_INFO, *pTR069_INFO;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T Choice;			// LTG/LWG select
}LTG_LWG_Select, *pLTG_LWG_Select;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T	ImageAddr;
 UINT_T	ImageSize;
 UINT_T	ImageCRC;
}OPT_RESM_LOC, *pOPT_RESM_LOC;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T CoreId;		// See COREID_T
 UINT_T AddressMapping;
}OPT_COREID, *pOPT_COREID;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T FuseIndex;		// Currently not used
 UINT_T StoreAddress;
}OPT_SOCPROFILEID, *pOPT_SOCPROFILEID;

// New DDR Configuration related structures
typedef struct
{
	unsigned int	ConsumerID;
	unsigned int    NumPackagesToConsume;
	unsigned int	PackageIdentifierList;
} CIDP_ENTRY, *pCIDP_ENTRY;

typedef struct
{
	WTP_RESERVED_AREA_HEADER WRAH;
	unsigned int	NumConsumers;
	CIDP_ENTRY		Consumers;
} CIDP_PACKAGE, *pCIDP_PACKAGE;

typedef struct
{
	unsigned int 	OpId;
	unsigned int	OpValue;
} DDR_OPERATION, *pDDR_OPERATION;

 typedef enum DDR_OPERATION_SPEC_E
{
	DDR_NOP = 0,              // Enums specify the following:
    DDR_INIT_ENABLE,          // DDR init flag (1 = init)
    DDR_MEMTEST_ENABLE,       // DDR Memtest flag (1 = test)
    DDR_MEMTEST_START_ADDR,   // Memtest start addr (Current default is 0)
    DDR_MEMTEST_SIZE,         // NumBytes to test (Current default is 2K)
    DDR_INIT_LOOP_COUNT,      // Loop Count for initialization attempts
    DDR_IGNORE_INST_TO,		  // Treat a time out waiting for a bit to set/clear
							  //   	as informational only, operation continues
    // New values go above here
	DDR_OPERATION_SPEC_E_MAX
} DDR_OPERATION_SPEC_T;

//
// Generic structures for handling instructions
//	Number of parameters are inherent in the command type according to:
//	Function					Descriptor			InstructionID	P1		P2				P3
//								   Text
//	 Write Register				WRITE				0x1				Addr	 Value			n/a
//   Read Register				READ				0x2				Addr	Num of reads	n/a
//   Delay Specified Time		DELAY				0x3				Value	n/a 			n/a
//   WaitForOperationToSetBit	WAIT_FOR_BIT_SET	0x4				Addr	Mask			TimeOut Value
//   WaitForOperationToClearBit	WAIT_FOR_BIT_CLEAR	0x5				Addr	Mask			TimeOut Value
//   AND value with Register	AND_VAL				0x6				Addr	Value			n/a
//   OR value with Register		OR_VAL				0x7				Addr 	Value			n/a
//

typedef enum INSTRUCTION_OP_CODE_SPEC_E
{
    // Enums specify the following:
	INSTR_NOP = 0,
	// Register instructions
    INSTR_WRITE = 1,
    INSTR_READ  = 2 ,
    INSTR_DELAY = 3,
    INSTR_WAIT_FOR_BIT_SET = 4,
    INSTR_WAIT_FOR_BIT_CLEAR = 5,
    INSTR_AND_VAL = 6,
    INSTR_OR_VAL = 7,
	INSTR_SET_BITFIELD = 8,
	INSTR_WAIT_FOR_BIT_PATTERN = 9,
	INSTR_TEST_IF_ZERO_AND_SET = 10,
	INSTR_TEST_IF_NOT_ZERO_AND_SET = 11,
	// Scratch Memory (SM) instructions
	INSTR_LOAD_SM_ADDR = 12,
	INSTR_LOAD_SM_VAL = 13,
	INSTR_STORE_SM_ADDR = 14,
	INSTR_MOV_SM_SM = 15,
	INSTR_RSHIFT_SM_VAL = 16,
	INSTR_LSHIFT_SM_VAL = 17,
	INSTR_AND_SM_VAL = 18,
	INSTR_OR_SM_VAL = 19,
	INSTR_OR_SM_SM = 20,
	INSTR_AND_SM_SM = 21,
	INSTR_TEST_SM_IF_ZERO_AND_SET = 22,
	INSTR_TEST_SM_IF_NOT_ZERO_AND_SET = 23,
 	 // New values go above here
    INSTRUCTION_OP_CODE_E_MAX
} INSTRUCTION_OP_CODE_SPEC_T;

 typedef struct
{
	unsigned int	InstructionId;  // InstructionID determines # of params for defined instructions
	unsigned int   Parameters;		// Parameters[ MAX_NUMBER_PARAMS = 5 ]
} INSTRUCTION_S, *pINSTRUCTION_S;

typedef struct
{
	WTP_RESERVED_AREA_HEADER WRAH;
    unsigned int		NumberOperations;
	unsigned int 		NumberInstructions;
	DDR_OPERATION		DDR_Operations;            // DDR_Op[NumberOperations];
	INSTRUCTION_S	 	DDR_Instructions;	// DDR_Instruction[NumberInstructions];
} DDR_PACKAGE, *pDDR_PACKAGE;
// End New DDR Configuration Related structures

// New Trustzone related structures
typedef struct
{
	unsigned int 	OpId;
	unsigned int	OpValue;
} TZ_OPERATION, *pTZ_OPERATION;

typedef enum TZ_OPERATION_SPEC_E
{
    TZ_NOP = 0,              	// Enums specify the following:
    TZ_CONFIG_ENABLE,            // TZ config enable flag (1 = configure)
	TZ_IGNORE_INST_TO,		  	// Treat a time out waiting for a bit to set/clear
								// as informational only, operation continues
    // New values go above here
    TZ_OPERATION_SPEC_E_MAX
} TZ_OPERATION_SPEC_T;

typedef struct
{
	WTP_RESERVED_AREA_HEADER WRAH;
    unsigned int		NumberOperations;
	unsigned int 		NumberInstructions;
	TZ_OPERATION		TZ_Operations;      // TZ_Op[NumberOperations];
	INSTRUCTION_S	 	TZ_Instructions;	// TZ_Instruction[NumberInstructions];
} TZ_PACKAGE, *pTZ_PACKAGE;
// End new Trustzone related structures


/****  Used only for backwards compatability!!! ***/
typedef struct
{
 UINT_T Identifier;
 UINT_T PortType;
 UINT_T Port;
 UINT_T GPIOPresent;
 UINT_T NumGpios;
 pGPIO_DEF	GPIO;
}OPT_SET, *pOPT_SET;

/*********************************************************************************/

typedef enum
{
	PlatformVerificationKey,
	NetworkOperatorCAKey,
	SoftwareUpdateCAKey,
	DRMDeviceCAKey,
	OEMReservedKey1,
	OEMReservedKey2,
	NUMKEYMODULES
}
KEYMODULES_T;

typedef enum
{
	Marvell_DS = 0,
	PKCS1_v1_5_Caddo = 1,
	PKCS1_v2_1_Caddo = 2,
	PKCS1_v1_5_Ippcp = 3,
	PKCS1_v2_1_Ippcp = 4,
	ECDSA_256 = 5,
	ECDSA_521 = 6
}
ENCRYPTALGORITHMID_T;

#define Intel_DS Marvell_DS

typedef enum
{
	SHA160 = 20,
	SHA256 = 32,
	SHA512 = 64
}
HASHALGORITHMID_T;

typedef struct
{
 UINT_T Version;
 UINT_T	Identifier;					// "TIMH"
 UINT_T Trusted;					// 1- Trusted, 0 Non
 UINT_T IssueDate;
 UINT_T OEMUniqueID;
} VERSION_I, *pVERSION_I;			// 0x10 bytes

typedef struct
{
 UINT_T WTMFlashSign;
 UINT_T WTMEntryAddr;
 UINT_T WTMEntryAddrBack;
 UINT_T WTMPatchSign;
 UINT_T WTMPatchAddr;
 UINT_T BootFlashSign;
} FLASH_I, *pFLASH_I;				// 0x10 bytes

typedef struct
{
 UINT_T ImageID;					// Indicate which Image
 UINT_T NextImageID;				// Indicate next image in the chain
 UINT_T FlashEntryAddr;			 	// Block numbers for NAND
 UINT_T LoadAddr;
 UINT_T ImageSize;
 UINT_T ImageSizeToHash;
 UINT_T HashAlgorithmID;            // See HASHALGORITHMID_T
 UINT_T Hash[16];					// Reserve 512 bits for the hash
 UINT_T PartitionNumber;
} IMAGE_INFO_3_4_0, *pIMAGE_INFO_3_4_0;			// 0x60 bytes

typedef struct
{
 UINT_T ImageID;					// Indicate which Image
 UINT_T NextImageID;				// Indicate next image in the chain
 UINT_T FlashEntryAddr;			 	// Block numbers for NAND
 UINT_T LoadAddr;
 UINT_T ImageSize;
 UINT_T ImageSizeToHash;
 UINT_T HashAlgorithmID;            // See HASHALGORITHMID_T
 UINT_T Hash[8];					// Reserve 256 bits for the hash
 UINT_T PartitionNumber;			// This is new for V3.2.0
} IMAGE_INFO_3_2_0, *pIMAGE_INFO_3_2_0;			// 0x40 bytes

typedef struct
{
 UINT_T	KeyID;						// Associate an ID with this key
 UINT_T HashAlgorithmID;            // See HASHALGORITHMID_T
 UINT_T KeySize;					// Specified in bits
 UINT_T PublicKeySize;				// Specified in bits
 UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS]; // Contents depend on PublicKeySize
 UINT_T RSAModulus[MAXRSAKEYSIZEWORDS]; // Up to 2K bits
 UINT_T KeyHash[8]; 				// Reserve 256 bits for the hash
} KEY_MOD_3_2_0, *pKEY_MOD_3_2_0;				// 0x22C bytes

#if RVCT
#pragma anon_unions
#endif
typedef struct
{
 UINT_T KeyID;				// Associate an ID with this key
 UINT_T HashAlgorithmID;    // See HASHALGORITHMID_T
 UINT_T KeySize;			// Specified in bits
 UINT_T PublicKeySize;		// Specified in bits
 UINT_T EncryptAlgorithmID;	// See ENCRYPTALGORITHMID_T;
 union
 {
	struct
	{
 		UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS];	// Contents depend on PublicKeySize
		UINT_T RSAModulus[MAXRSAKEYSIZEWORDS];			// Up to 2K bits
	}Rsa;

	struct
	{
        UINT_T PublicKeyCompX[MAXECCKEYSIZEWORDS]; // Contents depend on PublicKeySize
        UINT_T PublicKeyCompY[MAXECCKEYSIZEWORDS]; // Up to 521 bits
        // Pad this struct so it remains consistent with RSA struct
	    UINT_T Reserved[(2*MAXRSAKEYSIZEWORDS)-(2*MAXECCKEYSIZEWORDS)];
	}Ecdsa;
 };

 UINT_T KeyHash[8]; 				// Reserve 256 bits for the hash
} KEY_MOD_3_3_0, *pKEY_MOD_3_3_0;		//

typedef struct
{
 UINT_T KeyID;				// Associate an ID with this key
 UINT_T HashAlgorithmID;    // See HASHALGORITHMID_T
 UINT_T KeySize;			// Specified in bits
 UINT_T PublicKeySize;		// Specified in bits
 UINT_T EncryptAlgorithmID;	// See ENCRYPTALGORITHMID_T;
 union
 {
	struct
	{
 		UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS];	// Contents depend on PublicKeySize
		UINT_T RSAModulus[MAXRSAKEYSIZEWORDS];			// Up to 2K bits
	}Rsa;

	struct
	{
        UINT_T PublicKeyCompX[MAXECCKEYSIZEWORDS]; // Contents depend on PublicKeySize
        UINT_T PublicKeyCompY[MAXECCKEYSIZEWORDS]; // Up to 521 bits
        // Pad this struct so it remains consistent with RSA struct
	    UINT_T Reserved[(2*MAXRSAKEYSIZEWORDS)-(2*MAXECCKEYSIZEWORDS)];
	}Ecdsa;
 };

 UINT_T KeyHash[16]; 				// Reserve 512 bits for the hash
} KEY_MOD_3_4_0, *pKEY_MOD_3_4_0;		//


typedef struct
{
 UINT_T DSAlgorithmID; 		  // See ENCRYPTALGORITHMID_T
 UINT_T HashAlgorithmID;      // See HASHALGORITHMID_T
 UINT_T KeySize;	          // Specified in bits
 UINT_T Hash[8];			  // Reserve 256 bits for optional key hash
 union 	// Note that this union should not be included as part of the hash for TIM in the Digital Signature
 {
	struct
	{
		UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS];
		UINT_T RSAModulus[MAXRSAKEYSIZEWORDS];           	// Up to 2K bits
		UINT_T RSADigS[MAXRSAKEYSIZEWORDS];				// Contains TIM Hash
	}Rsa;

	struct
	{
		UINT_T ECDSAPublicKeyCompX[MAXECCKEYSIZEWORDS]; // Allow for 544 bits (17 words, 68 bytes for use with EC-521)
		UINT_T ECDSAPublicKeyCompY[MAXECCKEYSIZEWORDS];
		UINT_T ECDSADigS_R[MAXECCKEYSIZEWORDS];
		UINT_T ECDSADigS_S[MAXECCKEYSIZEWORDS];
		// Pad this struct so it remains consistent with RSA struct
		UINT_T Reserved[(MAXRSAKEYSIZEWORDS*3)-(MAXECCKEYSIZEWORDS*4)];
	} Ecdsa;
 };
} PLAT_DS, *pPLAT_DS;

// Constant part of the TIMs
typedef struct
{
 VERSION_I      VersionBind;         			// 0
 FLASH_I        FlashInfo;           			// 0x10
 UINT_T         NumImages;           			// 0x20
 UINT_T         NumKeys;						// 0x24
 UINT_T         SizeOfReserved;					// 0x28
} CTIM, *pCTIM;									// 0x2C

// TIM structure for use by DKB/OBM/BootROM
typedef struct
{
 pCTIM 				pConsTIM;			// Constant part
 pIMAGE_INFO_3_4_0	pImg;				// Pointer to Images
 pKEY_MOD_3_4_0		pKey;				// Pointer to Keys
 PUINT				pReserved;			// Pointer to Reserved Area
 pPLAT_DS       	pTBTIM_DS;			// Pointer to Digital Signature
} TIM, *pTIM;

// NTIM structure for use by DKB/OBM/BootROM
typedef struct
{
 pCTIM 				pConsTIM;			// Constant part
 pIMAGE_INFO_3_4_0	pImg;				// Pointer to Images
 PUINT				pReserved;			// Pointer to Reserved Area
} NTIM, *pNTIM;


#define MRD_LENGTH		0x20000 // 128KB
#define VALID_BUF_STMP	0xCAFECAFE
#define IMEI_TYPE		0xCAFE0010
#define END_BUF_STMP	0xDADADADA

typedef struct tag_entry_size_header
{
	unsigned long entrySize:24; // entry size of data pointed by pointer pFileToWrite, including padding size if needed
	unsigned long paddingSize:8; // padding size, for UINT32 alignment
}RD_ENTRY_SIZE;

typedef struct tag_buffer_flash_header
{
	unsigned long	validBufferStamp;
	unsigned long	bufCheckSum;
	unsigned long	version;
	unsigned long	reserve[2];
}RD_HEADER, *pRD_HEADER;

typedef struct tag_flash_entry_header
{
	unsigned long 	entryType;
	RD_ENTRY_SIZE	size;
	unsigned char	fileName[116];
	unsigned long	entryVersion;
	unsigned long	entryDate;
	unsigned long	reserved;
	unsigned long	*pFileToWrite; // it's not saved in MRD area
}RD_ENTRY, *pRD_ENTRY;

/* tim.c Function Proto's */
UINT_T SetTIMPointers( UINT8_T * StartAddr, TIM *pTIM_h);
UINT_T LoadTim(UINT8_T *TIMArea, TIM *pTIM_h, UINT_T SRAMLoad);
pIMAGE_INFO_3_4_0 ReturnPImgPtr(pTIM pTIM_h, UINT_T ImageNumber);
UINT_T ReturnImgPartitionNumber(pTIM pTIM_h, pIMAGE_INFO_3_4_0 pImg);
UINT_T GetTIMValidationStatus();
void SetTIMValidationStatus(UINT_T status);
pIMAGE_INFO_3_4_0 FindImageInTIM(pTIM, UINT_T);
pKEY_MOD_3_4_0 FindKeyInTIM(pTIM pTIM_h, UINT_T ImageKeyID);
pWTP_RESERVED_AREA_HEADER FindPackageInReserved (UINT_T * Retval, pTIM pTIM_h, UINT_T Identifier);
// These 2 functions allow for multiple instances of one "TYPE" of
// package to be retrieved from the reserved area of the TIM
//-----------------------------------------------------------------------------------------------------
pWTP_RESERVED_AREA_HEADER FindFirstPackageTypeInReserved (UINT_T * Retval, pTIM pTIM_h, UINT_T Identifier);
pWTP_RESERVED_AREA_HEADER FindNextPackageTypeInReserved (UINT_T * Retval);
pCIDP_ENTRY FindMyConsumerArray(pTIM pTIM_h, UINT_T CID);
UINT_T CheckReserved (pTIM pTIM_h);
void InitDefaultPort(pFUSE_SET pFuses);
// Returns a pointer to the static tim in tim.c
pTIM GetTimPointer();
#endif
