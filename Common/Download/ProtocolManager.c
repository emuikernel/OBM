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
**	 ProtocolManager.c
******************************************************************************/

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

/******************************************************************************
 *
 *	The boot ROM protocol is documented in the boot ROM specification.  This
 * implements the protocol over the USB or UART port.  It is expected that
 * the driver for the port has a minimal set of routines to handle receiving
 * and responding to commands.  This file will interpret the data and prepare
 * the response data for a particular command.  The driver is resonsible for
 * sending and receiving the data over the port.
 *
 *	Basic process:
 *     - Interrupt is received from one of the ports
 *     - The HandleRequest routine is called to get the command from the port
 *     - Based on the request type either a download request or a JTAG request
 *		 is started.  At this point the other ports are disabled until the
 *	     request is completed.
 *
 *
 ******************************************************************************/

#include "ProtocolManager.h"
#include "Errors.h"
#include "misc.h"
#include "Flash.h"
#include "FM.h"
#include "Interrupts.h"
#include "tim.h"
#include "PlatformConfig.h"
#include "loadoffsets.h"
#include "nand.h"

#if USBCI
// defined in CI2Download
extern UINT8_T *transmitBuff;
#endif
// Here's how to interprit the version
// MAJOR
// A (stepping) = 1
// B (stepping) = 2
// C (stepping) = 3
// D (stepping) = 4
// MINOR
// This is basically the stepping number, 0=0, 1=1, 2=2 etc
// For example:
// A0 = 1000
// A1 = 1001
// B2 = 2002
// C0 = 3000
// C2 = 3002


const UINT8_T preambleString[] = { 0x00, 0xD3, 0x02, 0x2B };
static UINT8_T upload_config_flash_flag = 0;
static UINT8_T upload_get_times_flag = 0;
UINT8_T upload_times = 1;

UINT8_T ResetUE = 0;
UINT8_T ResetDelay = 0;

#if NAND_CODE
extern UINT_T upload_nand_spare;
extern UINT_T NandID;
#endif

__attribute__((aligned(8)))ProtocolCmd GProtocolCmd;
__attribute__((aligned(8)))ProtocolRsp GProtocolRsp;
__attribute__((aligned(8)))ProtocolMsg GProtocolMsg;
__attribute__((aligned(8)))PortStates GPortState;

__attribute__((aligned(8)))ProtocolISR GProtocolISR = { 
	FALSE,						// Preamble received
	FALSE,						// Command received 
	PORT_NONE_D,				// Interrupt port
	INVALIDCMD,					// Current command
	FALSE,						// Downloads enabled
	FALSE,						// Data left	
	NULL						// Current download address
};

pProtocolISR getProtocolISR(void) {
	return &GProtocolISR;
}

pProtocolCmd getProtocolCmd(void) {
	return &GProtocolCmd;
}

pProtocolRsp getProtocolRsp(void) {
	return &GProtocolRsp;
}

pProtocolMsg getProtocolMsg(void) {
	return &GProtocolMsg;
}

static UINT8_T sProtocolError = NoError;
void setProtocolError(UINT8_T Error)
{
    sProtocolError = Error;
}

UINT8_T getProtocolError()
{
    return sProtocolError;
}

UINT_T GetCommand()
{
	UINT_T Retval = NoError;
	UINT_T StartTime;

	pProtocolISR pISR = getProtocolISR();

	StartTime = GetOSCR0(); //Dummy read to flush potentially bad data
	StartTime = GetOSCR0();

	while(pISR->CommandReceived == FALSE)
	{
		Retval = CheckProtocolTimeOut(StartTime);
		if(Retval != NoError)
			return Retval;
	}
	pISR->CommandReceived = FALSE;

	return Retval;
}

UINT_T GetPreamble()
{
	UINT_T Retval = NoError;
	UINT_T StartTime;

	StartTime = GetOSCR0(); //Dummy read to flush potentially bad data
	StartTime = GetOSCR0();
	
	while(getProtocolISR()->PreambleReceived == FALSE)
	{
		Retval = CheckProtocolTimeOut(StartTime);
		if(Retval != NoError)
			return Retval;
	}

	return Retval;
}
/*
 *  Dependent on Function in USB1.1 and 2.0 files
 */
UINT_T SendResponse(UINT8_T *pAckBuff, UINT_T size)
{
	UINT_T Retval = NoError;
	UINT_T i = 0;

	getProtocolISR()->CommandReceived = FALSE;

	switch(getProtocolISR()->InterruptPort){
#if USBCI
	case CI2_USB_D:
		Retval = PlatformCI2Transmit(size, pAckBuff, TRUE);
		break;
#endif
	default:
		Retval = NotSupportedError;
		break;
	}
	return Retval;
}

