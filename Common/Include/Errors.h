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
**  FILENAME:	Error.h
**
**  PURPOSE: 	Contains DKB error definitions
**                  
**	History: 	Created 5/1/05
**
******************************************************************************/

#ifndef __ERRORS_H__
#define __ERRORS_H__

#define PASSED   					0
#define FAILED   					1
#define NOTDONE						2
#define TRUE         				1
#define FALSE       				0
#define NULL        				0

#define CheckErrorReturn(x)	{if (x != NoError) return x;}

/**
 * Notification code definitions
 **/
 
#define PlatformBusy						0x1
#define PlatformReady						0x2
#define PlatformResetBBT					0x3
#define PlatformEraseAllFlash				0x4
#define PlatformDisconnect					0x5
#define PlatformEraseAllFlashDone			0x6
#define PlatformEraseAllFlashWithoutBurn	0x7
#define PlatformEraseAllFlashWithoutBurnDone	0x8

/**
 * General error code definitions			0x0 - 0x1F
 **/
#define NoError            					0x0
#define NotFoundError      					0x1
#define GeneralError       					0x2
#define WriteError         					0x3
#define ReadError		   					0x4
#define NotSupportedError  					0x5
#define InvalidPlatformConfigError			0x6
#define InvalidPlatformState				0x7
#define NotEnabledError						0x8

#define InvalidSizeError					0x9
#define ProbeListExhaustError				0xA
#define DDR_NotInitializedError				0xB
#define UnknownReservedPackage				0xD
#define NULLPointer							0xE
#define NANDIDDISMATCH						0xF
#define FBF_VersionNotMatch					0x10
#define FBF_DeviceMoreThanOne				0x11
#define NOVALIDMRD							0x12

// Flash Related Errors 					0x20 - 0x3F
#define EraseError		 					0x20
#define ProgramError						0x21
#define InvalidBootTypeError				0x22
#define ProtectionRegProgramError			0x23
#define NoOTPFound							0x24
#define BBTReadError						0x25
#define MDOCInitFailed						0x26
#define OneNandInitFailed                   0x27
#define MDOCFormatFailed                    0x28
#define BBTExhaustedError                   0x29
#define NANDNotFound                        0x2A
#define SDMMCNotFound                       0x2B
#define FlexOneNANDNotFound                 0x2C
#define SDMMCReadError                      0x2D
#define XIPReadError                        0x2E
#define FlexOneNANDError                    0x2F
#define FlashDriverInitError                0x30
#define FlashFuncNotDefined					0x31
#define OTPError							0x32
#define InvalidAddressRangeError			0x33
#define FlashLockError						0x34
#define ReadDisturbError					0x35
#define FlashReadError                      0x36
#define SPIFlashNotResponding				0x37
#define ImageOverlayError                   0x38
#define FlashAddrNotChunkAlign           	0x39
#define FlashAddrNotWordAlign           	0x3A
#define BBTOUTOFRANGE						0x3B
#define FlashWriteVerifyError               0x3C
#define FlashAddrOutOfRange					0x3d

// DFC Related Errors						0x40 - 0x5F
#define DFCDoubleBitError    				0x40
#define DFCSingleBitError    				0x41
#define DFCCS0BadBlockDetected  			0x42		
#define DFCCS1BadBlockDetected  			0x43
#define	DFCInitFailed						0x44
#define DFCONFIConfigError                  0x45
#define DFC_WRREQ_TO						0x46
#define DFC_WRCMD_TO						0x47
#define DFC_RDDREQ_TO						0x48
#define DFC_RDY_TO							0x49
#define DFCCS0CommandDoneError				0x4A
#define DFCCS1CommandDoneError				0x4B
#define DFC_PGDN_TO							0x4C

// SPI Related Errors
#define SPINORPROGRAMFAIL					0x50
#define SPINORERASEFAIL						0x51

// Security Related Errors 					0x60 - 0x8F
#define InvalidOEMVerifyKeyError			0x60
#define InvalidOBMImageError				0x61
#define SecureBootFailureError				0x62
#define InvalidSecureBootMethodError		0x63
#define UnsupportedFlashError				0x64
#define InvalidCaddoFIFOEntryError      	0x65
#define InvalidCaddoKeyNumberError			0x66
#define InvalidCaddoKeyTypeError			0x67
#define RSADigitalSignatureDecryptError 	0x68
#define InvalidHashValueLengthError     	0x69
#define InvalidTIMImageError				0x6A
#define HashSizeMismatch					0x6B
#define InvalidKeyHashError					0x6C
#define TIMNotFound							0x6D
#define WTMStateError						0x6E
#define FuseRWError							0x6F
#define InvalidOTPHashError					0x70
#define CRCFailedError						0x71
#define SaveStateNotFound					0x72
#define WTMInitializationError				0x73
#define ImageNotFound						0x74
#define InvalidImageHash					0x75
#define MicroCodePatchingError				0x76
#define SetJtagKeyError						0x77
#define WTMDisabled							0x78
#define PlatformVerifyFailure				0x79
#define ImageLoadError                      0x7A
#define IPPCPHASHERROR						0x7B

// Download Protocols						0x90 - 0xAF
#define DownloadPortError					0x90
#define DownloadError						0x91
#define FlashNotErasedError					0x92
#define InvalidKeyLengthError				0x93
#define DownloadImageTooBigError			0x94
#define UsbPreambleError					0x95
#define TimeOutError						0x96
#define UartReadWriteTimeOutError			0x97
#define UnknownImageError					0x98
#define MessageBufferFullError				0x99
#define NoEnumerationResponseTimeOutError 	0x9A
#define UnknownProtocolCmd					0x9B
#define UsbRxError                          0x9C
#define ForceDownloadPseudoError			0x9D
#define UsbTxError                          0x9E

