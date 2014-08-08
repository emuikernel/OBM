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
 *
 *  FILENAME:	Jtag.c
 *
 *  PURPOSE: 	Main JTAG ReEnabling over JTAG code.  Handles all aspects of the jtag
 * 				re-enabling process	over JTAG. Platform independent.
 *                  
********************************************************************************/

//#include "Errors.h"
#include "Typedef.h"
#include "Jtag.h"
#include "misc.h"
#include "Errors.h"		// Platform dependent-only referencing "NoError"
#include "UartDownload.h"	//for UARTMSGBUFFINDEXMAX
#include "wtm_tbr_lib.h"

#if JTAG_PROTOCOL_OVER_JTAG_SUPPORTED

extern UINT_T g_JTAG_CommandPending;
extern UINT_T g_JTAG_SessionActive;
extern UINT_T g_JTAG_UseInterrupts;
extern UINT_T g_JTAG_Initialized;

static int tran_seq;

UINT_T status;
UINT_T PublicKeyBuffer[PUBLIC_KEY_MAX_SIZE_IN_BITS/32];
UINT_T PasswordBuffer[PASSWORD_MAX_SIZE_IN_BITS/32];
UINT_T NonceBuffer[NONCE_MAX_SIZE_IN_BITS/32];

JtagCommunicationProtocolControlData  JTAG_CPCData;
pJtagCommunicationProtocolControlData  pJTAG_CPCData = &JTAG_CPCData;


UINT_T CheckForJTAGCommunicationProtocolTimeout()
 {
 UINT_T Retval = NoError;	
 UINT_T time_waited;

//#if !BOARD_DEBUG
	//check for timeout since last JTAG activity
	if(pJTAG_CPCData->LastActivityTime != 0)
	{
		GetOSCR0(); //initial dummy read
		//check to see how long we've waited: Last Activty Time <-> current time
		time_waited = OSCR0IntervalInMilli(pJTAG_CPCData->LastActivityTime, GetOSCR0());
		//if we have NOT waited long enough, go back to BootRomMain and wait
		if (time_waited >= pJTAG_CPCData->MaxTimeoutValue)
			Retval = JTAG_TIMEOUT_ERROR;
	}
//#endif
   return Retval;
}

void JTAG_DebugKey_InterruptHandler()
{
	UINT_T * DebugKeyBuffer;
	UINT_T command;
	UINT_T CommandControlBitsToValidate;

	#if JTAG_PROTOCOL_DEBUG
	UINT_T UartMsgBuffIndex;
	UINT8_T UartMsgBuff[9];
	UartMsgBuff[8] = '\0';
	#endif
		
	if (g_JTAG_UseInterrupts)
			{
				JTAG_Clear_DebugKey_InterruptRequest();
			}
	// Read the JTAG DebugKey registers
	JTAG_ReceiveCommand();
	
	// Note that the this interrupt handler has been called so JTAG_Check does not call it
	// a second time after it enables interrupts.
	if (!pJTAG_CPCData->InterruptHandlerHasBeenCalled)   
	{
		pJTAG_CPCData->InterruptHandlerHasBeenCalled = 1;
	}
   	// Check for New Data from host
	// If Command Register (DebugKey_1)equals 0 (Initial value) 
	// or the last command sent to the host (in the case of bidirectional DebugKey registers)
	// or the last command received from the host (in the case of unidirectional DebugKey registers)
	// then no change, or assume spurious interrupt, and return.
	if ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] == 0) || 
#if (NUM_JTAG_DEBUGKEY_REGISTERS == 8)	// (8 Registers, Uni-directional)
		//Unidirectional - Check against last data received from host 
		(pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] == pJTAG_CPCData->LastCommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX]))
#else	//Bidirectional	 - Check against last data sent to host
		(pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] == pJTAG_CPCData->LastCommandToHost[JTAG_TRANSACTION_CMDREG_INDEX]))
#endif
    {
		// Spurious interrupt or No NEW Host command
		// Data is zero (no command) 
		// or Data is what we sent to the host (Bidirectional comm registers)
		// or Data is the same as the host previously sent (Unidirectional comm registers)
		// in any of the above, ignore it.
     	return;	 
	}
#if JTAG_PROTOCOL_DEBUG
	AddMessage("Data Read From DebugKey Registers!\0");
	DumpJTAGDebugKeyRegistersToFFUART();