UINT_T SendAck()
{
	UINT_T Retval = NoError;
	pProtocolCmd pCmd = getProtocolCmd();
	pProtocolRsp pRsp = getProtocolRsp();

	pRsp->Command = pCmd->Command;
	pRsp->Sequence = pCmd->Sequence;
	pRsp->CID = pCmd->CID;
	pRsp->Status = Ack;
    pRsp->Flags = pRsp->Flags & ~(MESSAGE_FLAG);
	pRsp->Flags = pRsp->Flags | getProtocolMsg()->MessageFlag;

	Retval = SendResponse(&pRsp->Command, pRsp->Length + 6);

	return Retval;
}

UINT_T SendError(UINT_T ErrorCode){

	UINT_T Retval = NoError;
	UINT_T RspSize;
	//UINT_T i;
	pProtocolCmd pCmd = getProtocolCmd();
	pProtocolRsp pRsp = getProtocolRsp();

	pRsp->Command = pCmd->Command;
	pRsp->Sequence = pCmd->Sequence;
	pRsp->CID = pCmd->CID;
	pRsp->Status = Nack;
	pRsp->Flags = getProtocolMsg()->MessageFlag;
	if(ErrorCode)
	{
		pRsp->Length = 4;
        *(UINT16_T*)&pRsp->Data[0] = ErrorCode & 0x0000FFFF;
        *(UINT16_T*)&pRsp->Data[2] = (ErrorCode & 0xFFFF0000) >> 16;
	}
	else
		pRsp->Length = 0;
	
	RspSize = ErrorCode ? 6 + pRsp->Length : 6; 
	Retval = SendResponse(&pRsp->Command, RspSize);

	return Retval;
}

void ShutdownPort(UINT_T port)
{
	if(GetPortState(port) != 1) return;
	switch(port)
	{
#if USBCI
	case CI2_USB_D:
		DisablePeripheralIRQInterrupt(USB0_OTG_INT);
		PlatformCI2Shutdown();
		break;
#endif
	default:
		break;

		return;
	}
}

UINT8_T GetPortState(UINT_T port)
{
	UINT8_T Retval;
	switch(port)
	{
	case FFUART_D:
		Retval = GPortState.FFUART_Init;
		break;
	case ALTUART_D:
		Retval = GPortState.ALTUART_Init;
		break;
	case DIFF_USB_D:
		Retval = GPortState.DIFF_USB_Init;
		break;
	case SE_USB_D:
		Retval = GPortState.SE_USB_Init;
		break;
	case U2D_USB_D:
		Retval = GPortState.U2D_USB_Init;
		break;
	case CI2_USB_D:
		Retval = GPortState.CI2_USB_Init;
		break;
	default:
		Retval = DownloadPortError;
		break;

	}
	return Retval;
}

void SetPortAsInitialize(UINT_T port)
{
	switch(port)
	{
	case FFUART_D:
		GPortState.FFUART_Init = 1;
		break;
	case ALTUART_D:
		GPortState.ALTUART_Init = 1;
		break;
	case DIFF_USB_D:
		GPortState.DIFF_USB_Init = 1;
		break;
	case SE_USB_D:
		GPortState.SE_USB_Init = 1;
		break;
	case U2D_USB_D:
		GPortState.U2D_USB_Init = 1;
		break;
	case CI2_USB_D:
		GPortState.CI2_USB_Init = 1;
		break;
	default:
		break;
	}
	return;
}
/*************************************************************************************
*	Functions to handle specific commands
*************************************************************************************/


UINT_T HandleGetVersionCmd()
{
	UINT_T	Retval= 0;
	pProtocolRsp pRsp;

//	UINT8_T  *Version = (UINT8_T *)(VERSION_OFFSET + BOOTROMBASEADDR);	// legacy way to get version info...
//    Version used to come from BootROM's version info.
//    But now the BootROM's memory space is unavailable to BootLoader.
//    Furthermore, it is probably more appropriate to return the version 
//    info of whichever executable is running: BootROM or BootLoader.
	extern UINT8_T	ExecutableVersionInfo[];	// defined in [TBR|BL]_startup.s
	UINT8_T  *Version = ExecutableVersionInfo;	// new way to get version info.

	pRsp = getProtocolRsp();

	pRsp->Length = 12;
	memcpy( (void *)&(pRsp->Data[0]), Version, 12);

	Retval = SendAck();

	return Retval;
}

