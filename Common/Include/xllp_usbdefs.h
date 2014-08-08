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
**	 xllp_usbdefs.h
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
 
 ******************************************************************************
**  FILENAME:       xllp_usbdefs.h
**
**  PURPOSE:        This file defines constants for USB regardless of 1.1 or 2.0
**
**
******************************************************************************/

#ifndef __XLLP_USBDEFS_H
#define __XLLP_USBDEFS_H


#define XLLP_U2D_REQ_TYPE_MASK (0x3 << 5)      // Used to identify a Request Type

// USB Setup transaction structure
typedef struct XLLP_U2D_USB_SETUP_DATA_S
{
   XLLP_UINT8_T  bmRequestType;
   XLLP_UINT8_T  bRequest;
   XLLP_UINT16_T wValue;
   XLLP_UINT16_T wIndex;
   XLLP_UINT16_T wLength;
} XLLP_U2D_USB_SETUP_DATA_T, *P_XLLP_U2D_USB_SETUP_DATA_T;

// Enumerate Request types
typedef enum XLLP_U2D_USB_REQUEST_TYPE_E
{
   STANDARD_REQ    = 0x00,
   CLASS_REQ       = 0x01,
   VENDOR_REQ      = 0x02
} XLLP_U2D_USB_REQUEST_TYPE_T;

typedef XLLP_U2D_USB_REQUEST_TYPE_T XLLP_UDC_USB_REQUEST_TYPE_T;

// Enumerate UDC Transfers Types
typedef enum XLLP_UDC_EP_TYPE_E
{
    CNTRL   = 0x00,
    ISO     = 0x01,
    BULK    = 0x02,
    INTERPT = 0x03
} XLLP_UDC_EP_TYPE_T;

// Enumerate UDC Endpoint Direction
typedef enum XLLP_UDC_EP_DIR_E
{
    OUT = 0x00,
    IN  = 0x01
} XLLP_UDC_EP_DIR_T;

// Enumerate U2D Endpoint Double Buffering Enabled/Disabled
typedef enum XLLP_U2D_EP_DOUBLE_BUFF_E
{
    DB_DIS  = 0,
    DB_EN   = 1
} XLLP_U2D_EP_DOUBLE_BUFF_T;

// Enumerate U2D Endpoint Enabled/Disabled
typedef enum XLLP_U2D_EP_ENABLED_E
{
    EP_DIS  = 0,
    EP_EN   = 1
} XLLP_U2D_EP_ENABLED_T;


// Enumerate UDC Endpoint Double Buffering Enabled/Disabled
typedef XLLP_U2D_EP_DOUBLE_BUFF_T XLLP_UDC_EP_DOUBLE_BUFF_T;

// Enumerate UDC Endpoint Enabled/Disabled
typedef XLLP_U2D_EP_ENABLED_T XLLP_UDC_EP_ENABLED_T;

// Enumerate UDC Endpoint Interrupts
typedef enum XLLP_UDC_EP_INTERRUPT_TYPE_E
{
    PACKET_COMPL_INT = 0,
    FIFO_ERROR_INT   = 1
} XLLP_UDC_EP_INTERRUPT_TYPE_T;

// Enumerate EP0 State machine
typedef enum XLLP_UDC_EP0_STATE_E
{
    EP0_IDLE_STATE      = 0,
    EP0_OUT_DATA_STATE  = 1,
    EP0_IN_DATA_STATE   = 2,
    EP0_END_XFER_STATE  = 3,
    EP0_JUMP_TO_IDLE    = 0xfe
} XLLP_UDC_EP0_STATE_T;

// Enumerate USB Standard Request types
typedef enum XLLP_UDC_USB_STANDARD_REQ_E
{
    GET_STATUS      = 0x00,
    CLEAR_FEATURE   = 0x01,
    SET_FEATURE     = 0x03,
    SET_ADDRESS     = 0x05,
    GET_DESCRIPTOR  = 0x06,
    SET_DESCRIPTOR  = 0x07,
    GET_CONFIGURATION = 0x08,
    SET_CONFIGURATION = 0x09,
    GET_INTERFACE   = 0x0a,
    SET_INTERFACE   = 0x0b,
    SYNC_FRAME      = 0x0c
} XLLP_UDC_USB_STANDARD_REQ_T;

// USB Descriptor types
typedef enum XLLP_2D_USB_DESCRIPTORS_E
{
    DEVICE_DESCRIPTOR   = 0x01,
    CONFIG_DESCRIPTOR   = 0x02,
    STRING_DESCRIPTOR   = 0x03,
    INTERFACE_DESCRIPTOR = 0x04,
    ENDPOINT_DESCRIPTOR = 0x05
} XLLP_U2D_USB_DESCRIPTORS_T;


// USB Descriptor types
typedef XLLP_U2D_USB_DESCRIPTORS_T XLLP_UDC_USB_DESCRIPTORS_T;

// Enumerate Vendor Requests
typedef enum XLLP_UDC_VENDOR_REQ_E
{
    VENDOR_SETUP_IN_EP     = 0x01,
    VENDOR_SETUP_OUT_EP    = 0x02,
    VENDOR_SETUP_INT_EP    = 0x03,
    VENDOR_SETUP_LOOPBACK  = 0x04
} XLLP_UDC_VENDOR_REQ_T;

#define XLLP_USB_REQ_RECIPIENT_MASK 0x1f

#endif