#endif

  	//Extract the Upper 18 bits of the command that was in DebugKey1 register
	command =  pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX];
	// Keep only the MustHave Command bits/fields (Upper 18 bits) for intial validation.
	//command = command & (COMMAND_MASK |TRANSACTION_LENGTH_MASK |HOST_SENDING_MASK |TARGET_SENDING_MASK |DONE_MASK);	
	CommandControlBitsToValidate = command & (COMMAND_MASK | TRANSACTION_LENGTH_MASK | HOST_SENDING_MASK |TARGET_SENDING_MASK |DONE_MASK);
	if (!g_JTAG_SessionActive)
	{
		// No JTAG session active so ONLY look for a START command.
		// Keep only the MustHave Command bits/fields (Upper 18 bits) for intial validation.
		switch (CommandControlBitsToValidate) 
		{
			// Start Command
			case (START_SESSION_CMD_MASK | DONE_MASK | TRANSACTION_LENGTH_32_MASK | HOST_SENDING_MASK):
			{
				JTAG_StartSession_Handler();
				return; // Found a START command - continue, g_JTAG_SessionActive should now be set
			}
			default:
			{
				return; // No Session active and NOT a START command. Ignore.
			}
		}
   	}
   	// Session Active - Look for all commands
    
    // Process commands where initial validation only looks at the upper 18 bits of the Command Register.
	// These are the Command field [31:16] and the Host/Target sending bits[15:14] in the Control field.
	switch (CommandControlBitsToValidate)
	{
		// 0x01: Start Session Command
		case (START_SESSION_CMD_MASK | TRANSACTION_LENGTH_32_MASK | HOST_SENDING_MASK | DONE_MASK):
 		{
			JTAG_StartSession_Handler();
			break;
		}

#if JTAG_REENABLE_SUPPORTED
  		// 0x02: JTAG Request Command
		case (JTAG_REQ_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK | DONE_MASK):
		{
			JTAG_Request_Handler();
			break;
		}
#endif  // #if JTAG_REENABLE_SUPPORTED

		// 0x06: Set Parameter - transaction length field should be zero 
		case (SET_PARAMETER_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK  | DONE_MASK):
		{
			JTAG_SetParameter_Handler();
			break;
		}

		// 0x07: ACK Command
		case (ACK_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK | DONE_MASK):
		{
		 	JTAG_ACK_Handler();
		 	break;
		}

		// 0x08: NACK Command
		case (NACK_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK | DONE_MASK):
		{
			JTAG_NACK_Handler();
		    break;
		}

#if JTAG_REENABLE_SUPPORTED
#if (NUM_JTAG_DEBUGKEY_REGISTERS == 8)	// (8 Registers, Uni-directional)
		// 0x4: Password Command
		//1024  = 224*4+128 (128-Done)
		//2048  = 224*9+32 (32-Done)
		//256=224+32  (R: 32-NotDone, S: 32-Done)
		//521 = 224*2+73 (R: 73-NotDone, S: 73-Done)
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_224_MASK):
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK ):
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_73_MASK ):
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK  | DONE_MASK):
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_73_MASK  | DONE_MASK):
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_128_MASK | DONE_MASK):
#else	// (2 Registers, Bi-directional)
		// 0x04: Password Command
		//521 = 32*16 + 9
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK  ):
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_9_MASK   ):
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_9_MASK   | DONE_MASK):
		case (PASSWORD_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK  | DONE_MASK):
#endif
		{
			JTAG_Password_Handler();
        	break;
		}
#endif  // #if JTAG_REENABLE_SUPPORTED

#if JTAG_REENABLE_SUPPORTED 
#if (NUM_JTAG_DEBUGKEY_REGISTERS == 8)	// (8 Registers, Uni-directional)
		// 0x5: PublicKey Command
		//1024 = 224*4+128 (Exp: 128-NotDone, Mod: 128-Done)
		//2048=224*9+32	(Exp: 32-NotDone, Mod: 32-Done)
		//256=224+32  (X: 32-NotDone, Y: 32-Done)
		//521 = 224*2+73 (X: 73-NotDone, Y: 73-Done)
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_224_MASK):
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK ):
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_73_MASK ):		
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_128_MASK):
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK  | DONE_MASK):
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_73_MASK  | DONE_MASK):		
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_128_MASK | DONE_MASK):					

#else	// (2 Registers, Bi-directional)
		// 0x05: PublicKey Command
		//521 = 32*16 + 9
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK ):
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_9_MASK  ):
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_32_MASK  | DONE_MASK):
		case (PUBLICKEY_CMD_MASK | HOST_SENDING_MASK | TRANSACTION_LENGTH_9_MASK   | DONE_MASK):
#endif
		{
			JTAG_PublicKey_Handler();
            break;
		}
#endif  // #if JTAG_REENABLE_SUPPORTED

		// 0x1F: End Session Command - transaction length field should be zero 
		case (END_SESSION_CMD_MASK | HOST_SENDING_MASK | DONE_MASK):
		{
			JTAG_EndSession_Handler();
			break;
		}

		// Unsupported Command (Not recognized)
		default:
		{
			JTAG_NACK_Manager(UNSUPPORTED_COMMAND_ERROR);
	        break;
		}
	}//End Command Switch
 	return;
}


void JTAG_StartSession_Handler()
{
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_StartSession_Handler");
#endif
	// Initialize Timeout Timer
	UpdateJTAGCommunicationProtocolTimeoutTimer();

	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX]!= 0xA5A5A5A5)
	{
		//JTAG_NACK_Manager(UNSUPPORTED_COMMAND_ERROR);
		JTAG_SendNACK(ABORTING, UNSUPPORTED_COMMAND_ERROR);
		JTAG_AbortSession();
		return;				 
	}
	// Command validated as legitimate
 	if (g_JTAG_SessionActive)
	{
		JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
   	}
 	else
	{
	   	g_JTAG_SessionActive = 1;
		//pJTAG_CPCData->JTAG_ReEnableRequestActive = 0;
		if ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & WAIT_FOR_SET_PARAMETER_MASK) == WAIT_FOR_SET_PARAMETER)
			pJTAG_CPCData->WaitForSetParameterCommmand = 1;
		JTAG_SendACK(NOSTATUS, NOREQUEST);
	}
	return;
}//End JTAG_StartSession_Handler()

void JTAG_Request_Handler()
{
	UINT_T tsr;

#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_Request_Handler");
#endif
	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX]!= JTAGID)
	{
		//JTAG_NACK_Manager(UNSUPPORTED_COMMAND_ERROR);
		JTAG_SendNACK(ABORTING, UNSUPPORTED_COMMAND_ERROR);
		JTAG_AbortSession();
		return; 
	}
	
	// Command legitimate but JTAG permamently disabled or state is Failure Analysis (FA)
	status = JTAG_IsJtagReenableAllowed();
	if (status != NoError)
	{
	 	JTAG_SendACK(status, NOREQUEST);
		return;
	}

	// Command validated as legitimate
	if (!g_JTAG_SessionActive | pJTAG_CPCData->ReEnableRequestActive)
	{
		JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
	}
	else
	{

		// Save any Enabling requests for later (after Key/Password validation)
		// e.g. which cores to enable JTAG on, special debug settings, etc
		pJTAG_CPCData->JTAGRequestData = 
			pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX]& CONTROL_FIELD_MASK;

		//InitPublicKeyDownload();
		pJTAG_CPCData->ReEnableRequestActive = 1;
		pJTAG_CPCData->State = PK_DOWNLOAD_STATE;
		////tran_seq = 1; // start a new transaction for uni-directional case
		JTAG_SendACK(NOSTATUS, NOREQUEST);
	}
	return;
}

