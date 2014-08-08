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
**
**  FILENAME:
**
**  PURPOSE:
**      ChipIdea 1136-a UBSB 2.0 OTG Client Device Controller transfer routines
**
**
**  LAST MODIFIED:
**
******************************************************************************/

#include "xllp_defs.h"
#include "misc.h"      // memcpy
#include "xllp_ciu2d.h"
#include "ProtocolManager.h"
#include "Interrupts.h"
#include "loadoffsets.h"

#if USBCI

extern void EnableIrqInterrupts();
extern void DisableIrqInterrupts();


extern const  XLLP_U2D_USB_EP_T  usbTransmitEp = U2D_USB_EP_1;
extern const  XLLP_U2D_USB_EP_T  usbReceiveEp  = U2D_USB_EP_2;
extern const XLLP_U2D_EP_CONFIG_TABLE_T defaultU2DEndpointConfigTable[];

#if CI2_USB_DDR_BUF
#define BUFFER DDR_CI2_USB_DDR_BUF_ADDR
#define MAX_RECV_FIFO_LEN      (8192)
UINT8 *receiveBuff = (UINT8 *)BUFFER;
UINT8 *transmitBuff = (UINT8 *)BUFFER + MAX_RECV_FIFO_LEN;
#else
#define MAX_RECV_FIFO_LEN      (1024)
UINT8 receiveBuff[MAX_RECV_FIFO_LEN];
#endif
//needed for library support.  See xllp_ciu2d.c.
UINT8 * getCI2ReceiveBuff( void){
	 return receiveBuff;
}

//----------------------------------------------------------------------------
//  CI2Transmit
//
//
//
//----------------------------------------------------------------------------
XLLP_UINT32_T CI2Transmit( XLLP_UINT32_T TxDataLength,
                           XLLP_UINT8_T  *pTxBuff,
                           XLLP_UINT8_T  SyncTransfer      )
{
    UINT8_T Retval;
    XLLP_INT32_T actualRxCount;

    Retval = xllpCI2Xfer( usbTransmitEp,          // Endpoint
                            U2D_IN,
                            pTxBuff,                 // Memory buffer
                            TxDataLength,           // Buffer Size
                            &actualRxCount          // Not used on transmit...
                          );

    return (Retval);
}

//----------------------------------------------------------------------------
//  CI2Receive
//
//
//
//----------------------------------------------------------------------------
XLLP_UINT32_T CI2Receive( XLLP_UINT32_T RxDataLength, XLLP_UINT8_T  *pRxBuff, XLLP_INT32_T  *byteCount )
{
    UINT8_T Retval;

    Retval = xllpCI2Xfer( usbReceiveEp,        // Endpoint
                            U2D_OUT,
                            pRxBuff,             // Memory buffer
                            RxDataLength,       // Buffer Size
							byteCount           // # bytes transferred by U2D
                          );


    return (Retval);
}

//--------------------------------------------------------------------------
// CI2GetMsg -> Get bytes/packet etc. from U2D
//              This executes in Interrupt execution context chain
//
// This code has changed from the U2D code, since we don't get
// "short packet" interrupts, therefore more work to keep in line with
// protocol state.
//
//
//typedef void (*USB_SERVICE_FUNC)(void* handle, uint_8, boolean, uint_8, uint_8_ptr, uint_32, uint_8);
//--------------------------------------------------------------------------
static UINT cmd_in_progress = 0;
static UINT fast_download = 0;

#define USB_SEND 1	  // equivalent to MV_SEND_USB in chip idea headers

