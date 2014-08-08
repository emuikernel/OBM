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
 
 ******************************************************************************/
/*************************************************************
 * jtag.h
 *
 * Contents:
 *      Definitions and functions declarations used in the 
 *      JTAG Communication protocol. 
 *
 *************************************************************/
#ifndef Jtag_h
#define Jtag_h

#if JTAG_PROTOCOL_OVER_JTAG_SUPPORTED

#include "PlatformJtag.h"

//Timer update macro
#define UpdateJTAGCommunicationProtocolTimeoutTimer() 	 pJTAG_CPCData->LastActivityTime = GetOSCR0()

#define JTAGID		0x4A544147		// "JTAG"

#define  MAX_TRANSACTION_RETRY_COUNT		2
#define  MAX_COMMAND_SEQUENCE_RETRY_COUNT	1


// Key Lengths in bits
#define PKCS1024_LENGTH_IN_BITS	   	1024
#define PKCS2048_LENGTH_IN_BITS		2048
#define ECDSA256_LENGTH_IN_BITS		256
#define ECDSA521_LENGTH_IN_BITS		521

#define PUBLIC_KEY_MAX_SIZE_IN_BITS		PKCS2048_LENGTH_IN_BITS * 2 // Exponent and Modulus
#define PASSWORD_MAX_SIZE_IN_BITS		PKCS2048_LENGTH_IN_BITS
#define NONCE_MAX_SIZE_IN_BITS			256							// Max Nonce Size value in all platforms

#define JTAG_PROTOCOLTIMEOUTVALUE	1000  //milliseconds


#define JTAG_TRANSACTION_CMDREG_INDEX		 	0
#define JTAG_TRANSACTION_FIRST_DATAREG_INDEX	1

// Error Codes
//#define JTAG_NO_ERROR	0

typedef struct _JTAG_CommunicationProtocolControlData{
// PublicKey Download
UINT_T * pPublicKey;
UINT_T PublicKeyBitsReceived;
UINT_T PublicKeySizeInBits;
UINT_T PublicKeyCmdSeqActive;
UINT_T PublicKeyReceived;		//Useful for debug
UINT_T PublicKeyVerified;		//Useful for debug
UINT_T PublicKeyFirstCommand;
UINT_T PublicKeyAlgorithm;
UINT_T PublicKeyHashAlgorithm;
UINT_T PublicKeySchemeIDInHash;

// Password Download
UINT_T *pPassword;
UINT_T PasswordBitsReceived;
UINT_T PasswordSizeInBits;
UINT_T PasswordCmdSeqActive;
UINT_T PasswordReceived;
UINT_T PasswordVerified;
UINT_T PasswordFirstCommand;

// NonceUpload
UINT_T *pNonce;
UINT_T NonceBitsSent;
UINT_T NonceSizeInBits;
UINT_T NonceCmdSeqActive;
UINT_T NonceSent;

// State 
UINT_T ReEnableRequestActive;
UINT_T Initialized;
UINT_T State;
UINT_T LastActivityTime; 
UINT_T MaxTimeoutValue;
UINT_T WaitForSetParameterCommmand;
UINT_T WaitForEndSessionCommand;
UINT_T InterruptHandlerHasBeenCalled;

// NACK data
UINT_T TransactionRetryCount;
UINT_T CommandSequenceRetryCount;

// TransactionValidation
UINT_T CommandToHost[NUM_JTAG_DEBUGKEY_REGISTERS];
UINT_T LastCommandToHost[NUM_JTAG_DEBUGKEY_REGISTERS];
UINT_T CommandFromHost[NUM_JTAG_DEBUGKEY_REGISTERS];
UINT_T LastCommandFromHost[NUM_JTAG_DEBUGKEY_REGISTERS];

// JTAG Request Data
UINT_T JTAGRequestData
}JtagCommunicationProtocolControlData, * pJtagCommunicationProtocolControlData;

//JTAG States
#define PK_DOWNLOAD_STATE		0x01
#define PK_RECEIVED_STATE		0x02
#define PK_VERIFIED_STATE		0x03
//#define INITIATOR_REQUEST		0x04
#define NONCE_UPLOAD_STATE 		0x05
#define NONCE_SENT_STATE   		0x06
#define PW_DOWNLOAD_STATE		0x07
#define PW_RECEIVED_STATE		0x08
#define PW_VERIFIED_STATE		0x09


//Command Register Field Masks and Offsets
#define COMMAND_MASK				0xF8000000		// bits [31:27]
#define DONE_MASK					0x01000000		// bit  [24]
#define TRANSACTION_LENGTH_MASK		0x00FF0000		// bits [23:16]
#define CONTROL_FIELD_MASK			0x00003FFF		// bits [13:0]
#define COMMAND_OFFSET			  	27
#define DONE_OFFSET					24
#define TRANSACTION_LENGTH_OFFSET	16