void JTAG_SetParameter_Handler()
{
	UINT_T BootState, GPIO_Sel, WDT_Value;	
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_SetParameter_Handler");
#endif
	// Command already validated as legitimate
 	if (!g_JTAG_SessionActive)
	{
		JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
		return;
   	}
	if (!pJTAG_CPCData->WaitForSetParameterCommmand)
   	{
   		JTAG_NACK_Manager(UNEXPECTED_SET_PARAMETER);
   		return;
    }
	// Now do the work
	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX] & SETPARAM_USE_JTAG_INTERRUPTS_MASK)
	{
		JTAG_SetUseJTAGInterrupts();
	}

	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX] & SETPARAM_FORCE_DOWNLOAD_MASK)
	{
		JTAG_SetForceDownload();
	}

	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX] & SETPARAM_BOOTSTATE_ENABLE_MASK)
	{
		BootState = (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX] & SETPARAM_BOOTSTATE_MASK)>> SETPARAM_BOOTSTATE_OFFSET;
		JTAG_SetBootState(BootState);
	}

	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX] & SETPARAM_GPIO_SEL_ENABLE_MASK)
	{
		GPIO_Sel =  (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX] & SETPARAM_GPIO_SEL_MASK) >> SETPARAM_GPIO_SEL_OFFSET;
		JTAG_SetGPIO(GPIO_Sel);
	}

	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX] & SETPARAM_WDT_ENABLE_MASK)
	{
		WDT_Value = (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_FIRST_DATAREG_INDEX] & SETPARAM_WDT_MASK) >> SETPARAM_WDT_OFFSET;
		JTAG_SetWDT (WDT_Value);
   	}

	pJTAG_CPCData->WaitForSetParameterCommmand = 0; // Let the boot continue
	JTAG_SendACK(TARGET_PARAMETERS_SET, NOREQUEST);
}

void JTAG_ACK_Handler()
{
	int i;
	int nonce_size_remaining;
	int tl, num_data_regs;
	int next_buffer_index;
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_ACK_Handler");
#endif
	//
	switch (pJTAG_CPCData->State)
	{
		case (PK_VERIFIED_STATE):
		{
			if ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & ACK_COMMAND_STATUS_MASK) == INITIATOR_REQUEST_GRANTED)
			{
				pJTAG_CPCData->State = NONCE_UPLOAD_STATE;
				////tran_seq = 1; // start a new transaction for uni-directional case
				JTAG_NACK_Manager(BUSY);
				g_JTAG_CommandPending = 1; // Enable the deferred handler to be called from JTAG_Check()
	 			return;
			}
			if ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & ACK_COMMAND_STATUS_MASK) == INITIATOR_REQUEST_DENIED)
			{
				JTAG_SendNACK(ABORTING, UNEXPECTED_COMMAND_ERROR);
				JTAG_AbortSession();
				return;
			}
			break;
		}
		case(NONCE_UPLOAD_STATE):
		{
			if (pJTAG_CPCData->NonceSizeInBits <= pJTAG_CPCData->NonceBitsSent) // if we have sent enough bits
			{
		   		if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & INITIATOR_REQUEST_MASK)
				{
					// Initiator Request set 
					pJTAG_CPCData->State = PW_DOWNLOAD_STATE;
					////tran_seq = 1; // start a new transaction for uni-directional case
					pJTAG_CPCData->NonceBitsSent = 0;
					JTAG_SendACK(INITIATOR_REQUEST_GRANTED, NOREQUEST);
			   	}
				else
				{
					// Initiator Request not set after all nonce bits uploaded
					JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
					return;
 				}
		   	} 
			else
			{	
				if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & INITIATOR_REQUEST_MASK)
				{
					// Initiator Request set, but we are done sending the nonce
					JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
					return;
			   	}

				//Calculate nonce bits remaining to upload
				nonce_size_remaining = pJTAG_CPCData->NonceSizeInBits - pJTAG_CPCData->NonceBitsSent;
				// Send remainder of Nonce to host
				// Build nonce Command to send to host
				InitializeCommandToHostBuffer();
				// The previous nonce_cmd is succesfully ACKed, send the next transaction
				tran_seq++;	//No need to worry about wrap around after F
				pJTAG_CPCData->CommandToHost[0]  = (NONCE_CMD_MASK | pJTAG_CPCData->NonceSizeInBits | TARGET_SENDING_MASK);
				pJTAG_CPCData->CommandToHost[0] |= ((tran_seq << TRANSACTION_SEQUENCE_OFFSET) & TRANSACTION_SEQUENCE_MASK);

			   	// Calc next nonce buffer index to send
			   	next_buffer_index = (pJTAG_CPCData->NonceBitsSent/32); 
				num_data_regs = NUM_JTAG_DEBUGKEY_REGISTERS-1;
				// tl = transaction length, nonce_size_remaining will now get decremented
			   	tl = 0;
			   	for (i=0; i<num_data_regs; i++)
				{
					if(nonce_size_remaining > 0) 
					{
						pJTAG_CPCData->CommandToHost[i+1]= NonceBuffer[next_buffer_index+i];
						nonce_size_remaining -=32;
						tl +=32;
						pJTAG_CPCData->NonceBitsSent +=32;
					}
				}
				// Update command with transaction length
				pJTAG_CPCData->CommandToHost[0] |= ((tl << TRANSACTION_LENGTH_OFFSET)& TRANSACTION_LENGTH_MASK);
				// set the done bit
				if (nonce_size_remaining == 0)
				{
					pJTAG_CPCData->CommandToHost[0] |= DONE_MASK;	 
				} 
				JTAG_SendCommand(&pJTAG_CPCData->CommandToHost[0]);	// Send the command to host
	   		}
		default:
			{
				JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
				return;
			}
		}
	} 
}

