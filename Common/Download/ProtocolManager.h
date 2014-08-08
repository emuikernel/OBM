/******************************************************************************
**
** INTEL CONFIDENTIAL
** Copyright 2003-2004 Intel Corporation All Rights Reserved.
**
** The source code contained or described herein and all documents
** related to the source code (Material) are owned by Intel Corporation
** or its suppliers or licensors.  Title to the Material remains with
** Intel Corporation or its suppliers and licensors. The Material contains
** trade secrets and proprietary and confidential information of Intel
** or its suppliers and licensors. The Material is protected by worldwide
** copyright and trade secret laws and treaty provisions. No part of the
** Material may be used, copied, reproduced, modified, published, uploaded,
** posted, transmitted, distributed, or disclosed in any way without Intel's
** prior express written permission.
**
** No license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise. Any license under such intellectual property rights
** must be express and approved by Intel in writing.
**
**	 ProtocolManager.h
******************************************************************************//******************************************************************************
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

/******************************************************************************
**
**  FILENAME:	ProtocolManager.h
**
**  PURPOSE: 	This module manages the boot ROM protocol used for communication
**				over the UART or USB ports
**
**
******************************************************************************/
#ifndef _protocol_manager_h
#define _protocol_manager_h

#include "Typedef.h"
#include "Flash.h"
#include "general.h"
#include "loadoffsets.h"


#define VERSION_OFFSET 0x24
/********************* Image Types ***********************************/
#define FIRSTIMAGE				1

/************* Commands definitions **********************************/

//ErrorCodes for Host Application
#define Ack						0x00
#define Nack					0x01
#define SeqError				0x02
#define JtagAccessGranted       0x11
#define JtagAccessDenied        0xFF
#define JtagREIssued			0x05
#define DebugCmdIssued			0x06
#define DebugBootIssued			0x07
#define Untested				0x02
#define InvalidParameterError   0x08

#define INVALIDCMD				0x00
#define GetVersionCmd			0x20
#define StartAddrCmd			0x21
#define DownloadDataCmd			0x22
#define JumpAddrCmd				0x23
#define OEMPublicKeyCmd			0x24
#define OEMSignedKeyCmd			0x25
#define SelectImageCmd			0x26
#define VerifyImageCmd			0x27
#define PasswordCmd				0x28
#define	DataHeaderCmd			0x2a
#define MessageCmd				0x2b
#define ViewOTPCmd				0x2c
#define DebugBootCmd			0x2d
#define	DoneCmd					0x30
#define DisconnectCmd			0x31
#define UploadDataHeaderCmd     0x32
#define UploadDataCmd           0x33
#define ProtocolVersionCmd      0x34
#define GetParametersCmd        0x35

#define PROTOCOL_WAITTIME		0x80 // in seconds, max time to wait for command
#if CI2_USB_DDR_BUF
#define DATA_BLOCK_SIZE			0x1FF8 //block size + sizeof(ProtocolCmd) must be word aligned
#else
#define DATA_BLOCK_SIZE			0x3f8 //block size + sizeof(ProtocolCmd) must be word aligned
#endif
#define FD_BLOCK_SIZE           0x4000 //Fast download block size
#define UPLOAD_BLOCK_SIZE       0x1FFA // Upload block size. 6 bytes short of USB FIFO size for Status portion,
#define LENGTHOFMESSAGES		0x40
#define MAXNUMOFMESSAGES		0x40
#define PREAMBLE_SIZE           0x4
#define FAST_DOWNLOAD 0x4
#define MESSAGE_FLAG 0x1
#define UPLOAD_DATA_AREA DDR_DOWNLOAD_AREA_ADDR // download and upload use the same ddr address
#if BOOTROM
#define COMMAND_DATALENGTH      0x4 // Maximum command data size except Data Cmd
#else
#define COMMAND_DATALENGTH      0x10 // Maximum command data size
#endif
//Communication Protocol Version
#define COMM_PROTOCOL_MAJOR_VERSION 0x1
#define COMM_PROTOCOL_MINOR_VERSION 0x1
#define COMM_PROTOCOL_REV_VERSION   0x0

//Communication Protocol Parameters
#define COMM_PROTOCOL_BUFFER_SIZE 0x2000
// 3 more reserved parameters

// Upload Cmd Data Source Types
#define UPLOAD_FLASH 0x0

//Download ISR information
typedef struct
{
	volatile UINT8_T PreambleReceived;
	volatile UINT8_T CommandReceived;
	volatile UINT8_T InterruptPort;
	volatile UINT8_T CurrentCommand;
	volatile UINT8_T DownloadsEnabled;
	volatile UINT32_T DataLeft;
	volatile UINT32_T CurrentTransferAddress;
} ProtocolISR, *pProtocolISR;

typedef struct {
    volatile UINT8_T Command;
    volatile UINT8_T Sequence;
    volatile UINT8_T CID;
    volatile UINT8_T Flags;
    volatile UINT32_T  Length;
    volatile UINT8_T Data[COMMAND_DATALENGTH];
} ProtocolCmd, *pProtocolCmd;

typedef struct {
	UINT8_T Command;
	UINT8_T Sequence;
	UINT8_T CID;
	UINT8_T Status;
	UINT8_T Flags;
	UINT8_T Length;
	UINT8_T Data[12];
} ProtocolRsp, *pProtocolRsp;