#if DOWNLOAD_USED

UINT_T HandleSelectImageCmd(UINT_T imageType)
{
	UINT_T Retval;
	pProtocolRsp pRsp = getProtocolRsp();

	pRsp->Length = 4;

	*(UINT16_T *)&pRsp->Data[0] = imageType & 0x0000FFFF;
	*(UINT16_T *)&pRsp->Data[2] = (imageType & 0xFFFF0000) >> 16;

	Retval = SendAck();

	return Retval;
}

#endif // end if DOWNLOAD_USED

#if DOWNLOAD_USED

UINT_T HandleVerifyImageCmd()
{
	UINT_T	Retval= 0;

	getProtocolRsp()->Length = 0;

	if(getProtocolCmd()->Data[0] == Nack)
		Retval = UnknownImageError;

	Retval = SendAck();
	return Retval;

}

#endif // end if DOWNLOAD_USED

#if DOWNLOAD_USED

UINT_T HandleDataHeaderCmd()
{
	UINT_T	Retval = 0;
	UINT_T	RemainingLen = 0;
    UINT_T  BufferSize = 0;

	pProtocolRsp pRsp = getProtocolRsp();
	pProtocolISR pISR = getProtocolISR();
	pProtocolCmd pCmd = getProtocolCmd();

	pRsp->Length = 4;
	RemainingLen = *(UINT32_T *)&pCmd->Data[0];

	//if we are downloading into ISRAM
	if((pISR->CurrentTransferAddress & ISRAM_PHY_ADDR) == ISRAM_PHY_ADDR)
	{
		//check to see if we will run out of ISRAM
		if((RemainingLen + pISR->CurrentTransferAddress) > (ISRAM_IMAGE_LOAD_BASE + PLATFORMISRAMLIMIT))
		{
			AddMessageError(REPORT_ERROR, DownloadImageTooBigError);
			Retval = SendError(0);
			return DownloadImageTooBigError;
		}
	}
    
    if (pCmd->Flags & FAST_DOWNLOAD)
    {
        BufferSize = (RemainingLen > FD_BLOCK_SIZE) ? FD_BLOCK_SIZE : RemainingLen;
        pRsp->Flags = pRsp->Flags | FAST_DOWNLOAD;
    }
    else
    {
        BufferSize = (RemainingLen > DATA_BLOCK_SIZE) ? DATA_BLOCK_SIZE : RemainingLen;
    	pRsp->Flags = pRsp->Flags & ~FAST_DOWNLOAD;
    }
	
	*(UINT16_T*)&pRsp->Data[0] = BufferSize & 0x0000FFFF;
	*(UINT16_T*)&pRsp->Data[2] = (BufferSize & 0xFFFF0000) >> 16;
    
	Retval = SendAck();

	return Retval;
}

#endif // end if DOWNLOAD_USED

#if DOWNLOAD_USED

UINT_T HandleDataCmd()
{
	UINT_T	Retval = 0;

	getProtocolRsp()->Length = 0;

	Retval = SendAck();
	
	return Retval;
}

#endif // end if DOWNLOAD_USED

UINT_T HandleDoneCmd()
{
	UINT_T	Retval= 0;

	getProtocolRsp()->Length = 0;

	getProtocolISR()->PreambleReceived = FALSE;

	Retval = SendAck();
	return Retval;

}


// debug info for unknow protocol command
void unknown_protocol_command(pProtocolCmd cmd)
{
	UINT_T i;
#if USE_SERIAL_DEBUG
	serial_outstr("cmd->Command\n");
	serial_outnum(cmd->Command);
	serial_outstr("\n");
	serial_outstr("cmd->CID\n");
	serial_outnum(cmd->CID);
	serial_outstr("\n");
	serial_outstr("cmd->Sequence\n");
	serial_outnum(cmd->Sequence);
	serial_outstr("\n");
	serial_outstr("cmd->Flags\n");
	serial_outnum(cmd->Flags);
	serial_outstr("\n");
	serial_outstr("cmd->Length\n");
	serial_outnum(cmd->Length);
	serial_outstr("\n");

	for (i = 0; i < cmd->Length; i++)
	{
		serial_outstr("cmd->Data++\n");
		serial_outnum(cmd->Data[i]);
		serial_outstr("\n");
	}
	
#endif
}