void JTAG_NACK_Handler()
{
	UINT_T command_status;
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_NACK_Handler");
#endif
	if (pJTAG_CPCData->State != NONCE_UPLOAD_STATE)
	{
		JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
		return;
	}
    // State is NONCE_UPLOAD_STATE
	command_status = (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & NACK_COMMAND_STATUS_MASK)>> NACK_COMMAND_STATUS_OFFSET;
	switch(command_status)
	{
		case RETRY_TRANSACTION:
			{
				JTAG_SendCommand(&pJTAG_CPCData->LastCommandToHost[0]);
				break;
			}
		case RETRY_COMMAND_SEQUENCE:
			{
				// State is already NONCE_UPLOAD_STATE, set Command Pending and let the 
				// SendNonceDeferredHandler deal with resending a nonce
				JTAG_NACK_Manager(BUSY);
				g_JTAG_CommandPending = 1;
				break; 
 			}
		case ABORTING:
			{
			 JTAG_AbortSession();
			 break;
			}
		default:
			{
			 JTAG_AbortSession();
			 break;
			}
	}//End Switch
	return;
}


/*	case ((UNIPUBLICKEY_CMD << COMMAND_OFFSET) | HOST_SENDING_MASK  | (JTAGID_SHA160 << HASH_OFFSET) | 
			 JTAGID_PKCS1024 |(TRANSACTION_LENGTH_96 << TRANSACTION_LENGTH_OFFSET)):
  	case ((UNIPUBLICKEY_CMD << COMMAND_OFFSET) | HOST_SENDING_MASK  | (JTAGID_SHA160 << HASH_OFFSET) |
			 JTAGID_PKCS1024 |(TRANSACTION_LENGTH_64 << TRANSACTION_LENGTH_OFFSET) | DONE_MASK):
*/
void JTAG_PublicKey_Handler()
{
	UINT_T i, KeyId, HashId, UseCryptoSchemeIDInHash, bitsToRead, registersToRead, status;
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_PublicKey_Handler");
#endif
	// 
	if (pJTAG_CPCData->State != PK_DOWNLOAD_STATE)
	{
		JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
		return;
	}
	// Check Command Key and Hash Algorithm, and whether Crypto_Scheme is used
	KeyId = ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & KEY_MASK) >> KEY_OFFSET);
	HashId = ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & HASH_MASK) >> HASH_OFFSET);
	UseCryptoSchemeIDInHash = ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & USE_SCHEME_ID_IN_HASH_MASK) >> USE_SCHEME_ID_IN_HASH_OFFSET);
	if (pJTAG_CPCData->PublicKeyBitsReceived == 0)
	{
		// First transaction - Save KeyId, HashId and UseCryptoSchemeIDInHash
		pJTAG_CPCData->PublicKeyAlgorithm = KeyId;
		pJTAG_CPCData->PublicKeyHashAlgorithm = HashId;
		pJTAG_CPCData->PublicKeySchemeIDInHash = UseCryptoSchemeIDInHash;

		// Check if the key and hash are supported on this platform
		status = JTAG_IsHashSupported(HashId);
		if (status != NoError)
			{
			JTAG_NACK_Manager(UNSUPPORTED_COMMAND_ERROR);
			return;
			}
		status = JTAG_IsKeySupported(KeyId);
		if (status != NoError)
			{
			JTAG_NACK_Manager(UNSUPPORTED_COMMAND_ERROR);
			return;
			}

		// Find the number of bits we expect to download
		switch(KeyId)
		{
			case(JTAGID_PKCS1024):
				pJTAG_CPCData->PublicKeySizeInBits = PKCS1024_LENGTH_IN_BITS * 2; // Exponent and Modulus
				pJTAG_CPCData->PasswordSizeInBits =  PKCS1024_LENGTH_IN_BITS;
				break;
			case(JTAGID_PKCS2048):
				pJTAG_CPCData->PublicKeySizeInBits = PKCS2048_LENGTH_IN_BITS * 2; // Exponent and Modulus
				pJTAG_CPCData->PasswordSizeInBits = PKCS2048_LENGTH_IN_BITS;
				break;
			case(JTAGID_ECDSA256):
				pJTAG_CPCData->PublicKeySizeInBits = ECDSA256_LENGTH_IN_BITS * 2; // X and Y
				pJTAG_CPCData->PasswordSizeInBits  = ECDSA256_LENGTH_IN_BITS * 2; // R and S
				break;
			case(JTAGID_ECDSA521):
				pJTAG_CPCData->PublicKeySizeInBits = ECDSA521_LENGTH_IN_BITS * 2; // X and Y
				pJTAG_CPCData->PasswordSizeInBits  = ECDSA521_LENGTH_IN_BITS * 2; // R and S
				break;
			default:
				JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
				return;
		}
	}//Endif PublicKeyBitsReceived == 0 
	else
	{
		// nth transaction - check that HashId and KeyID and UseCryptoSchemeIDInHash match those in the first transaction
		if ((pJTAG_CPCData->PublicKeyAlgorithm != KeyId) | 
			(pJTAG_CPCData->PublicKeyHashAlgorithm != HashId) |
			(pJTAG_CPCData->PublicKeySchemeIDInHash != UseCryptoSchemeIDInHash) )
		{
			JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
			return;
		}			 
	}//Endif PublicKeyBitsReceived != 0

	//Get the transaction length in bits
	bitsToRead = (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & TRANSACTION_LENGTH_MASK) >> TRANSACTION_LENGTH_OFFSET;
   
	// How many registers to read (registersToRead)?
	switch (bitsToRead)
	{
#if (NUM_JTAG_DEBUGKEY_REGISTERS == 8) // (8 Registers, Uni-directional)
		case(224): {registersToRead = 7; break;}
		case(128): {registersToRead = 4; break;}
		case (73): {registersToRead = 3; break;}		 
		case (32): {registersToRead = 1; break;}
#else	// (2 Registers, Bi-directional)
		case(32): {registersToRead = 1; break;}
		case(9):  {registersToRead = 1; break;}
#endif //NUM_JTAG_DEBUGKEY_REGISTERS

		default:
		{
			JTAG_NACK_Manager(UNSUPPORTED_COMMAND_ERROR);
			return;
		}
	 }


	for (i=1; i<=registersToRead; i++)
	{
	*pJTAG_CPCData->pPublicKey = pJTAG_CPCData->CommandFromHost[i];
	pJTAG_CPCData->pPublicKey++;
	}

	// Update bits read
	pJTAG_CPCData->PublicKeyBitsReceived += bitsToRead;
	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & DONE_MASK)
	{
		if (pJTAG_CPCData->PublicKeyBitsReceived == pJTAG_CPCData->PublicKeySizeInBits)
		{
			pJTAG_CPCData->PublicKeyReceived = 1;
			JTAG_NACK_Manager(BUSY);
			g_JTAG_CommandPending = 1; // Enable the deferred handler to be called from JTAG_Check()
 		}
		else
		{
			JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
			return;
		} 
	}
	else
	{ 
	 	if (pJTAG_CPCData->PublicKeyBitsReceived < pJTAG_CPCData->PublicKeySizeInBits)
		{
			JTAG_SendACK(NOSTATUS, NOREQUEST);
			return;
 		}
		else
		{
		 	JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
			return;
		} 
	 
	}
}

