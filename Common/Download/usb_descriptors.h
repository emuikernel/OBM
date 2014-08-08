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
**	 USB_DESCRIPTORS.h
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
 
 ******************************************************************************/

/******************************************************************************
**
**  FILENAME:       usb_descriptors.h
**
**  PURPOSE:   		USB descriptor defs
**
**  LAST MODIFIED: Modtime: 2/14/2007
**
******************************************************************************/
#ifndef _USB_DESCRIPTORS_H
#define _USB_DESCRIPTORS_H

#include "Typedef.h"
typedef struct
{
 unsigned int DesSize;
 unsigned int * pDesPayload;
}
OPT_USB_DESCRIPTOR_LOADS, *pOPT_USB_DESCRIPTOR_LOADS;

typedef struct
{
	UINT8_T 	bLength;
	UINT8_T 	bDescriptorType;
	UINT8_T		bcdUSB[2];
	UINT8_T		bDeviceClass;
	UINT8_T		bDeviceSubClass;
	UINT8_T		bDeviceProtocol;
	UINT8_T		bMaxPacketSize;
	UINT8_T		idVendor[2];
	UINT8_T		idProduct[2];
	UINT8_T		bcdDevice[2];
	UINT8_T		iManufacturer;
	UINT8_T		iProduct;
	UINT8_T		iSerialNumber;
	UINT8_T		bNumConfigurations;
} USBDeviceDescriptor, *pUSBDeviceDescriptor;

typedef struct
{
 	UINT8_T		bLength;
 	UINT8_T		bDescriptorType;
	UINT8_T		wTotalLength[2];
	UINT8_T		bNumInterfaces;
	UINT8_T		bConfigurationValue;
	UINT8_T		Configuration;
	UINT8_T		bmAttributes;
	UINT8_T		bMaxPower;
} USBConfigurationDesc, *pUSBConfigurationDesc;

typedef struct
{
 	UINT8_T		bLength;
 	UINT8_T		bDescriptorType;
 	UINT8_T		bInterfaceNumber;
 	UINT8_T		bAlternateSetting;
 	UINT8_T		bNumEndpoints;
 	UINT8_T		bInterfaceClass;
 	UINT8_T		bInterfaceSubClass;
 	UINT8_T		bInterfaceProtocol;
 	UINT8_T		iInterface;
} USBInterfaceDesc, *pUSBInterfaceDesc;

typedef struct
{
	UINT8_T		bLength;
	UINT8_T		bDescriptorType;
	UINT8_T		bEndpointAddress;
	UINT8_T		bmAttributes;
	UINT8_T		wMaxPacketSize[2];
	UINT8_T		bInterval;
} USBEndPointDesc, *pUSBEndPointDesc;

typedef struct
{
	USBConfigurationDesc 	ConfigDesc;
	USBInterfaceDesc		InterDesc;
	USBEndPointDesc			EndDesc[2];
} USBConfigDescriptor, *pUSBConfigDescriptor;

UINT_T SetUpUSBDescriptors (pTIM pTIM_h, UINT_T TIM_Use, UINT_T PortType);

#endif