//JTAG ReEnable Error Codes					0xB0 - 0xCF
#define JtagReEnableError					0xB0
#define JtagReEnableOEMPubKeyError			0xB1
#define JtagReEnableOEMSignedPassWdError	0xB2
#define JtagReEnableTimeOutError			0xB3
#define JtagReEnableOEMKeyLengthError   	0xB4

// SDMMC Errors 							0xD0-0xE5
#define SDMMC_SWITCH_ERROR	 				0xD0
#define SDMMC_ERASE_RESET_ERROR	 			0xD1
#define SDMMC_CIDCSD_OVERWRITE_ERROR		0xD2
#define SDMMC_OVERRUN_ERROR 				0xD3
#define SDMMC_UNDERUN_ERROR 				0xD4
#define SDMMC_GENERAL_ERROR 				0xD5
#define SDMMC_CC_ERROR 						0xD6
#define SDMMC_ECC_ERROR 					0xD7
#define SDMMC_ILL_CMD_ERROR 				0xD8
#define SDMMC_COM_CRC_ERROR					0xD9
#define SDMMC_LOCK_ULOCK_ERRROR				0xDA
#define SDMMC_LOCK_ERROR 					0xDB
#define SDMMC_WP_ERROR 						0xDC
#define SDMMC_ERASE_PARAM_ERROR				0xDD
#define SDMMC_ERASE_SEQ_ERROR				0xDE
#define SDMMC_BLK_LEN_ERROR 				0xDF
#define SDMMC_ADDR_MISALIGN_ERROR 			0xE0
#define SDMMC_ADDR_RANGE_ERROR 				0xE1
#define SDMMCDeviceNotReadyError			0xE2
#define SDMMCInitializationError			0xE3
#define SDMMCDeviceVoltageNotSupported		0xE4
#define SDMMCWriteError						0xE5

// WTM and GEU FuseBlock Errors				0x100 - 0x119
#define FUSE_FuseBlockNotActive				0x100
#define FUSE_BufferTooSmall					0x101
#define FUSE_FuseBlockLocked				0x102
#define FUSE_UnsupportedFuseBlock			0x103
#define FUSE_IncompleteBurnRequest			0x104
#define FUSE_NoBurnRequest					0x105
#define FUSE_FuseBlockFieldNotActive		0x106
#define FUSE_BurnError						0x107
#define FUSE_FuseBlockFieldOverflow			0x108
#define FUSE_InvalidFuseBlockField          0x109 
#define FUSE_FuseBlockCompareFailed         0x10A
#define FUSE_InvalidState                   0x10B
#define FUSE_InvalidBufferSize              0x10C
#define FUSE_BitSetError                    0x10D
#define FUSE_FuseBlockLockedAndNotReadable  0x10E
#define FUSE_FuseBlockFieldFull				0x10F
#define FUSE_FuseSoftwareResetTimeout		0x110
#define FUSE_FuseBurnTimeout				0x111
#define FUSE_FuseBlockStickyBitSet			0x112
#define FUSE_UsbPhyBandgapNotEnabled		0x113
#define FUSE_UncorrectableEccError			0x114
#define FUSE_FieldWriteProtectedByEcc		0x115
#define FUSE_LastLogicalBitBurned			0x116
#define FUSE_TamperError					0x117
#define FUSE_FuseReprogrammingError			0x118
#define FUSE_IncompleteFuseFieldsSetup		0x119


// TrustZone Related Errors
#define TZ_Region0NotProtected				0x120
#define TZ_FusesNotConfiguredForTrustzone	0x121
#define TZ_PackageNotFound					0x122
#define TZ_InitDisabled						0x123
#define TZ_UnknownOperation					0x124

//DDR Related Error Codes
#define DDR_Package_Obsolete				0x200
#define DDR_Unknown_Operation				0x201
#define DDR_MemoryTest_Failed				0x202
#define DDR_InitDisabled					0x203
#define DDR_CMCC_Package_Obsolete			0x204
#define DDR_Trustzone_Package_Obsolete		0x205
#define DDR_PackageNotFound					0x206
#define DDR_FusesNotConfiguredForTrustzone	0x207

//Instruction related codes
#define INSTR_UnknownInstruction			0x300
#define INSTR_Timeout						0x301
#define INSTR_InvalidAddress				0x302
#define INSTR_InvalidScratchMemoryId		0x303
#define INSTR_InvalidMaskOrValue			0x304

// qpress related codes
#define QPRESS_ERROR_HEADER_FORMAT			0x350
#define QPRESS_ERROR_IMAGE_FORMAT			0x351
#define QPRESS_NOT_SUPPORT_FLAG				0x352
#define QPRESS_DATA_ERROR					0x353
#define QPRESS_DATA_CRC_ERROR				0x354

// sparse file related codes
#define Sparse_ImageLengthLimitExceeded		 0x400
#define Sparse_BadMagic						 0x401
#define Sparse_IncompatibleFormat			 0x402
#define Sparse_BadChunkSize					 0x403
#define Sparse_BogusDontCareChunk			 0x404
#define Sparse_UnknownChunkId				 0x405
#define Sparse_WriteError					 0x406

#endif
