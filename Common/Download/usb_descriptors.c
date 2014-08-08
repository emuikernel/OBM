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
**	 USB_DESCRIPTORS.c
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
**  FILENAME:       usb_descriptors.c
**
**  PURPOSE:   		This files sets up USB descriptors from TIM or uses default.
**
**  LAST MODIFIED: $Modtime: 4/04/06 4:26p $
**
******************************************************************************/

#include "general.h"
#include "misc.h"
#include "Errors.h"
#include "tim.h"
#include "xllp_defs.h"
#include "xllp_u2d.h"
#include "xllp_udc.h"
#include "usb_descriptors.txt"
#include "ProtocolManager.h"
#include "usb_descriptors.h"

OPT_USB_DESCRIPTOR_LOADS USBDescriptors[NUM_USB_DESCRIPTORS];
UINT_T USBStringIndexTable[MAX_USB_STRINGS];
static pTIM  pTIMUSB_h = NULL;			// For Reference later by the USB driver for Host Device Requests in TIM

void CheckAndFillStringIndexTable (UINT_T index)
{
 pUSBDeviceDescriptor pUSBDevDesc;
 pUSBConfigDescriptor pUSBConfigDesc;

	// Is this a Device Descriptor? If so save off string index's
	if ((DESCRIPTOR_RES_ID | index) == USB_DEVICE_DESCRIPTOR)
	{
		pUSBDevDesc = (pUSBDeviceDescriptor)USBDescriptors[index].pDesPayload;
		if ((pUSBDevDesc->iManufacturer < MAX_USB_STRINGS) && (pUSBDevDesc->iManufacturer != 0))
			USBStringIndexTable[pUSBDevDesc->iManufacturer] = USB_MANUFACTURER_STRING_DESCRIPTOR;
		if ((pUSBDevDesc->iProduct < MAX_USB_STRINGS) && (pUSBDevDesc->iProduct != 0))
			USBStringIndexTable[pUSBDevDesc->iProduct] = USB_PRODUCT_STRING_DESCRIPTOR;
		if ((pUSBDevDesc->iSerialNumber < MAX_USB_STRINGS) && (pUSBDevDesc->iSerialNumber != 0))
			USBStringIndexTable[pUSBDevDesc->iSerialNumber] = USB_SERIAL_STRING_DESCRIPTOR;
	}
	// Is this a Config Descriptor? If so save off string index's
	if ((DESCRIPTOR_RES_ID | index) == USB_CONFIG_DESCRIPTOR)
	{
		pUSBConfigDesc = (pUSBConfigDescriptor)USBDescriptors[index].pDesPayload;
		if ((pUSBConfigDesc->InterDesc.iInterface <	MAX_USB_STRINGS) && (pUSBConfigDesc->InterDesc.iInterface != 0))
			USBStringIndexTable[pUSBConfigDesc->InterDesc.iInterface] = USB_INTERFACE_STRING_DESCRIPTOR;
	}
	return;
}