void JTAG_Password_Handler()
{	
	UINT_T i, keyId, HashId, bitsToRead, registersToRead, status;
	UINT_T * pBuffer;
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_Password_Handler");
#endif
	// 
	if (pJTAG_CPCData->State != PW_DOWNLOAD_STATE)
	{
		JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
		return;
	}

	//Get the transaction length in bits
	bitsToRead = ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & TRANSACTION_LENGTH_MASK) >> TRANSACTION_LENGTH_OFFSET);

	// How many registers to read (registersToRead)?
	switch (bitsToRead)
	{
#if (NUM_JTAG_DEBUGKEY_REGISTERS == 8) // (8 Registers, Uni-directional)
		case(224): {registersToRead = 7; break;}
		case(128): {registersToRead = 4; break;}
		case (73): {registersToRead = 3; break;}		 
		case (32): {registersToRead = 1; break;}
#else	// (2 Registers, Bi-directional)
		case(32): {registersToRead = 1; break;}
		case(9):  {registersToRead = 1; break;}
#endif //NUM_JTAG_DEBUGKEY_REGISTERS

		default:
		{
			JTAG_NACK_Manager(UNSUPPORTED_COMMAND_ERROR);
			return;
		}
	 }


	for (i=1; i<=registersToRead; i++)
	{
	*pJTAG_CPCData->pPassword = pJTAG_CPCData->CommandFromHost[i];
	pJTAG_CPCData->pPassword++;
	}

	// Update bits read
	pJTAG_CPCData->PasswordBitsReceived += bitsToRead;
	if (pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX] & DONE_MASK)
	{
		if (pJTAG_CPCData->PasswordBitsReceived == pJTAG_CPCData->PasswordSizeInBits)
		{
			pJTAG_CPCData->PasswordReceived = 1;
			JTAG_NACK_Manager(BUSY);
			g_JTAG_CommandPending = 1; // Enable the deferred handler to be called from JTAG_Check()
 		}
		else
		{
		 JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
		 return;
		} 
	}
	else
	{ 
	 	if (pJTAG_CPCData->PasswordBitsReceived < pJTAG_CPCData->PasswordSizeInBits)
		{
			JTAG_SendACK(NOSTATUS, NOREQUEST);
 		}
		else
		{
		 JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
		} 
	 
	}
}

void JTAG_EndSession_Handler()
{
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_EndSession_Handler");
#endif
	// Command already validated as legitimate
 	if ( !g_JTAG_SessionActive)
	{
		JTAG_NACK_Manager(UNEXPECTED_COMMAND_ERROR);
   	}
 	else
	{
	   	g_JTAG_SessionActive = 0;
		JTAG_SendACK(NOSTATUS, NOREQUEST);
	}
}

void JTAG_SendCommand(UINT_T * ptrToCommand)
{
	int i;
	// Send the command to host
	JTAG_WriteDebugKeys(ptrToCommand);
#if JTAG_PROTOCOL_DEBUG
	AddMessage("Data Written To MESSAGE_FROM_SP_TO_JTAG Registers!\0");
	DumpJTAG_SPToJTAGMessageRegisters_ToFFUART(ptrToCommand);
#endif
	
	// Save the command for future reference
	for (i=0; i<NUM_JTAG_DEBUGKEY_REGISTERS; i++)
	{
	 	pJTAG_CPCData->LastCommandToHost[i]= ptrToCommand[i];
	}
	return;	
}

void JTAG_ReceiveCommand()
{
	int i;
	
	// Save the command for future reference
	for (i=0; i<NUM_JTAG_DEBUGKEY_REGISTERS; i++)
	{
	 	pJTAG_CPCData->LastCommandFromHost[i]= pJTAG_CPCData->CommandFromHost[i];
	}

	// Receive the command from host
	JTAG_ReadDebugKeys(pJTAG_CPCData->CommandFromHost);
	
	return;	
}

/*
void JTAG_ACK_Manager(UINT_T command_status)
{
 UINT_T Retval = NoError;	
	
	// TBD
	//
	JTAG_SendACK(UINT_T command_status, UINT_T InitiatorRequest)
 
  return;
}
*/