UINT_T HandleInvalidCmd()
{
	UINT_T	Retval= 0;

	getProtocolRsp()->Length = 0;

	getProtocolISR()->PreambleReceived = FALSE;

	Retval = SendError(UnknownProtocolCmd);
	unknown_protocol_command(getProtocolCmd());
	return Retval;

}

UINT_T HandleMessageCmd(void){
	UINT_T Retval= 0;

#if VERBOSE_MODE

	UINT_T mIndex;
	UINT_T size;
	UINT_T i;
	//UINT8_T Buff[LENGTHOFMESSAGES + 6];
    UINT8_T *Buff = transmitBuff;
	pProtocolCmd pCmd = getProtocolCmd();
	pProtocolMsg pMsg = getProtocolMsg();

	ClearPortInterruptFlag();
	//tool sending extra message commands to catch messages
	//if we don't have any messages, just NACK the message and continue
	if(pMsg->NumMessagesQueued == 0){
		Retval = SendError(0);
		return Retval;
	}

	//must use Buff instead of GProtocolRsp
	Buff[0] = pCmd->Command;
	Buff[1] = pCmd->Sequence;
	Buff[2] = pCmd->CID;
	Buff[3] = Ack;

	mIndex = pMsg->CurrentMessageIndex;
	size = pMsg->Lengths[mIndex];

	Buff[5] = size;
	for(i = 0; i < size; i++)
		Buff[6+i] = pMsg->Messages[mIndex][i];

	//increment the pointer to the new current message
	pMsg->CurrentMessageIndex++;
	pMsg->CurrentMessageIndex %= MAXNUMOFMESSAGES;

	pMsg->NumMessagesQueued--;

	if(pMsg->NumMessagesQueued == 0)
		pMsg->MessageFlag = 0;
	else
		pMsg->MessageFlag = 1;

	//mark flags (message flag      message type flag   )
	Buff[4] = (pMsg->MessageFlag | (pMsg->MessageType[mIndex] << 1));

	Retval = SendResponse(&Buff[0], 6+size);

#else // end if VERBOSE_MODE

	Retval = SendError(0);
#endif
	return Retval;
}

/*	InitPort()
 *
 *	Call into the driver to setup the ports and enable interrupts
 *
 */
UINT_T InitPort(UINT_T port, pFUSE_SET pFuses)
{

	UINT_T Retval = NoError;
	pProtocolISR pISR = getProtocolISR();

	//SetTIMValidationStatus(Untested);

	// setup up ISR status info
	pISR->DownloadsEnabled = TRUE;
	pISR->PreambleReceived = FALSE;
	pISR->CommandReceived = FALSE;
	pISR->InterruptPort = 0xff;

	switch(port){
#if USBCI
	case CI2_USB_D:
		PlatformCI2Init();
		break;
#endif
	default:
		Retval = NotSupportedError;
		break;
	}
	if(Retval == NoError) SetPortAsInitialize(port);
	return Retval;
}

/* ShutdownPorts */
void ShutdownPorts ()
{
	ShutdownPort(CI2_USB_D);
}

UINT_T HandleDisconnect()
{
	UINT_T Retval = 0;
	//counters
	// c1 - keep track of NACK's based on message left in queue.  We should give up
	//      waiting for a message command after a couple tried.  This allows for
	//		host tool to run with messaging off
	UINT_T c1 = 0;
	// c2 - keeps track of commands received that aren't the Disconnect command.
	//		If there was a error in previous download, then we have a different
	//		command.  However, if this happens multiple times, either Host tool
	//		is hosed, or we lost synch with tool.  Either way, quit.
	UINT_T c2 = 0;
	pProtocolRsp pRsp;
	pProtocolMsg pMsg;
	pProtocolISR pISR;

	pRsp = getProtocolRsp();
	pMsg = getProtocolMsg();
	pISR = getProtocolISR();

	//if we never had an interrupt, then just return
	if(pISR->InterruptPort == 0xff)
		return NoError;

	pRsp->Length = 0;

	//wait until we get the command
	do{
		Retval = GetCommand();
		if(Retval != NoError)
			return Retval;

		if(getProtocolCmd()->Command != DisconnectCmd){
			Retval = SendError(0);
			if(c2 > 1)
			{
				unknown_protocol_command(getProtocolCmd());
				return UnknownProtocolCmd;
			}
			c2++;
			continue;
		}

		//if the tool is ignoring messages, break out of loop
		if(c1 >= 1) break;

		//if we still have messages, NACK the disconnect, and wait
		if(pMsg->MessageFlag == 1)
		{
			c1++;
			Retval = SendError(0);
			continue;
		}
		break;
	} while (TRUE);

	ClearPortInterruptFlag();
	//acknowledge the command
	Retval = SendAck();

	//clear status fields
	pISR->InterruptPort = 0xff;
	pISR->PreambleReceived = FALSE;
	pISR->CommandReceived = FALSE;

	return Retval;
}