//Transaction Length Masks
#define TRANSACTION_LENGTH_0_MASK		  0 << TRANSACTION_LENGTH_OFFSET
#define TRANSACTION_LENGTH_9_MASK		  9 << TRANSACTION_LENGTH_OFFSET
#define TRANSACTION_LENGTH_32_MASK		 32 << TRANSACTION_LENGTH_OFFSET
#define TRANSACTION_LENGTH_64_MASK		 64 << TRANSACTION_LENGTH_OFFSET
#define TRANSACTION_LENGTH_73_MASK		 73 << TRANSACTION_LENGTH_OFFSET
#define TRANSACTION_LENGTH_128_MASK		128 << TRANSACTION_LENGTH_OFFSET
#define TRANSACTION_LENGTH_192_MASK		192 << TRANSACTION_LENGTH_OFFSET
#define TRANSACTION_LENGTH_224_MASK		224 << TRANSACTION_LENGTH_OFFSET

//Command Masks
#define START_SESSION_CMD_MASK	0x01 << COMMAND_OFFSET  
#define JTAG_REQ_CMD_MASK	  	0x02 << COMMAND_OFFSET
#define NONCE_CMD_MASK			0x03 << COMMAND_OFFSET
#define PASSWORD_CMD_MASK		0x04 << COMMAND_OFFSET
#define PUBLICKEY_CMD_MASK		0x05 << COMMAND_OFFSET
#define SET_PARAMETER_CMD_MASK	0x06 << COMMAND_OFFSET
#define ACK_CMD_MASK		  	0x07 << COMMAND_OFFSET
#define NACK_CMD_MASK			0x08 << COMMAND_OFFSET
#define END_SESSION_CMD_MASK  	0x1F << COMMAND_OFFSET


//Control Field Definitions
// Common:
#define HOST_SENDING_MASK			 0x00008000		// bit  [15]
#define TARGET_SENDING_MASK			 0x00004000		// bit  [14]
#define GP_CORE_ENABLE_MASK			0x00000001		// General Purpose Processors (e.g. PJ4s)
#define SP_CORE_ENABLE_MASK			0x00000002		// Secure Processor
#define PD_CORE_ENABLE_MASK			0x00000004		// Platform Dependent Processor (e.g. comm)
#define JTAG_REQUEST_TYPE_NIDEN_MASK		0x00000020
#define JTAG_REQUEST_TYPE_SPNIDEN_MASK	0x00000040
#define JTAG_REQUEST_TYPE_SPIDEN_MASK	0x00000080
// Uni-Directional Common:
#define TRANSACTION_SEQUENCE_MASK	 0x00003C00		// bits [13:10]
#define TRANSACTION_SEQUENCE_OFFSET	 0xA

// 0x01:Start
#define WAIT_FOR_SET_PARAMETER_MASK	  0x00000001	// bit [0]
#define PROTOCOL_VERSION_MASK		  0x000000FE	// bits [7:1]
#define WAIT_FOR_SET_PARAMETER_OFFSET 0x0
#define PROTOCOL_VERSION_OFFSET		  0x1
#define PROTOCOL_VERSION		0x2

// Wait For Set Parameter
#define WAIT_FOR_SET_PARAMETER	0x1

// 0x03:Nonce
#define NONCE_DATASIZE_MASK 		 0x000003FF		// bits [9:0]

// 0x04:Password
#define PW_JTAG_ENABLE_MASK		 0x00000080		// bit [7]

// 0x05:PublicKey
#define USE_SCHEME_ID_IN_HASH_MASK	 0x00000100		// bit [8]
#define HASH_MASK					 0x000000F0		// bits [7:4]
#define KEY_MASK        			 0x0000000F		// bits [3:0]
#define USE_SCHEME_ID_IN_HASH_OFFSET 8
#define HASH_OFFSET					 4					
#define KEY_OFFSET					 0
// Hash and Key IDs
#define JTAGID_SHA160	    0x0
#define JTAGID_SHA256	    0x1
#define JTAGID_SHA512	    0x2

#define JTAGID_PKCS1024	   	0x0
#define JTAGID_PKCS2048		0x1
#define JTAGID_ECDSA256		0x2
#define JTAGID_ECDSA521		0x3

// 0x06:SetParameters
// One bit fields that don't need an enable bit
#define SETPARAM_USE_JTAG_INTERRUPTS_MASK		0x80000000	// bit [31]
#define SETPARAM_USE_JTAG_IINTERRUPTS_OFFFSET	31