void JTAG_NACK_Manager(UINT_T ErrorCode)
{
	UINT_T Retval = NoError;	
	
	//if Errorcode == BUSY just send NACK with:
	// Command Status [9:6] = 0xF (busy)
	// Error Code [13:10] = 0 (no status)
	// Target Sending [14] = 1  
	//
	UpdateJTAGCommunicationProtocolTimeoutTimer();

	if (ErrorCode == BUSY)
	{
		JTAG_SendNACK(BUSY, NOERROR);
		return;
	}
	if (ErrorCode == UNEXPECTED_SET_PARAMETER)
	{
		JTAG_SendNACK(ABORTING, UNEXPECTED_SET_PARAMETER);
		JTAG_AbortSession();
		return;
	}

	if (pJTAG_CPCData->TransactionRetryCount >= MAX_TRANSACTION_RETRY_COUNT)
	{
		// Transaction retry execeeded, see if we can retry the transaction
		pJTAG_CPCData->TransactionRetryCount = 0;
		// If command sequence active
		if ((pJTAG_CPCData->State == PK_DOWNLOAD_STATE) ||
			(pJTAG_CPCData->State == PW_DOWNLOAD_STATE) ||
			(pJTAG_CPCData->State == NONCE_UPLOAD_STATE))
		{
			if (pJTAG_CPCData->CommandSequenceRetryCount >=  MAX_COMMAND_SEQUENCE_RETRY_COUNT)
			{
				//pJTAG_CPCData->CommandSequenceRetryCount = 0;
   				JTAG_SendNACK(ABORTING, ErrorCode);
				JTAG_AbortSession();
			}
			else
			{
				pJTAG_CPCData->CommandSequenceRetryCount++;
				switch (pJTAG_CPCData->State)
				{
					case (PK_DOWNLOAD_STATE):
						// Initialize public key download parameters
						InitializePublicKeyDownloadParameters();
						break;
				  	case (PW_DOWNLOAD_STATE):
						// Initialize password download parameters
						InitializePasswordDownloadParameters();
						break;
					case (NONCE_UPLOAD_STATE):
						// Initialize nonce upload parameters
						InitializeNonceUploadParameters();
						break;
				 }//End Switch
				 JTAG_SendNACK(RETRY_COMMAND_SEQUENCE, ErrorCode );
 			}
			
		}//End Command sequence active
		else
		{
			//Tranaction retry count exceeded. 
			// This is a single transaction cmd sequence so abort.
			JTAG_SendNACK(ABORTING, ErrorCode);
			JTAG_AbortSession();
		}		     
	}	
	return;
}


void JTAG_SendACK(UINT_T command_status, UINT_T InitiatorRequest)
{
	int i;
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_SendACK");
#endif
	UpdateJTAGCommunicationProtocolTimeoutTimer();
	// Build a command to host
	InitializeCommandToHostBuffer();

	pJTAG_CPCData->CommandToHost[0]= ( ACK_CMD_MASK | DONE_MASK | TARGET_SENDING_MASK | 
									   (command_status << ACK_COMMAND_STATUS_OFFSET) );
	pJTAG_CPCData->CommandToHost[0] |= TRANSACTION_LENGTH_32_MASK;


	if (InitiatorRequest)
	{
	   	pJTAG_CPCData->CommandToHost[0] |= INITIATOR_REQUEST_MASK;
#if JTAG_PROTOCOL_DEBUG
	AddMessage("InitiatorRequest");
#endif
	}
	pJTAG_CPCData->CommandToHost[1]= pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX];	// Copy command being ACKed

	//if Start_Session is being ACKed, add version info
	if ( ((pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX]) & COMMAND_MASK) == START_SESSION_CMD_MASK )
	{
		pJTAG_CPCData->CommandToHost[1] |= ((PROTOCOL_VERSION << PROTOCOL_VERSION_OFFSET) & PROTOCOL_VERSION_MASK);
	}

	JTAG_SendCommand(&pJTAG_CPCData->CommandToHost[0]);	// Send the command to host
	return;
}

/**************************************************************
*  JTAG_SendNACK
* 
*  Description: Builds a NACK command and sends to host
*
***************************************************************/
void JTAG_SendNACK(UINT_T command_status, UINT_T error)
{
   	int i;
#if JTAG_PROTOCOL_DEBUG
	AddMessage("JTAG_SendNACK");
#endif
   	// Build a command to host
	InitializeCommandToHostBuffer();
	pJTAG_CPCData->CommandToHost[0]= ( NACK_CMD_MASK | DONE_MASK | TARGET_SENDING_MASK | 
									  (command_status<<NACK_COMMAND_STATUS_OFFSET) | (error<< NACK_ERROR_CODE_OFFSET) );
	pJTAG_CPCData->CommandToHost[1]= pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX];	// Copy command being ACKed
#if (JTAG_FOUR_DEBUGKEY_REGISTERS)
	pJTAG_CPCData->CommandToHost[2]= 0;
	pJTAG_CPCData->CommandToHost[3]= 0;
#endif

	JTAG_SendCommand(&pJTAG_CPCData->CommandToHost[0]);	// Send the command to host
	return;
}

void JTAG_AbortSession()
{
 	UINT_T Retval = NoError;	
   	// Clear all data so we don't get hung up in any loops and continue boot.
	//Clear WaitForSetParameterCommmand
	pJTAG_CPCData->WaitForSetParameterCommmand = 0;
 	// Clear global flags defined in this module
 	g_JTAG_CommandPending = 0;
	g_JTAG_SessionActive = 0;
	g_JTAG_UseInterrupts = 0;
	return;
}

void InitializePublicKeyDownloadParameters()
{
	int i;
	UINT_T buffersize = PUBLIC_KEY_MAX_SIZE_IN_BITS/32;

	pJTAG_CPCData->pPublicKey = &PublicKeyBuffer[0];
	pJTAG_CPCData->PublicKeyBitsReceived = 0;
	pJTAG_CPCData->PublicKeySizeInBits = 0;
	pJTAG_CPCData->PublicKeyCmdSeqActive = 0;
	pJTAG_CPCData->PublicKeyReceived = 0;
	pJTAG_CPCData->PublicKeyVerified = 0;
	pJTAG_CPCData->PublicKeyFirstCommand = 0;
	pJTAG_CPCData->PublicKeyAlgorithm = 0;
	pJTAG_CPCData->PublicKeyHashAlgorithm = 0;
	pJTAG_CPCData->PublicKeySchemeIDInHash = 0;
	for (i=0; i<buffersize; i++)
	{
		PublicKeyBuffer[i]=0;
	} 
	return;
}