#if DOWNLOAD_USED

/*
 * HandleDownloadFlow()
 *
 *	This function will handle a download request unitl it completes or fails
 *	and return the status
 */

UINT_T HandleDownloadFlow(UINT_T address, UINT_T image_id)
{
	UINT_T Retval;
	pProtocolCmd pCmd = getProtocolCmd();
	pProtocolISR pISR = getProtocolISR();

	pISR->CurrentTransferAddress = address;

	do{
		switch(pCmd->Command)
		{
		case GetVersionCmd:
			Retval = HandleGetVersionCmd();
			break;
		case SelectImageCmd:
			Retval = HandleSelectImageCmd(image_id);
			break;
		case VerifyImageCmd:
			Retval = HandleVerifyImageCmd();
			break;
		case DataHeaderCmd:
			Retval = HandleDataHeaderCmd();
			break;
		case DownloadDataCmd:
			Retval = HandleDataCmd();
			break;
		default:
			if(pISR->CommandReceived == TRUE)
			{
				Retval = SendError(0);
				unknown_protocol_command(getProtocolCmd());
				Retval = UnknownProtocolCmd;
			}else
				Retval = NoError;
			break;
		}

		if (Retval != NoError)
		{
		#if USE_SERIAL_DEBUG
			serial_outstr("Retval\n");
			serial_outnum(Retval);
			serial_outstr("\n");
		#endif
			break;
		}
		
		Retval = GetCommand();
		if((Retval != NoError))
			break;
		
	}while (pCmd->Command != DoneCmd);

	return Retval;
}

#endif // end if DOWNLOAD_MODE

#if DOWNLOAD_USED
/*
 * HandleDownloadFlow()
 *
 *	This function will handle a upload request unitl it completes or fails
 *	and return the status
 */

UINT_T HandleUploadFlow(UINT_T address)
{
	UINT_T Retval;
	pProtocolCmd pCmd = getProtocolCmd();
	pProtocolISR pISR = getProtocolISR();

	pISR->CurrentTransferAddress = address;

	do{
		switch(pCmd->Command)
		{
		case UploadDataHeaderCmd:
			Retval = HandleUploadDataHeaderCmd();
			break;
		case UploadDataCmd:
			Retval = HandleUploadDataCmd();
			break;
		default:
			if(pISR->CommandReceived == TRUE)
			{
				Retval = SendError(0);
				unknown_protocol_command(getProtocolCmd());
				Retval = UnknownProtocolCmd;
			}else
				Retval = NoError;
			break;
		}
        if(Retval != NoError){
			break;
		}
		GetCommand();
		
	}while (pCmd->Command != DoneCmd);
	return Retval;
}
#endif

/**
 * Debug Commands
 **/