#define REPORT_LENGTH		4

#define REPORT_NOTIFICATION 			0
#define REPORT_ERROR					1
#define REPORT_BURNT_IMAGE_LENGTH		2
#define REPORT_UPLOAD_NOTIFICATION		3
#define REPORT_FLASH_SIZE_NOTIFICATION		4

typedef struct {
	volatile UINT8_T ReportType; // indicate an error (1) or a notification (0) or others
	volatile UINT8_T Reserved;
	volatile UINT8_T ReportCode[REPORT_LENGTH];
} MessageReport, *pMessageReport;

typedef struct {
	VUINT_T MessageFlag;
	volatile UINT8_T Messages[MAXNUMOFMESSAGES][LENGTHOFMESSAGES];
	volatile UINT8_T Lengths[MAXNUMOFMESSAGES];
	volatile UINT8_T MessageType[MAXNUMOFMESSAGES];
	UINT8_T CurrentMessageIndex;
	volatile UINT8_T NumMessagesQueued;
} ProtocolMsg, *pProtocolMsg;

/*
 * Upload Data structures
 * UploadDataParams; To select data source, address and size
 * ProtocolParams  ; To select buffer size etc. This is meant
 *                 ; to be general for the protocol. But currently
 *                 ; only used for Upload Data.
 */
typedef struct {
	UINT8_T Type;
	UINT8_T SubType;
	UINT8_T Partition;
	UINT8_T Times; // indicate how many times SWD want to do upload
	UINT_T Offset;
	UINT_T DataSize;
	UINT8_T IncludeSpareAreas; // indicate this upload include spare areas or not
	UINT8_T ResetUE; // indicate whether reset UE or not
	UINT8_T ResetTimeDelay; // indicate the delay time before resetting UE
	UINT8_T Reserved2;
} UploadDataParameters, *pUploadDataParameters;

typedef struct
{
	UINT_T BufferSize;
	UINT_T Rsvd1;
	UINT_T Rsvd2;
	UINT_T Rsvd3;
} ProtocolParams, *pProtocolParams;

typedef struct
{
	UINT8_T MajorVersion;
	UINT8_T MinorVersion;
	UINT16_T RevisionVersion;
} ProtocolVersion, *pProtocolVersion;


typedef struct {
	UINT8_T FFUART_Init;
	UINT8_T ALTUART_Init;
	UINT8_T DIFF_USB_Init;
	UINT8_T SE_USB_Init;
    UINT8_T U2D_USB_Init;
    UINT8_T CI2_USB_Init;
} PortStates, *pPortStates;

#define BTUART_D 		1		// Indicate to the routines which device to use
#define FFUART_D 		0 //2
#define DIFF_USB_D	 	2 //3		//	Unsupported for Bulverde DKB
#define SE_USB_D		3 //4		//	Unsupported for Bulverde DKB
#define ALTUART_D		4
#define U2D_USB_D       5
#define CI2_USB_D       6           // ChipIdea code
#define PORT_NONE_D		0xff

//Prototype Definitions
pProtocolISR getProtocolISR(void);
pProtocolCmd getProtocolCmd(void);
pProtocolRsp getProtocolRsp(void);
pProtocolMsg getProtocolMsg(void);
pProtocolParams getProtocolParams(void);

UINT8_T GetPortState(UINT_T);
UINT_T GetCommand(void);
UINT_T GetPreamble(void);
UINT_T SendResponse(UINT8_T *, UINT_T);
UINT_T SendAck();
UINT_T SendError(UINT_T ErrorCode);

UINT_T InitPort(UINT_T, pFUSE_SET);
void ShutdownPort(UINT_T);
UINT_T HandleDoneCmd(void);
UINT_T HandleDisconnect(void);
UINT_T HandleInvalidCmd(void);
UINT_T HandleMessageCmd(void);
UINT_T AddMessageError(UINT_T, UINT_T);
void InitMessageQueue(void);
UINT8_T ProtocolVerifyPreamble( UINT8_T *candidateString );
UINT8_T CheckProtocolTimeOut(UINT_T StartTime);
void ShutdownPorts();
void unknown_protocol_command(pProtocolCmd cmd);
FUNC_STATUS HandleRequest(UINT_T, UINT_T);

/* Upload Command handler and helper functions */
UINT_T HandleUploadDataHeaderCmd();
UINT_T HandleUploadDataCmd();
UINT_T CopyUploadDataIntoBuffer(UINT_T address, pUploadDataParameters pUploadParams);
UINT_T VerifyUploadParameters(pUploadDataParameters pUploadParams);
UINT_T HandleUploadFlow(UINT_T address);

#if DOWNLOAD_USED
	UINT_T HandleGetVersionCmd(void);
	UINT_T HandleSelectImageCmd(UINT_T imageType);
	UINT_T HandleVerifyImageCmd(void);
	UINT_T HandleDataHeaderCmd(void);
	UINT_T HandleDataCmd(void);
	UINT_T HandleDownloadFlow(UINT_T address, UINT_T image_id);
#endif // #if DOWNLOAD_USED


	UINT_T GetTIMValidationStatus();
	void SetTIMValidationStatus(UINT_T status);


#endif