void InitializePasswordDownloadParameters()
{
	int i;
	UINT_T buffersize = PASSWORD_MAX_SIZE_IN_BITS/32;

	pJTAG_CPCData->pPassword = &PasswordBuffer[0];
	pJTAG_CPCData->PasswordBitsReceived = 0;
	pJTAG_CPCData->PasswordSizeInBits = 0;
	pJTAG_CPCData->PasswordCmdSeqActive = 0;
	pJTAG_CPCData->PasswordReceived = 0;
	pJTAG_CPCData->PasswordVerified = 0;
	pJTAG_CPCData->PasswordFirstCommand = 0;
	for (i=0; i<buffersize; i++)
	{
		PasswordBuffer[i]=0;
	}
	return;
}

void InitializeNonceUploadParameters()
{
	int i;
	UINT_T buffersize = NONCE_MAX_SIZE_IN_BITS/32;

	pJTAG_CPCData->pNonce = &NonceBuffer[0];
	pJTAG_CPCData->NonceBitsSent = 0;
	pJTAG_CPCData->NonceSizeInBits = 0;
	pJTAG_CPCData->NonceCmdSeqActive = 0;
	pJTAG_CPCData->NonceSent = 0;
	for (i=0; i<buffersize; i++)
	{
		NonceBuffer[i]=0;
	} 

	return;
}

UINT_T VerifyPublicKeyDeferredHandler()
{
 	UINT_T Retval = NoError;	
	UINT_T status;

	g_JTAG_CommandPending = 0; 
	status = JTAG_CalculateHashOfPublicKey(&PublicKeyBuffer[0], 
										pJTAG_CPCData->PublicKeyAlgorithm, 
										pJTAG_CPCData->PublicKeyHashAlgorithm,
										pJTAG_CPCData->PublicKeySchemeIDInHash);
	if (status != NoError)
	{
	   	// Force a retry of the full command sequence
		pJTAG_CPCData->TransactionRetryCount = MAX_TRANSACTION_RETRY_COUNT;
		JTAG_NACK_Manager(BAD_VALUE);
	}
	else
	{
#if JTAG_PROTOCOL_DEBUG
		AddMessage("PublicKeyVerified !");
#endif
		pJTAG_CPCData->PublicKeyVerified = 1;
		pJTAG_CPCData->State = PK_VERIFIED_STATE;
		JTAG_SendACK(PUBLICKEY_VERIFIED, INITIATOR_REQUEST_FLAG);
	}
	return Retval;
}

UINT_T VerifyPasswordDeferredHandler()
{
	UINT_T Retval = NoError;	
	UINT_T status;

#if JTAG_PROTOCOL_DEBUG
		AddMessage("JTAG VerifyPasswordDeferredHandler");
#endif

	g_JTAG_CommandPending = 0; 
	status = JTAG_VerifyPassword(&PublicKeyBuffer[0], 
								  pJTAG_CPCData->PublicKeyAlgorithm,
								  pJTAG_CPCData->PublicKeyHashAlgorithm,	
								  &PasswordBuffer[0], 
								  &NonceBuffer[0], 
								  pJTAG_CPCData->NonceSizeInBits);
	if (status != NoError)
	{
		// Force a retry of the full command sequence
		pJTAG_CPCData->TransactionRetryCount = MAX_TRANSACTION_RETRY_COUNT;
#if JTAG_PROTOCOL_DEBUG
		AddMessage("JTAG Password Verify FAILED!");
#endif
		JTAG_NACK_Manager(BAD_VALUE);
	}
	else
	{
#if JTAG_PROTOCOL_DEBUG
		AddMessage("JTAG Password Verified!");
#endif
		pJTAG_CPCData->PasswordVerified = 1;
		pJTAG_CPCData->State = PW_VERIFIED_STATE;
		JTAG_EnableJTAG(pJTAG_CPCData->JTAGRequestData);
		JTAG_SendACK(JTAG_ENABLED, NOREQUEST);
	} 
	return Retval;
}

// This is called from JTAG_Check() in BootromMain.
UINT_T SendNonceDeferredHandler()
{
	UINT_T Retval = NoError;	
	int i, nonce_size;
	UINT_T tl, num_data_regs;
#if JTAG_PROTOCOL_DEBUG
		AddMessage("SendNonceDeferredHandler");
#endif	
	g_JTAG_CommandPending = 0;
	InitializeNonceUploadParameters(); 
    nonce_size = JTAG_GetNonceSize();
	nonce_size &= NONCE_DATASIZE_MASK;
	
	pJTAG_CPCData->NonceSizeInBits = nonce_size;
    JTAG_GetNonce(&NonceBuffer[0], nonce_size);


#if JTAG_PROTOCOL_DEBUG
    // Use a constant nonce that we know, for debug purposes
	/*
	if ( nonce_size == 256 )
	{
		NonceBuffer[7]= 0x74C0E0C4;
		NonceBuffer[6]= 0x5569d3ff;
		NonceBuffer[5]= 0xbe6192fe;
		NonceBuffer[4]= 0x3b244ece;
		NonceBuffer[3]= 0x81d62aaa;
		NonceBuffer[2]= 0x5e992bdd;
		NonceBuffer[1]= 0x1403cf2d;
		NonceBuffer[0]= 0x7d2a6875;
	}
	*/
#endif

	// Build nonce Command to send to host
				pJTAG_CPCData->CommandToHost[0]  = (NONCE_CMD_MASK | nonce_size | TARGET_SENDING_MASK);
				pJTAG_CPCData->CommandToHost[0] |= ((tran_seq << TRANSACTION_SEQUENCE_OFFSET) & TRANSACTION_SEQUENCE_MASK);
	// Determine if this is a single or multiple transaction and set the DONE bit accordingly.
	// Nonce size is in bits
   	num_data_regs = NUM_JTAG_DEBUGKEY_REGISTERS-1; 
	// tl = transaction length, nonce_size will now get decremented
   	tl = 0;
   	for (i=0; i<num_data_regs; i++)
	{
		if(nonce_size > 0) 
		{
			pJTAG_CPCData->CommandToHost[i+1]= NonceBuffer[i];
			nonce_size -=32;
			tl +=32;
			pJTAG_CPCData->NonceBitsSent +=32;
		}
	}
	// Update command with transaction length
	pJTAG_CPCData->CommandToHost[0] |= ((tl << TRANSACTION_LENGTH_OFFSET) & TRANSACTION_LENGTH_MASK);
	// If this is a single transcation
	if (nonce_size == 0)
	{
		pJTAG_CPCData->CommandToHost[0] |= DONE_MASK;	 
	} 
	JTAG_SendCommand(&pJTAG_CPCData->CommandToHost[0]);	// Send the command to host
		return Retval;
}