#if DOWNLOAD_USED
UINT_T HandleUploadDataHeaderCmd()
{
    UINT_T Retval = NoError, startTime, endTime, txDataSize;
    pProtocolCmd pCommand;
    pProtocolRsp pRsp;
    pProtocolISR pISR;
    UploadDataParameters uploadParams, *pUploadParams;

#if NAND_CODE
	P_NAND_Properties_T pNandP = GetNANDProperties();
	UINT_T nand_spare_size;
	UINT_T numbers, total_spare_size;
#endif
    
    pUploadParams = &uploadParams;
    pCommand = getProtocolCmd();
    pISR = getProtocolISR();
    memcpy((void*)pUploadParams,(void*)&pCommand->Data[0],sizeof(UploadDataParameters));

	if (!upload_get_times_flag)
	{
		upload_times = pUploadParams->Times; // upload_times >= 1
		
	#if USE_SERIAL_DEBUG
		serial_outstr("upload_times\n");
		serial_outnum(upload_times);
		serial_outstr("\n");
	#endif

		ResetUE = pUploadParams->ResetUE;
		ResetDelay = pUploadParams->ResetTimeDelay;

		serial_outstr("ResetUE\n");
		serial_outnum(ResetUE);
		serial_outstr("\n");
		serial_outstr("ResetDelay\n");
		serial_outnum(ResetDelay);
		serial_outstr("\n");
	
		upload_get_times_flag = 1;
	}
    
    // Are the upload parameters ok? 
    Retval = VerifyUploadParameters(pUploadParams);
    AddMessageError(REPORT_NOTIFICATION, PlatformBusy);
    
    if(Retval != NoError)
    {
        AddMessageError(REPORT_NOTIFICATION, PlatformReady);
        SendError(Retval);
        return Retval;
    }
    else
    {
        getProtocolRsp()->Length = 0;
        SendAck();
    }
    
    // Copy data requested into upload buffer
    
    //AddMessageError(PlatformBusy);
    //This could take a while
    
    
    Retval = CopyUploadDataIntoBuffer(pISR->CurrentTransferAddress, pUploadParams);
    AddMessageError(REPORT_NOTIFICATION, PlatformReady);
    // data should in buffer now
    
    if(Retval != NoError)
    {   
        setProtocolError(Retval);
        // This was probably an error reading from source as UploadParams checked out ok.
        // Set to No Error so that error can be sent to Host in HandleUploadDataCmd.
        Retval =  NoError;
    }
    else
    {
    #if NAND_CODE
    	if (upload_nand_spare == TRUE)
    	{
    		nand_spare_size = pNandP->SpareAreaSize; // get nand spare areas size
    		numbers = pUploadParams->DataSize / pNandP->PageSize; // calculate spare areas number
			total_spare_size = numbers * nand_spare_size; // total spare size
			
			pISR->DataLeft = pUploadParams->DataSize + total_spare_size; // data length + total spare length
    	}
		else
	#endif
		{
        	pISR->DataLeft = pUploadParams->DataSize;
		}
    }
    return Retval;

}

UINT_T HandleUploadDataCmd()
{
    UINT_T Retval = NoError, startTime, endTime, txDataSize = 0;
    pProtocolRsp pRsp = (pProtocolRsp)transmitBuff;
    pProtocolCmd pCmd = getProtocolCmd();
    pProtocolISR pISR = getProtocolISR();
    
	Retval = getProtocolError();
    if(Retval!= NoError)
    {
    // We had a problem reading data for Data Upload
        SendError(Retval);
        return Retval;
    }
    if(pISR->DataLeft <=0)
    {
        Retval = SeqError;
        SendError(Retval);
        return Retval;
    }  
    
    pRsp->Command = UploadDataCmd;
	pRsp->Sequence = pCmd->Sequence;
	pRsp->CID = pCmd->CID;
	pRsp->Status = Ack;
    
    // Too bad length is only a byte. Not using it.
    pRsp->Length = 0;
      
    txDataSize = pISR->DataLeft < UPLOAD_BLOCK_SIZE ? pISR->DataLeft : UPLOAD_BLOCK_SIZE;
    // Not a fan but have to do this. We have 6 byte status and arbitrary data.
    memcpy((void*)&pRsp->Data[0], (void*)pISR->CurrentTransferAddress, txDataSize);
    
    
    // Message bit should NOT be set in flags.
    // At this point getProtocolMsg()->MessageFlag will be 0.
    pRsp->Flags = pRsp->Flags & ~(MESSAGE_FLAG);
	pRsp->Flags = pRsp->Flags | getProtocolMsg()->MessageFlag;
    
	Retval = SendResponse(&pRsp->Command, txDataSize + 6);
    
    pISR->CurrentTransferAddress += txDataSize;
    pISR->DataLeft -= txDataSize;
    
    return Retval;
}    
    
UINT_T CopyUploadDataIntoBuffer(UINT_T address, pUploadDataParameters pUploadParams)
{
    UINT_T Retval = NoError;
    // Need to implement GetTimPointer
    pTIM pTIM_h = GetTimPointer();
	
    switch(pUploadParams->Type)
    {
        case UPLOAD_FLASH:
            // Fix me. Need code to check if sub type of flash is supported.
			if (!upload_config_flash_flag)
			{
            	Retval = Configure_Flashes(pUploadParams->SubType & 0x1F, BOOT_FLASH);
				if (Retval != NoError)
					FatalError(Retval);
				
            	// This will invalidate BBT table, PT State etc.
            	InitializeFM(LEGACY_METHOD, BOOT_FLASH);

				upload_config_flash_flag = 1; // configure flash for upload only one time
			}

		#if NAND_CODE

			if (pUploadParams->ResetUE == 1)
			{
				AddMessageError(REPORT_UPLOAD_NOTIFICATION, NandID);
			}
		
			if (pUploadParams->IncludeSpareAreas == TRUE)
			{
				upload_nand_spare = TRUE;
			}
			else
			{
				upload_nand_spare = FALSE;
			}
		#endif
            // Set the partition.
		#if MMC_CODE
            SetPartition(pUploadParams->Partition, BOOT_FLASH);
		#endif
			
            Retval = ReadFlash(pUploadParams->Offset, address, pUploadParams->DataSize, BOOT_FLASH);
        break;
        default:
        // Not supported.
            Retval = NotSupportedError;
        break;
            
    }
    return Retval;
}