#define SETPARAM_FORCE_DOWNLOAD_MASK		0x40000000	// bit [30]
#define SETPARAM_FORCE_DOWNLOAD_OFFSET		30

// Multi bit fields that DO need an enable bit
#define SETPARAM_WDT_ENABLE_MASK 		0x00002000	// bit [13]
#define SETPARAM_WDT_MASK				0x00001800	// bits [12:11]
#define SETPARAM_WDT_OFFSET				11

#define SETPARAM_GPIO_SEL_ENABLE_MASK	0x00000200  // bit [10]
#define SETPARAM_GPIO_SEL_MASK		 	0x000001E0	// bits [9:6]
#define SETPARAM_GPIO_SEL_OFFSET		6

#define SETPARAM_BOOTSTATE_ENABLE_MASK	0x00000020  // bit [5] 
#define SETPARAM_BOOTSTATE_MASK		 	0x0000001F	// bits [4:0]
#define SETPARAM_BOOTSTATE_OFFSET		0






// 0x07:ACK
#define INITIATOR_REQUEST_MASK		0x00000200	// bit [9]
#define ACK_COMMAND_STATUS_MASK		0x0000003F	// bits[5:0]
#define INITIATOR_REQUEST_OFFSET	0x9
#define ACK_COMMAND_STATUS_OFFSET   0x0
//ACK Initiator Request
#define NOREQUEST					0x0
#define INITIATOR_REQUEST_FLAG		0x1
//ACK Command Status
#define NOSTATUS					0x00 //ACK/NACK	
#define PUBLICKEY_VERIFIED			0x01
#define JTAG_ENABLED				0x02
#define JTAG_ENABLE_FAILED			0x03
#define JTAG_PERMANENTLY_DISABLED   0x04
#define FA_STATE_SET				0x05
#define TARGET_PARAMETERS_SET		0x06
#define INITIATOR_REQUEST_GRANTED	0x08
#define INITIATOR_REQUEST_DENIED	0x09

// 0x08:NACK
#define NACK_ERROR_CODE_MASK		0x000003C0	// bits [9:6]   
#define NACK_COMMAND_STATUS_MASK	0x0000003F	// bits [5:0]  
#define NACK_ERROR_CODE_OFFSET		0x6
#define NACK_COMMAND_STATUS_OFFSET	0x0
//NACK Command Status
#define RETRY_TRANSACTION			0x01
#define RETRY_COMMAND_SEQUENCE		0x02
#define ABORTING				    0x03
#define BUSY						0x0F
// NACK Errors
#define NOERROR					   	0x00
#define UNSUPPORTED_COMMAND_ERROR  	0x01
#define UNEXPECTED_COMMAND_ERROR   	0x02
#define BAD_VALUE				   	0x03
#define JTAG_TIMEOUT_ERROR		   	0x04
#define UNEXPECTED_SET_PARAMETER	0x05
#define INCORRECT_TRANSACTION_SEQ	0x06
#define RETRY_COUNT_EXCEEDED_ERROR	0x07


UINT_T CheckForJTAGCommunicationProtocolTimeout();
void JTAG_DebugKey_InterruptHandler(void);
void JTAG_StartSession_Handler();
void JTAG_Request_Handler();
void JTAG_SetParameter_Handler();
void JTAG_ACK_Handler();
void JTAG_NACK_Handler();
void JTAG_PublicKey_Handler();
void JTAG_Password_Handler();
void JTAG_EndSession_Handler();
void JTAG_SendCommand(UINT_T * ptrToCommand);
void JTAG_ReceiveCommand();
void JTAG_ACK_Manager(UINT_T status, UINT_T request);
void JTAG_NACK_Manager(UINT_T error_code);
void JTAG_AbortSession();
void JTAG_SendACK(UINT_T command_status, UINT_T InitiatorRequest);
void JTAG_SendNACK(UINT_T status, UINT_T errorcode);
void InitializePublicKeyDownloadParameters();
void InitializePasswordDownloadParameters();
void InitializeNonceUploadParameters();
UINT_T VerifyPublicKeyDeferredHandler();
UINT_T VerifyPasswordDeferredHandler();
UINT_T SendNonceDeferredHandler();
void InitializeJtagProtocolOverJtag();
void InitializeCommandToHostBuffer();
void DumpJTAGDebugKeyRegistersToFFUART();
void DumpJTAG_SPToJTAGMessageRegisters_ToFFUART(UINT_T * ptrToCommand);

#endif //#if JTAG_PROTOCOL_OVER_JTAG_SUPPORTED
#endif // Jtag_h