void InitializeJtagProtocolOverJtag()
{
 	UINT_T Retval = NoError;	
 	// Initialize the JtagCommunicationProtocolControlData structure
	memset(pJTAG_CPCData, 0, sizeof(JtagCommunicationProtocolControlData));
	// Initialize CPC Data
	// Set max timeout value
	pJTAG_CPCData->MaxTimeoutValue = JTAG_PROTOCOLTIMEOUTVALUE;
   	// Initialize public key download parameters
	InitializePublicKeyDownloadParameters();
	// Initialize password download parameters
	InitializePasswordDownloadParameters();
	// Initialize nonce upload parameters
	InitializeNonceUploadParameters();

	JTAG_Initialize_DebugKey_Access();

	g_JTAG_Initialized = 1;
	return;
}

void InitializeCommandToHostBuffer()
{
	int i;
	for (i=0; i<NUM_JTAG_DEBUGKEY_REGISTERS; i++)
	{
		pJTAG_CPCData->CommandToHost[i]= 0;
	}
	return;
}

void WaitForJtagStartCommand()
{
#if JTAG_PROTOCOL_DEBUG
	UINT_T command;
	UINT_T CommandControlBitsToValidate;
	UINT_T i =0 ;
	UINT8_T UartMsgBuff[UARTMSGBUFFINDEXMAX+1];
	UINT_T UartMsgBuffIndex = 0;
	UartMsgBuff[8] = '\0';

	UartMsgBuff[0]= 0xD; // CR
	UartMsgBuff[1]= 0x0;
	WriteStringToFFUart(UartMsgBuff);
	AddMessage("JTAG Protocol Debug Enabled!\0");
	// Initialize the JtagCommunicationProtocolControlData structure
	memset(pJTAG_CPCData, 0, sizeof(JtagCommunicationProtocolControlData));
	while(1)
	{
		// Read the JTAG DebugKey registers
		JTAG_ReceiveCommand();
		//Extract the Upper 18 bits of the command that was in DebugKey1 register
		command =  pJTAG_CPCData->CommandFromHost[JTAG_TRANSACTION_CMDREG_INDEX];
		// Keep only the MustHave Command bits/fields (Upper 5 bits) for intial validation.
	   	//CommandControlBitsToValidate = command & COMMAND_MASK ;
		// Wait until start command found
		//if ( CommandControlBitsToValidate != START_SESSION_CMD_MASK)
		if (command == 0)
		{
			WriteAliveCharToFFUart(0x62,&UartMsgBuffIndex); //"b"
		}
		else
		{

			break; // Start command found (or some non-zero command found)
		}
	}//Endwhile
#endif
	return;
}

void SimulateXFER_State()
{
#if JTAG_PROTOCOL_DEBUG
	static UINT_T UartMsgBuffIndex = 0;

	if ((!g_JTAG_CommandPending) && (!g_JTAG_SessionActive))
	{
		UartMsgBuffIndex = 0;
		AddMessage("Exiting State Machine Loop!\0");
		while (1)
		{
		   	WriteAliveCharToFFUart(0x78,&UartMsgBuffIndex);	//"x"
		}//Endwhile
	}
	else
	{
	 	WriteAliveCharToFFUart(0x2E,&UartMsgBuffIndex);	//"."
	}
#endif	
	return;
}

void DumpJTAGDebugKeyRegistersToFFUART()
{
#if JTAG_PROTOCOL_DEBUG
	UINT8_T UartMsgBuff[9];
	UartMsgBuff[8] = '\0';
	#if (NUM_JTAG_DEBUGKEY_REGISTERS == 8) // (8 Registers, Uni-directional)
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[7], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R7 - Data 6)\0");
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[6], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R6 - Data 5)\0");
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[5], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R5 - Data 4)\0");
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[4], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R4 - Data 3)\0");
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[3], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R3 - Data 2)\0");
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[2], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R2 - Data 1)\0");
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[1], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R1 - Data 0)\0");
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[0], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R0 - cmd/ctrl)\r\n--------------------------\0");
	#else	// (2 Registers, Bi-directional)
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[1], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R1 - Data 0)\0");
		ConvertIntToBuf8(UartMsgBuff,pJTAG_CPCData->CommandFromHost[0], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R0 - cmd/ctrl)\r\n--------------------------\0");
	#endif
#endif
	return;
}



void DumpJTAG_SPToJTAGMessageRegisters_ToFFUART(UINT_T * ptrToCommand)
{
#if JTAG_PROTOCOL_DEBUG
	UINT8_T UartMsgBuff[9];
	UartMsgBuff[8] = '\0';
	#if (NUM_JTAG_DEBUGKEY_REGISTERS == 8) // (8 Registers, Uni-directional)
		
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[7], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R7 - Data 6)\0");
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[6], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R6 - Data 5)\0");
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[5], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R5 - Data 4)\0");
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[4], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R4 - Data 3)\0");
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[3], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R3 - Data 2)\0");
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[2], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R2 - Data 1)\0");
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[1], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R1 - Data 0)\0");
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[0], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R0 - cmd/ctrl)\r\n--------------------------\0");
	#else	// (2 Registers, Bi-directional)
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[1], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R1 - Data 0)\0");
		ConvertIntToBuf8(UartMsgBuff,ptrToCommand[0], 8,0x10);
		WriteStringToFFUart("0x\0");
		WriteStringToFFUart(UartMsgBuff);
		AddMessage(" (R0 - cmd/ctrl)\r\n--------------------------\0");
	#endif
#endif
	return;
}

#endif //JTAG_PROTOCOL_OVER_JTAG_SUPPORTED