void CI2GetMsg( void *handle, UINT8 ep_num, UINT32_T setup, UINT8 direction,
                        UINT8 *buff_start_address, UINT32 actual_transfer_length, UINT8 errors)
{
  pProtocolCmd pCmd;
  pProtocolISR pISR;
  XLLP_INT32_T byteCount = 0, packetLength = 0, size;
  XLLP_UINT32_T bit_pos, dummy[2];
  UINT8_T Buff[PREAMBLE_SIZE], *buffPtr;
  //Chip Idea peripheral will transfer minimum of packet size per DTD when host sends
  // more than packet size worth of data. This is important when handling the data commands
  // and trying to strip out the command from the data.  When host sends less than a
  // packet size worth of data then the transfer completes as expected.
  size = defaultU2DEndpointConfigTable[U2D_ENDPOINT_B].maxPacketSize;
  buffPtr = Buff;
  pISR    = getProtocolISR();
  pCmd    = getProtocolCmd();

  //reject calls from Tx callback
  //defensive coding
  //-----------------------------
  if( direction == USB_SEND )
    return;

  if (actual_transfer_length == 0)
  {
      CI2_ResetBufPosition( usbReceiveEp , size);
  }


  // if theres an error
  // reprime the EP and hope
  // the WTPTP host re-sends
  //------------------------
  if( errors ){
  	#if USE_SERIAL_DEBUG
  		serial_outstr("USB ISR with error\n");
		serial_outnum(errors);
		serial_outstr("\n");
	#endif
      CI2_ResetBufPosition( usbReceiveEp , size);
	  return;
   }
  // if downloads are disabled then reprime so we do not crash the host
  // dump any data received in the bit bucket
  if( !(getProtocolISR()->DownloadsEnabled) ){
        CI2_ResetBufPosition( usbReceiveEp , size);
  		return;
  	}
  packetLength = actual_transfer_length;

  // Is it the Preamble ??
  //----------------------
  if((packetLength == PREAMBLE_SIZE ) && (pISR->PreambleReceived == FALSE))
  {
      //Clear the stack buffer
      //----------------------
      memset( buffPtr, 0, sizeof(Buff));

      //Get the bytes from the U2D
      //--------------------------
      CI2Receive( PREAMBLE_SIZE, buffPtr, &byteCount);

      // See if it's the Preamble string
      //--------------------------------
      if( ProtocolVerifyPreamble( buffPtr ) != 0 )
      {
         //ShutdownPort(FFUART_D);
         //ShutdownPort(DIFF_USB_D);
         pISR->PreambleReceived = TRUE;
         pISR->InterruptPort    = CI2_USB_D;

		 ClearPortInterruptFlag();

         // Preamble checks out, so respond
         //--------------------------------
         SendResponse(Buff, PREAMBLE_SIZE);
         CI2_ResetBufPosition( usbReceiveEp , size);
		 return;
        }
  }

  //-------------------------------------------------------------------
  // ok, process command stream
  //-------------------------------------------------------------------


  // packet size seems ok ...
  // read constant part of command packet
  //-------------------------------------
  if(!cmd_in_progress)
  {
	CI2Receive( 8, (XLLP_UINT8_T *)&(pCmd->Command), &byteCount);
  	packetLength -= byteCount;

	// update Interrupt Status Flags
	//-----------------------------
	pISR->CurrentCommand  = pCmd->Command;
    if(pISR->CurrentCommand!= UploadDataCmd && pISR->CurrentCommand!= MessageCmd)
        pISR->DataLeft        = pCmd->Length;
        
  }

  switch( pISR->CurrentCommand )
  {
  	case INVALIDCMD:
		 HandleInvalidCmd();
		 CI2_ResetBufPosition( usbReceiveEp , size);
		 break;
	case MessageCmd:
		 //try and send a message
		 HandleMessageCmd();
		 CI2_ResetBufPosition( usbReceiveEp , size);
		 break;
	case ProtocolVersionCmd:
		 // Send the major and minor protocol version bacl
		 HandleProtocolVersionCmd();
		 CI2_ResetBufPosition( usbReceiveEp , size);
		 break;
	case GetParametersCmd:
		 // Send Protocol Parameters to host.
		 HandleGetParametersCmd();
		 CI2_ResetBufPosition( usbReceiveEp , size);
		 break;
	case DownloadDataCmd:
         // Next we get the rest of the image that follows
         // the command stucture and put it in the proper
         // image area
         //
         //-----------------------------------------------
		 //read the remaining part of the buffer
         pISR->CommandReceived = TRUE;
         if(!fast_download)
         {
		 CI2Receive(packetLength , (XLLP_UINT8_T *)(pISR->CurrentTransferAddress), &byteCount);
		 //update counting stats
		 pISR->CurrentTransferAddress += byteCount;
		 pISR->DataLeft -= byteCount;
         }
         else
         {
         // No temporary buffer. Data already in DDR. Decrement DataLeft.
         pISR->CurrentTransferAddress += packetLength;
		 pISR->DataLeft -= packetLength;
         }

		 //don't prime for a zero length
		 if(pISR->DataLeft > 0)
		 {
            cmd_in_progress = 1;
			pISR->CommandReceived = FALSE;
            if(!fast_download)
            {
            /* Normal Download */
            size = (pISR->DataLeft > DATA_BLOCK_SIZE) ? pISR->DataLeft : defaultU2DEndpointConfigTable[U2D_ENDPOINT_B].maxPacketSize;
            CI2_ResetBufPosition( usbReceiveEp , size);
            }
            else
            {
            /* Fast Download */
			size = (pISR->DataLeft < FD_BLOCK_SIZE ) ? pISR->DataLeft : FD_BLOCK_SIZE;
			
			CI2_ResetBufPositionForDDR( usbReceiveEp , size, (UINT8 *)pISR->CurrentTransferAddress);
            }
		 }
		 else
		 {
		 	cmd_in_progress = 0;
            fast_download = 0;

		 	pISR->CommandReceived = TRUE;
            // No cmd portion in data for fast download. HandleDownloadFlow needs to go to right handler after download.
            pCmd->Command = DownloadDataCmd;
            CI2_ResetBufPosition( usbReceiveEp , size);
		 }

		 break;
	case DisconnectCmd:
         pISR->CommandReceived = TRUE;
		 pISR->PreambleReceived = TRUE;
		 // Note that the "break" is intentionally left out in this case. 
		 // Coverity might complain about this but it is safe to ignore. 
    default:
         pISR->CommandReceived = TRUE;
         if(packetLength)
           CI2Receive( packetLength, (XLLP_UINT8_T *)(&(pCmd->Data[0])), &byteCount);

         if(pISR->CurrentCommand == DataHeaderCmd)
         {
            if (pCmd->Flags & FAST_DOWNLOAD)
            {
                cmd_in_progress = 1;
                fast_download = 1; // will not strip cmd from data packet and download directly to DDR.
                pISR->DataLeft = *(UINT32_T *)&pCmd->Data[0]; // size of entire file obtained from Data header payload.
                pISR->CurrentCommand = DownloadDataCmd; // So that we fall into DownloadDataCmd for next interrupt.
                
                CI2_ResetBufPositionForDDR( usbReceiveEp , pISR->DataLeft < FD_BLOCK_SIZE ? pISR->DataLeft: FD_BLOCK_SIZE, (UINT8 *)pISR->CurrentTransferAddress);
            }
            else
            {
                CI2_ResetBufPosition( usbReceiveEp , size);
            }
         }
         else
            CI2_ResetBufPosition( usbReceiveEp , size);
         break;
  }
  return;

}


//-----------------------------------------
//  ImageU2DInterruptHandler
//
//  Top Level Application interrupt handler
//  for U2D
//
//  We pass through the initial interrupt
//  to the lower-level handler first,
//  then take care of the application next
//
//-----------------------------------------
void ImageCI2InterruptHandler(void)
{

  // This central handler will chain off
  // to
  //-------------------------------------
  CI2InterruptHandler();

  //allow enumeration.  check to make sure downloads are allowed
  if( ( xllpIsEnumCompleted()) == XLLP_TRUE )
  {
      if( !(getProtocolISR()->DownloadsEnabled) )
	  {
         ClearPortInterruptFlag();
         xllpCI2InterruptDisable();
      }
  }
}
   
#endif // USBCI
