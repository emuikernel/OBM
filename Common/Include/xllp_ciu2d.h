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
#ifndef __XLLP_CIU2D_H
#define __XLLP_CIU2D_H

#include "xllp_defs.h"
#include "xllp_u2d.h"


// Define some option fields / flags for platform
// specific options
// optionFlags is passed down init chain
//-----------------------------------------------
#define CI_OTG_OPTIONS_FULL_SPEED (1 << 0)
#define CI_OTG_OPTIONS_8BIT_XCVR_WIDTH (1 << 1)
#define CI_OTG_OPTIONS_XCR_MASK (7 << 2) /* 3 bits */
#define CI_OTG_OPTIONS_UTMI_XCVR_SELECT ( 1 << 2 )
#define CI_OTG_OPTIONS_UPLI_XCVR_SELECT ( 1 << 3 )
#define CI_OTG_OPTIONS_SERIAL_XCVR_SELECT ( 1 << 4 )


XLLP_STATUS_T   xllpCI2HWSetup ();
XLLP_STATUS_T   xllpCI2HWShutdown();
XLLP_UINT32_T   xllpCI2SWInit (P_XLLP_U2D_EP_CONFIG_TABLE_T pEPCfgTbl, void (*pCI2CpuIntEnable)(void),
                                   void (*pPlatformReadyFunc)(void), UINT32 optionFlags);
XLLP_UINT8_T    xllpCI2Xfer ( XLLP_U2D_USB_EP_T  usbEndPtId, XLLP_U2D_EP_DIR_T  epXferDir,  P_XLLP_UINT8_T  bufPtr, XLLP_UINT32_T  bufSize, XLLP_INT32_T  *actualCount );
void            CI2InterruptHandler();
XLLP_INT32_T    xllpCI2GetTransferLength (XLLP_U2D_USB_EP_T epNum, XLLP_U2D_EP_DIR_T direction);
XLLP_BOOL_T     xllpIsEnumCompleted  ();
XLLP_UINT32_T   xllpCI2GetEndPointTransferDirection (XLLP_U2D_USB_EP_T endPtId);
void            xllpCI2InterruptDisable();
void            xllpCI2InterruptEnable();
UINT8           CI2_PrimeRxDtd( UINT32 RxSize, XLLP_U2D_USB_EP_T usbEndPtId );


/*------------------------------------------------------------------------------------------------------
// PUBLIC FUNCTIONS
//----------------------------------------------------------------------------------------------------*/
XLLP_UINT32_T CI2Transmit(XLLP_UINT32_T TxDataLength, P_XLLP_UINT8_T pTxBuff, XLLP_UINT8_T WaitState);
XLLP_UINT32_T CI2Download( void );
XLLP_UINT32_T BootRomCI2HWShutdown (void);
XLLP_UINT32_T BootRomCI2HWSetup(void);
XLLP_UINT32_T BootRomCI2SWInit (void(*CpuCI2InterruptEnable)(void), void(*pPlatformReadyFunc)(void), UINT32 optionFlags);

// Application starts the CI/OTG world with this func
//---------------------------------------------------
void CI2DownloadInit(void(*CpuCI2InterruptEnable)(void), void(*pPlatformReadyFunc)(void), UINT32 optionFlags);
void CI2InterruptHandler(void);
void ImageCI2InterruptHandler(void);

void CI2SetEndpointInterrupt( void *handle, UINT32 ep_num, UINT32 direction );
UINT32 CI2CheckEndpointInterrupt(void *handle, UINT32 ep_num, UINT32 direction );
void CI2ClearEndpointInterrupt(void *handle, UINT32 ep_num, UINT32 direction);

void CI2GetMsg( void *handle, UINT8 ep_num, UINT32_T setup, UINT8 direction,
                        UINT8 *buff_start_address, UINT32 actual_transfer_length, UINT8 errors);

void CI2_ResetBufPosition( XLLP_U2D_USB_EP_T  usbEndPtId , UINT recv_length);
void CI2_ResetBufPositionForDDR( XLLP_U2D_USB_EP_T  usbEndPtId , UINT recv_length, UINT8 * DDR_Location);
UINT8 CI2_PrimeRxDtd( UINT32 RxSize, XLLP_U2D_USB_EP_T usbEndPtId );
XLLP_UINT8_T xllpCI2EPInterruptReceived ( XLLP_U2D_USB_EP_T   usbEndPtId );
UINT8 mv_usb_start_ep0(void* handle);
#endif /* _xllp_ciu2d_h */