/* This routine fills out an array of descriptors from either the reserved area in the TIM or by defaults
* Format is in the form of:
*
*/
UINT_T SetUpUSBDescriptors (pTIM pTIM_h, UINT_T TIM_Use, UINT_T PortType)
{
 UINT_T Retval = NoError;
 pWTP_RESERVED_AREA_HEADER pWRAH = NULL;
 UINT_T 	i;
 UINT16_T   usb_id[2]; //vid and pid from fuses
 UINT8_T	ReservedAreaData = TRUE;

 // Save for later
 pTIMUSB_h = pTIM_h;

 //check for platform specific Vid and Pid overriding
 if ( GetUSBIDFuseBits(&usb_id[0], &usb_id[1]) != NoError)
 // zero out VID and PID on an error (most likely not supported by platform
 {	
 	usb_id[0] = 0;	//clear VID
 	usb_id[1] = 0;  //clear PID
 }

 // Check for USB Device Descriptor
 for (i=0; i<NUM_USB_DESCRIPTORS; i++)
 {
	if (TIM_Use)
	{
 		pWRAH = FindPackageInReserved (&Retval, pTIM_h, (DESCRIPTOR_RES_ID | i));
 		if (Retval == NoError)
		{
			USBDescriptors[i].DesSize = pWRAH->Size - sizeof (WTP_RESERVED_AREA_HEADER);
			USBDescriptors[i].pDesPayload = (UINT_T *)(pWRAH + 1);	// Go past the header
			CheckAndFillStringIndexTable(i);
		}
	}
	if ((!TIM_Use) || (Retval != NoError))
	{
	 	switch (DESCRIPTOR_RES_ID | i)
		{
		 case USB_DEVICE_DESCRIPTOR:
          if((PortType == U2D_USB_D) || (PortType == CI2_USB_D))
			 {
				USBDescriptors[i].DesSize = sizeof (U2DDeviceDesc);
				USBDescriptors[i].pDesPayload = (UINT_T *) U2DDeviceDesc;
			 	CheckAndFillStringIndexTable(i);
				if(usb_id[0])  //if VID from fuses is nonzero, then fill in VID
					*(UINT16_T *)&(((pUSBDeviceDescriptor)USBDescriptors[i].pDesPayload)->idVendor) = usb_id[0];
				if(usb_id[1])  //if PID from fuses is nonzero, then fill in PID
					*(UINT16_T *)&(((pUSBDeviceDescriptor)USBDescriptors[i].pDesPayload)->idProduct) = usb_id[1];
			 }
			#if USB1
			 else
			 {
		 		USBDescriptors[i].DesSize = sizeof (UsbDeviceDesc);
				USBDescriptors[i].pDesPayload = (UINT_T *) UsbDeviceDesc;
				CheckAndFillStringIndexTable(i);
			 }
			 #endif
		 	break;
		 case USB_CONFIG_DESCRIPTOR:
          if((PortType == U2D_USB_D) || (PortType == CI2_USB_D))
			 {
		 		USBDescriptors[i].DesSize = sizeof (U2DConfigDesc);
				USBDescriptors[i].pDesPayload = (UINT_T *) U2DConfigDesc;
				CheckAndFillStringIndexTable(i);
			 }
			 #if USB1
			 else
			 {
		 		USBDescriptors[i].DesSize = sizeof (UsbConfigDesc);
				USBDescriptors[i].pDesPayload = (UINT_T *) UsbConfigDesc;
			 	CheckAndFillStringIndexTable(i);
			 }
			 #endif
		 	break;
		 case USB_INTERFACE_DESCRIPTOR:
		 	break;
		 case USB_LANGUAGE_STRING_DESCRIPTOR:
		 	USBDescriptors[i].DesSize = sizeof (UsbStringDesc_LangID);
			USBDescriptors[i].pDesPayload = (UINT_T *) UsbStringDesc_LangID;
		 	USBStringIndexTable[0] = USB_LANGUAGE_STRING_DESCRIPTOR;
		 	break;
		 case USB_MANUFACTURER_STRING_DESCRIPTOR:
		 	USBDescriptors[i].DesSize = sizeof (UsbStringDesc_ManufacturerStr);
			USBDescriptors[i].pDesPayload = (UINT_T *) UsbStringDesc_ManufacturerStr;
		 	break;
		 case USB_PRODUCT_STRING_DESCRIPTOR:
		 	USBDescriptors[i].DesSize = sizeof (UsbStringDesc_ProductStr);
			USBDescriptors[i].pDesPayload = (UINT_T *) UsbStringDesc_ProductStr;
		 	break;
		 case USB_SERIAL_STRING_DESCRIPTOR:
		 	USBDescriptors[i].DesSize = sizeof (UsbStringDesc_SerialNumStr);
			USBDescriptors[i].pDesPayload = (UINT_T *) UsbStringDesc_SerialNumStr;
		 	break;
		 case USB_INTERFACE_STRING_DESCRIPTOR:
		 	USBDescriptors[i].DesSize = sizeof (UsbStringDesc_InterfaceStr);
			USBDescriptors[i].pDesPayload = (UINT_T *) UsbStringDesc_InterfaceStr;
		 	break;
		 case USB_DEFAULT_STRING_DESCRIPTOR:
		 	USBDescriptors[i].DesSize = sizeof (UsbStringDesc_DefaultStr);
			USBDescriptors[i].pDesPayload = (UINT_T *) UsbStringDesc_DefaultStr;
			USBStringIndexTable[5] = USB_DEFAULT_STRING_DESCRIPTOR;			// Index 5 is the default id.
		 	break;
		 case USB_ENDPOINT_DESCRIPTOR:
		 	break;
		}
	 }
  }
  return Retval;
}

/* This routine will search the TIM and find the Vendor specific requested package.
   If Retval == NoError then success. Failure otherwise.
*/
UINT_T FindVendorRequestInTIM (UINT8 bRequest, UINT8 bmRequestType, PUINT32 pTxBuffEp0, PINT32 ppacketLength)
{
    pWTP_RESERVED_AREA_HEADER pWRAH;
	pUSB_VENDOR_REQ pUSBVREQ;
	UINT_T Retval;

	// Any USB Vendor Request packages?
	pWRAH = FindFirstPackageTypeInReserved (&Retval, pTIMUSB_h, USBVENDORREQ);
	if (Retval != NoError)
		return Retval;
	do
	{
		pUSBVREQ = (pUSB_VENDOR_REQ) pWRAH;
		if ((pUSBVREQ->bRequest == bRequest) && (pUSBVREQ->bmRequestType == bmRequestType))
		{
			*ppacketLength = pUSBVREQ->wLength;
			(*pTxBuffEp0) = (UINT_T) &pUSBVREQ->wData;
			return Retval;
		}
		pWRAH = FindNextPackageTypeInReserved(&Retval);
	}
	while (Retval == NoError);
	return Retval;
}