UINT_T VerifyUploadParameters(pUploadDataParameters pUploadParams)
{
    UINT_T Retval = NoError, flashNum;
    pTIM pTIM_h = GetTimPointer();
    switch(pUploadParams->Type)
    {
        case UPLOAD_FLASH:
            // Is flash subtype supported. Looking in Tim.
            flashNum = pTIM_h->pConsTIM->FlashInfo.BootFlashSign & 0xFF;
            if(flashNum != pUploadParams->SubType & 0xFF)
                Retval = UnsupportedFlashError;
        break;
        default:
        // Not supported.
            Retval = NotSupportedError;
        break;
            
    }
    return Retval;
}
    
UINT_T HandleProtocolVersionCmd()
{
	UINT_T Retval = 0;
	pProtocolRsp pRsp;
	pProtocolCmd pCmd;
	pProtocolVersion pVersion;
	
	pCmd = getProtocolCmd();
	pRsp = getProtocolRsp();
    
	pRsp->Command = pCmd->Command;
	pRsp->Sequence = pCmd->Sequence;
	pRsp->CID = pCmd->CID;
	pRsp->Status = Ack;
	pRsp->Length = 4;
	
	pRsp->Data[0] = COMM_PROTOCOL_MAJOR_VERSION;
	pRsp->Data[1] = COMM_PROTOCOL_MINOR_VERSION;
	*(UINT16_T*)&pRsp->Data[2] = COMM_PROTOCOL_REV_VERSION;
	
	pRsp->Flags = pRsp->Flags & ~(MESSAGE_FLAG);
	pRsp->Flags = pRsp->Flags | getProtocolMsg()->MessageFlag;
	
	SendResponse(&pRsp->Command, 6 + pRsp->Length);
	return Retval;
}

UINT_T HandleGetParametersCmd()
{
	UINT_T Retval = 0;
	pProtocolRsp pRsp = (pProtocolRsp)transmitBuff;
	pProtocolCmd pCmd;
	
	pCmd = getProtocolCmd();
	pRsp = getProtocolRsp();
    
	pRsp->Command = pCmd->Command;
	pRsp->Sequence = pCmd->Sequence;
	pRsp->CID = pCmd->CID;
	pRsp->Status = Ack;
	// Currently only sending buffer size. But we have 3 more reserved paramters. 
	// Note: Rsp structure only has space for 12 bytes of data ( 3 params). If we 
	// are going to use all 4 parameters, we need a buffer and memcpy.
	pRsp->Length = 16;
	
	*(UINT16_T *)&pRsp->Data[0] = COMM_PROTOCOL_BUFFER_SIZE & 0xffff;
	*(UINT16_T *)&pRsp->Data[2] = (COMM_PROTOCOL_BUFFER_SIZE & 0xffff0000)>> 16;
	pRsp->Flags = pRsp->Flags & ~(MESSAGE_FLAG);
	pRsp->Flags = pRsp->Flags | getProtocolMsg()->MessageFlag;
	
	// This should be
    // SendResponse(&pRsp->Command, 6 + pRsp->Length);
    // See note above
    SendResponse(&pRsp->Command, 6 + pRsp->Length);
	return Retval;
}
#endif
UINT_T AddMessageError(UINT_T ReportType, UINT_T ReportCode)
{
	UINT_T Retval = 0;

  #if VERBOSE_MODE
	UINT_T mIndex, i;
	pProtocolMsg pMsg = getProtocolMsg();
	MessageReport Report;

	if(pMsg->NumMessagesQueued == MAXNUMOFMESSAGES)
		return MessageBufferFullError; //this is when message buffer is full

	//find the next open slot in the Queue
	//It will be right after the last good message
	//this is a circular queue, so we use the '%' operation
	mIndex = (pMsg->CurrentMessageIndex + pMsg->NumMessagesQueued) % MAXNUMOFMESSAGES;

	Report.ReportType = ReportType;
	Report.Reserved = 0;

	for (i = 0; i < 4; i++)
	{
		Report.ReportCode[i] = (ReportCode >> (8 * i)) & 0xff;
	}

	// copy message to buffer
	pMsg->Lengths[mIndex] = 6;

	pMsg->Messages[mIndex][0] = Report.ReportType;
	pMsg->Messages[mIndex][1] = Report.Reserved;

	for (i = 0; i < 4; i++)
	{
		pMsg->Messages[mIndex][i + 2] = Report.ReportCode[i];
	}
	
	pMsg->MessageType[mIndex] = 1; // Message type 1 indicate report code
	//set flag
	pMsg->MessageFlag = MESSAGE_FLAG;

	pMsg->NumMessagesQueued++;

  #endif //endif VERBOSE_MODE
	return NoError;
}

