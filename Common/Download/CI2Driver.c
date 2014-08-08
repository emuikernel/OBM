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
**  COPYRIGHT (C) 2006 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be construed as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document.
**  Except as permitted by such license, no part of this document may be
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation.
**
**  FILENAME:       CI2Driver.c
**
**  PURPOSE:        This files contains the BootROM specific functions for
**                  ChipIdea 13611-a USB 2.0 OTG Device controller
**
**  LAST MODIFIED:
**
******************************************************************************/

/*
*******************************************************************************
*   HEADER FILES
*******************************************************************************
*/

#include "Interrupts.h"
#include "misc.h"
#include "Errors.h"
#include "xllp_ciu2d.h"

#if USBCI


extern const XLLP_U2D_EP_CONFIG_TABLE_T defaultU2DEndpointConfigTable[];
extern void ClearPortInterruptFlag();

extern const    XLLP_U2D_USB_EP_T             usbTransmitEp;
extern const    XLLP_U2D_USB_EP_T             usbReceiveEp;

//---------------------------------------------------------------
// BootRomCI2HWShutdown
//
//
//---------------------------------------------------------------
XLLP_UINT32_T BootRomCI2HWShutdown (void)
{
    XLLP_UINT32_T status = FALSE;

    // Disable the CI2 interrupt at the SOC CPU
    //-----------------------------------------
    DisablePeripheralIRQInterrupt(USB0_OTG_INT);

    //Shut down the CI2 peripheral
    //----------------------------
    status = xllpCI2HWShutdown();

    return status;
}

//---------------------------------------------------------------
//  BootRomCI2SWInit()
//      Get entier driver S/W and periperhal H/W up
//
//---------------------------------------------------------------
XLLP_UINT32_T BootRomCI2SWInit(void (*pCpuCI2IntEnable)(void), void (*pPlatformReadyFunc)(void), UINT32 optionFlags )
{
   XLLP_UINT32_T status;

   status = xllpCI2SWInit ((P_XLLP_U2D_EP_CONFIG_TABLE_T)defaultU2DEndpointConfigTable, pCpuCI2IntEnable, pPlatformReadyFunc, optionFlags);
   return status;
}

#endif  // USBCI