void InitMessageQueue(void){

	pProtocolMsg pMsg = getProtocolMsg();
	// setup message buffer
	pMsg->MessageFlag = 0;
	pMsg->CurrentMessageIndex = 0;
	pMsg->NumMessagesQueued = 0;

}



/*
 *	HandleRequest()
 *
 *	This is the main function of the protocol manager.  It is the entry point
 *	to start a request over a port after an interrupt is received.  The process
 *	for handling a request is as follows:
 *
 *	1) Get the preamble from the port and verify it
 *	2) Get the first command packet from the port
 *	3) Send back and ack with proper flag settings
 *	4) Based on the command jump to the download flow or the JTAG flow
 *	5) Complete the download or JTAG request and when complete disconnect
 *	6) return status to the caller
 *
 *
 */

FUNC_STATUS HandleRequest(UINT_T address, UINT_T image_id)
{
	FUNC_STATUS Retval;
	UINT8_T Complete;
	pProtocolCmd pCmd = getProtocolCmd();

	Retval.StatusCode = NoError;
    Retval.PCRCode = NoError;

	if(getProtocolISR()->PreambleReceived == FALSE)
	{
		Retval.ErrorCode = GetPreamble();
		if(Retval.ErrorCode != NoError) 
			return Retval;
	}

	do
	{
        Retval.ErrorCode = GetCommand();
		if(Retval.ErrorCode != NoError) 
			return Retval;
		switch(pCmd->Command)
		{
#if DOWNLOAD_USED
		case GetVersionCmd:
				Retval.ErrorCode = HandleDownloadFlow(address, image_id);
			break;

        case UploadDataHeaderCmd:
            Retval.ErrorCode = HandleUploadFlow(address);
            if(Retval.ErrorCode != NoError)
                return Retval;
            break;
#endif
		case DisconnectCmd:
			//if there was an issue with a download, then tool will try to disconnect
			//handle the disconnect, but return an error since we couldn't do a download
            //getProtocolISR()->CommandReceived = TRUE; // This hack is needed for optimization. Handledisconnect waits for a command again
			Retval.ErrorCode = HandleDisconnect();
			Retval.ErrorCode = DownloadError;
			return Retval;
		default:
			//Retval.ErrorCode = SendError(0);
			//Retval.ErrorCode = UnknownProtocolCmd;
			break;
		}
		if((Retval.ErrorCode != NoError))
			break;

	}while(pCmd->Command != DoneCmd);

	if(pCmd->Command == DoneCmd)
		HandleDoneCmd();
	else{
		Retval.ErrorCode = SendError(0);
		unknown_protocol_command(getProtocolCmd());
		Retval.ErrorCode = UnknownProtocolCmd;
	}

	return Retval;
}

//--------------------------------------------------------
// Utility routines
//--------------------------------------------------------



/*
 *  Verify that the Preamble matches
 */
UINT8_T ProtocolVerifyPreamble( UINT8_T *candidateString )
{
	UINT8_T i;

	for( i=0; i<sizeof(preambleString); i++ )
	{
		if( preambleString[i] != candidateString[i] )
			/* wrong preamble */
			return FALSE;
	}

	return TRUE;
}

/*
 *  Check to see if routine has timed out
 */
UINT8_T CheckProtocolTimeOut(UINT_T StartTime)
{
	UINT_T WaitTime = 0;
	UINT_T CurrentTime;

	CurrentTime = GetOSCR0(); //Dummy read to flush potentially bad data
	CurrentTime = GetOSCR0();

	if (CurrentTime < StartTime)
		CurrentTime = ((0x0 - StartTime) + CurrentTime);
	WaitTime = OSCR0IntervalInSec(StartTime, CurrentTime);  // measure wait time in sec

	if (WaitTime >= PROTOCOL_WAITTIME)
		return (TimeOutError);
	else return NoError;
}
