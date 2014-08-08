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

#include "tim.h"
#include "Typedef.h"
#include "Flash.h"
#include "Errors.h"
#include "usb_descriptors.h"
#include "PlatformConfig.h"
#include "ProtocolManager.h"
#include "loadoffsets.h"    //For platform dependent FLASH_STREAM_SIZE definition

// Single copy must reside
static UINT_T TIMValidationStatus;

#if !BOOTROM
/* Single copy of the Tim
 * Currently only used by the Bootloader. Future versions of Bootrom 
 * can use this pointer.
 */
static TIM sTim;
pTIM GetTimPointer()
{
    return &sTim;
}
#endif

UINT_T GetTIMValidationStatus() { return TIMValidationStatus; }
void SetTIMValidationStatus(UINT_T status) { TIMValidationStatus = status; }

UINT_T SetTIMPointers( UINT8_T *pStartAddr, TIM *pTIM_h)
{
	pTIM_h->pConsTIM = (pCTIM) pStartAddr;	// Overlap Contant Part of TIM with actual TIM...

	// Assign a pointer to start of Images Area
	pTIM_h->pImg = (pIMAGE_INFO_3_4_0) (pStartAddr + sizeof (CTIM));

	// Assign a pointer to start of Key Area
	if (pTIM_h->pConsTIM->VersionBind.Version >= TIM_3_4_00)
		pTIM_h->pKey = (pKEY_MOD_3_4_0) ((UINT_T)pTIM_h->pImg + ((pTIM_h->pConsTIM->NumImages) * sizeof (IMAGE_INFO_3_4_0)));
	else
		pTIM_h->pKey = (pKEY_MOD_3_4_0) ((UINT_T)pTIM_h->pImg + ((pTIM_h->pConsTIM->NumImages) * sizeof (IMAGE_INFO_3_2_0)));

	// Assign a pointer to start of reserved area
	if (pTIM_h->pConsTIM->VersionBind.Version >= TIM_3_4_00)
		pTIM_h->pReserved = (PUINT) ((UINT_T)pTIM_h->pKey + ((pTIM_h->pConsTIM->NumKeys) * sizeof (KEY_MOD_3_4_0)));
	else if (pTIM_h->pConsTIM->VersionBind.Version >= TIM_3_3_00)
		pTIM_h->pReserved = (PUINT) ((UINT_T)pTIM_h->pKey + ((pTIM_h->pConsTIM->NumKeys) * sizeof (KEY_MOD_3_3_0)));
	else
		pTIM_h->pReserved = (PUINT) ((UINT_T)pTIM_h->pKey + ((pTIM_h->pConsTIM->NumKeys) * sizeof (KEY_MOD_3_2_0)));

	// Assign a pointer to start of DS
	pTIM_h->pTBTIM_DS = (pPLAT_DS) ((UINT_T)pTIM_h->pReserved + pTIM_h->pConsTIM->SizeOfReserved);
	return (NoError);
}

UINT_T LoadTim(UINT8_T *pTIMArea, TIM *pTIM_h, UINT_T SRAMLoad )
{
	UINT_T Retval = NoError;
	UINT_T TimSize = 0, i = 0;
	P_FlashProperties_T pFlashProp;
	UINT_T imageInfoSize,TimFlashAddress;
	pIMAGE_INFO_3_4_0 pImage;
	UINT8_T pTempTIMArea[512]; //512 bytes is required to support MMC reads in SDMA mode

	//clear out the first 512 bytes of TIM area, to delete any stale data
	memset(pTempTIMArea, 0, 512);

    if( SRAMLoad == TRUE)
    {
        // if no Flash properties were passed in then this was from a download and the TIM is in ISRAM
        // just set the pointers correctly
        SetTIMPointers(pTIMArea, pTIM_h);
        if( pTIM_h->pConsTIM->VersionBind.Identifier != TIMIDENTIFIER )
                Retval = TIMNotFound;

        return Retval;
    }

	// Determine default TIM address
	pFlashProp = GetFlashProperties(BOOT_FLASH);

	//read up to (including) the first Image_Info field of the TIM
	//then read the rest of the TIM based on the info read
	// - for XIP, this will let us know whether we can just leave the TIM in flash
	// - for NAND, this will keep us from reading unwritten pages
	TimSize = sizeof(VERSION_I) + sizeof(FLASH_I) + 12 + sizeof(IMAGE_INFO_3_4_0);

	//Note: For NAND and ONENAND, we need to read up to the first 10 blocks, incase
	//  block 0 got erased.  The backup boot block must be in the first good block
	for(i = 0; i < 10; i++)
	{
		TimFlashAddress = pFlashProp->TimFlashAddress + i*GetBlockSize(BOOT_FLASH);	
		
		//catch error, but we don't need to 'break' on the error.  this will be caught after loop
		Retval = ReadFlash(TimFlashAddress, (UINT_T)pTempTIMArea, TimSize, BOOT_FLASH);

		//don't need to catch error return, since SetTIMPointers doesn't have error code
		SetTIMPointers(pTempTIMArea, pTIM_h);
		//if we found a TIM, break out of loop
		if( pTIM_h->pConsTIM->VersionBind.Identifier == TIMIDENTIFIER )
		{	//remember where we found the TIM, so we know which block is the backup boot block
			pFlashProp->TimFlashAddress = TimFlashAddress;
			break;
		}
		//if we are reading any device BUT NAND or ONENAND, break after first iteration
		if( (pFlashProp->FlashType != ONENAND_FLASH) && (pFlashProp->FlashType != NAND_FLASH) && (pFlashProp->FlashType != SDMMC_FLASH) )
			break;
	}

	//now return if we had an error	or did not find a TIM or if TIM version is earlier than 3_2_00
	if ( (Retval != NoError) || 
		 (pTIM_h->pConsTIM->VersionBind.Identifier != TIMIDENTIFIER) ||
		 (pTIM_h->pConsTIM->VersionBind.Version < TIM_3_2_00) )
		return TIMNotFound;

	//at this point, we have read part of a TIM... read the rest
	pImage = FindImageInTIM(pTIM_h, TIMIDENTIFIER);
	TimSize = pImage->ImageSize; //less than 4K/8K

	// Read in the rest of the TIM
	Retval = ReadFlash(pFlashProp->TimFlashAddress, (UINT_T) pTIMArea, TimSize, BOOT_FLASH);

	if (Retval != NoError)
		return Retval;

	Retval = SetTIMPointers(pTIMArea, pTIM_h);
	if( pTIM_h->pConsTIM->VersionBind.Identifier != TIMIDENTIFIER )
		return TIMNotFound;

	return Retval;
}

// This function has been added to support to isolate backwards compatibility to tim.c
// It will return a point to pIMAGE_INFO based on TIM version.
pIMAGE_INFO_3_4_0 ReturnPImgPtr(pTIM pTIM_h, UINT_T ImageNumber)
{
 pIMAGE_INFO_3_4_0 pIMG;

 if (pTIM_h->pConsTIM->VersionBind.Version >= TIM_3_4_00)
	pIMG = (pIMAGE_INFO_3_4_0) &pTIM_h->pImg[ImageNumber];
 else
#ifdef LINUX_BUILD
	pIMG = &( ((pIMAGE_INFO_3_2_0) pTIM_h->pImg)[ImageNumber]);
#else
	(pIMAGE_INFO_3_2_0)pIMG = &( ((pIMAGE_INFO_3_2_0) pTIM_h->pImg)[ImageNumber]);
#endif

 return pIMG;
}

// This function will return the correct partition 
// taking offset for different versions into account
UINT_T ReturnImgPartitionNumber(pTIM pTIM_h, pIMAGE_INFO_3_4_0 pImg)
{
    if (pTIM_h->pConsTIM->VersionBind.Version >= TIM_3_4_00)
        return  pImg->PartitionNumber;
    else
        return  ((pIMAGE_INFO_3_2_0)pImg)->PartitionNumber;
}


UINT_T CheckReserved (pTIM pTIM_h)
{
	pWTP_RESERVED_AREA pWRA	= (pWTP_RESERVED_AREA) pTIM_h->pReserved;

	// Make sure that the TIM has a credible size
	if(pTIM_h->pConsTIM->SizeOfReserved == 0)
		return NotFoundError;

 	// Older TIM's had old reserved fields definition
 	if (pTIM_h->pConsTIM->VersionBind.Version == (0x00030101))
		return NotFoundError;

 	// Was this area in reserved field created by a WTP compliant tool so we can parse?
 	if (pWRA->WTPTP_Reserved_Area_ID != WTPRESERVEDAREAID)
		return NotFoundError;

	return NoError;
}


// Finds a Package of WTP recognized information in the reserved area based on identifier
pWTP_RESERVED_AREA_HEADER FindPackageInReserved (UINT_T * Retval, pTIM pTIM_h, UINT_T Identifier)
{
	UINT_T Count = 0;
	pWTP_RESERVED_AREA_HEADER pWRAH = NULL;
	pWTP_RESERVED_AREA pWRA	= (pWTP_RESERVED_AREA) pTIM_h->pReserved;

	*Retval = CheckReserved(pTIM_h);
	if (*Retval != NoError)
		return NULL;

	// Start from the begining
	pWRAH = (pWTP_RESERVED_AREA_HEADER) (pWRA + 1);

	while ((pWRAH->Identifier != Identifier) && (pWRAH->Identifier != TERMINATORID) && (Count < pWRA->NumReservedPackages))
	{
#ifdef LINUX_BUILD
			pWRAH = (UINT_T)pWRAH + (pWRAH->Size + 3) & ~3; // Skip to the next one
#else
			((UINT_T) pWRAH) += (pWRAH->Size + 3) & ~3;	// Skip to the next one
#endif
			Count++;
	}
	if (pWRAH->Identifier != Identifier)
	{
		*Retval = NotFoundError;
		pWRAH = NULL;
	}

	return pWRAH;
}



//--------------------------------------------------------------------------------------------------------
// These 2 functions allow for more than one instance of each package "TYPE" to be retrieved
// from the reserved area.
// The TIM is expected to be a static data area.
//--------------------------------------------------------------------------------------------------------

//------------------------------------------------------
// This data is shared between typical Getfirst/GetNext
// functions below.
//------------------------------------------------------
static pTIM pGetNextTim_h = NULL;
static pWTP_RESERVED_AREA_HEADER pLastIdFound = NULL;
static UINT_T lastIdType = INVALIDID; // = "!!!!"
static UINT_T reservedPackageCount = 0;

//---------------------------------------------------------------------------------------------------
//  FindFirstPackageInReserved()
//
//   Starting from the begining of the reserved area, search for the first instance of
//   a reserved package of the type specified by "Identifier"
//
//   update the static data for FindNextPackageInReserved()
//
//---------------------------------------------------------------------------------------------------
pWTP_RESERVED_AREA_HEADER FindFirstPackageTypeInReserved (UINT_T * Retval, pTIM pTIM_h, UINT_T Identifier)
{
  UINT_T Count = 0;
  pWTP_RESERVED_AREA_HEADER pWRAH;
  pWTP_RESERVED_AREA pWRA  = (pWTP_RESERVED_AREA) pTIM_h->pReserved;

  *Retval = CheckReserved(pTIM_h);
  if (*Retval != NoError)
  	return NULL;

  // Start from the begining of the reserved area
  //---------------------------------------------
  pWRAH = (pWTP_RESERVED_AREA_HEADER) (pWRA + 1);

  while( (pWRAH->Identifier != Identifier)   &&
         (pWRAH->Identifier != TERMINATORID) &&
         (Count < pWRA->NumReservedPackages)     )
  {
#ifdef LINUX_BUILD
	  pWRAH = (UINT_T)pWRAH + (pWRAH->Size + 3) & ~3;       // Skip to the next one
#else
	 ((UINT_T) pWRAH) += (pWRAH->Size + 3) & ~3;		// Skip to the next one
#endif
	 Count++;
  }

  if (pWRAH->Identifier != Identifier)
  {
    *Retval       = NotFoundError;
    pLastIdFound  = NULL;
    pGetNextTim_h = NULL;
    lastIdType    = INVALIDID;
    reservedPackageCount = 0;
    return NULL;
  }
  else
  {
    pLastIdFound         = pWRAH;
    lastIdType           = Identifier;
    reservedPackageCount = Count;
    pGetNextTim_h        = pTIM_h;
	return pWRAH;
  }
}

//---------------------------------------------------------------------------------------------------
//  FindNextPackageInReserved()
//
//
//
//---------------------------------------------------------------------------------------------------
pWTP_RESERVED_AREA_HEADER FindNextPackageTypeInReserved(UINT_T *Retval )
{
	pWTP_RESERVED_AREA_HEADER pWRAH;
	pWTP_RESERVED_AREA pWRA	= (pWTP_RESERVED_AREA) pGetNextTim_h->pReserved;
	*Retval = NoError;

    if( (lastIdType    == INVALIDID)  ||
        (pLastIdFound  == NULL)       ||
        (pGetNextTim_h == NULL)          )
	{
		*Retval = NotFoundError;
		return NULL;
	}

	// Start from the last item of Identifier type found
    //--------------------------------------------------
	pWRAH = pLastIdFound;
#ifdef LINUX_BUILD
    pWRAH = (UINT_T)pWRAH + pWRAH->Size;	// Skip to the next one
#else
    (UINT_T) pWRAH += pWRAH->Size;	  	// Skip to the next one
#endif
    reservedPackageCount++;
    while(  (pWRAH->Identifier != lastIdType)   &&
            (pWRAH->Identifier != TERMINATORID) &&
            (reservedPackageCount < pWRA->NumReservedPackages)  )
    {
#ifdef LINUX_BUILD
		pWRAH = (UINT_T)pWRAH + (pWRAH->Size + 3) & ~3; // Skip to the next one
#else
		((UINT_T) pWRAH) += (pWRAH->Size + 3) & ~3;	// Skip to the next one
#endif
		reservedPackageCount++;
	}

    // Reset the get First / Next interaction
    //---------------------------------------
	if( pWRAH->Identifier != lastIdType )
    {
      *Retval      = NotFoundError;
      pLastIdFound = NULL;
      lastIdType   = INVALIDID;
      reservedPackageCount = 0;
      return NULL;
    }
    else
    {
      pLastIdFound = pWRAH;
	  return pWRAH;
    }
}

pKEY_MOD_3_4_0 FindKeyInTIM(pTIM pTIM_h, UINT_T KeyID)
{
	pKEY_MOD_3_4_0 pKeyInfo = NULL;
	UINT8_T i;

	if (pTIM_h)
	{
		for( i = 0, pKeyInfo = pTIM_h->pKey; i < pTIM_h->pConsTIM->NumKeys; i++)
		{
			if(pKeyInfo->KeyID == KeyID)
				return pKeyInfo;
			pKeyInfo++;
		}
	}
	return NULL;
}


// This function is used to find an image information field in the TIM
// of the image with the ID passed in

pIMAGE_INFO_3_4_0 FindImageInTIM(pTIM pTIM_h, UINT_T ImageID)
{
	pIMAGE_INFO_3_4_0 pImageInfo = NULL;
	UINT8_T i;

	if (pTIM_h)
	{
		for( i = 0; i < pTIM_h->pConsTIM->NumImages; i++)
		{
			pImageInfo = ReturnPImgPtr(pTIM_h, i);
			if(pImageInfo->ImageID == ImageID)
				return pImageInfo;
		}
	}
	return NULL;
}

// This function is used to find an image information field in the TIM
// of the image with the ID passed in

pIMAGE_INFO_3_4_0 FindImageTypeInTIM(pTIM pTIM_h, UINT_T TypeID)
{
	pIMAGE_INFO_3_4_0 pImageInfo = NULL;
	UINT8_T i;

	if (pTIM_h)
	{
		for( i = 0; i < pTIM_h->pConsTIM->NumImages; i++)
		{
			pImageInfo = ReturnPImgPtr(pTIM_h, i);
			if(((pImageInfo->ImageID & TYPEMASK)>> 8) == TypeID)
				return pImageInfo;
		}
	}
	return NULL;
}


//-----------------------------------------------------------------------------------------
// InitDefaultPort
//
// Used to initialize default ports when no TIM is found or no port is specified in the TIM
//-----------------------------------------------------------------------------------------
void InitDefaultPort(pFUSE_SET pFuses)
{
	unsigned int port_selection;
	unsigned int usb_default_port;

	//if(pFuses->bits.PortEnabled || pFuses->bits.Download_Disable)
	//	return;

	//read the fuses to determine port configuration
	//port_selection= getPlatformPortSelection(&usb_default_port);
	port_selection = 7;
	usb_default_port = CI2_USB_D;
	
	switch(port_selection)
	{
	case 0:			// Single Ended USB only
		SetUpUSBDescriptors (NULL, 0, SE_USB_D);
		InitPort(SE_USB_D, pFuses);
		pFuses->bits.USBPort = USB_SE_PORT;
		break;
	case 1:			// AlTUART only
		InitPort(ALTUART_D, pFuses);
		pFuses->bits.UARTPort = ALTUART_PORT;
		break;
	case 2:			// FFUART and USB 1.1
	case 3:
		InitPort(FFUART_D, pFuses);
		SetUpUSBDescriptors (NULL, 0, usb_default_port);
		InitPort(usb_default_port, pFuses);
		pFuses->bits.USBPort = USB_DIFF_PORT;
		pFuses->bits.UARTPort = FFUART_PORT;
		break;
	case 4:			// FFUART and USB 2.0
		InitPort(FFUART_D, pFuses);
		SetUpUSBDescriptors (NULL, 0, usb_default_port);
		InitPort(usb_default_port, pFuses);
		pFuses->bits.USBPort = USB_U2D_PORT;
		pFuses->bits.UARTPort = FFUART_PORT;
		break;
	case 5:			// FFUART only
		InitPort(FFUART_D, pFuses);
		pFuses->bits.UARTPort = FFUART_PORT;
		break;
	case 6:			// FFUART and CI2
		InitPort(FFUART_D, pFuses);
		SetUpUSBDescriptors (NULL, 0, usb_default_port);
		InitPort(usb_default_port, pFuses);
		pFuses->bits.USBPort = USB_CI2_PORT;
		pFuses->bits.UARTPort = FFUART_PORT;
		break;
	case 7:			// CI2 only
		SetUpUSBDescriptors (NULL, 0, usb_default_port);
		InitPort(usb_default_port, pFuses);
		pFuses->bits.USBPort = USB_CI2_PORT;
		break;


	default:
		return;
	}
	pFuses->bits.PortEnabled = 1;
}

UINT_T CheckResumeBLPackage(void *pTIM){

	UINT_T Retval = NoError;
	pOPT_RESUME_SET pOPT_Resume;
	pWTP_RESERVED_AREA_HEADER pWRAH = NULL;

	// Is this TIM set for auto-bind?
	pWRAH = FindPackageInReserved (&Retval, pTIM, RESUMEBLID);
	if (Retval == NoError)
	{
		pOPT_Resume = (pOPT_RESUME_SET) pWRAH;
		if (!(pOPT_Resume->ResumeDDRInfo.ResumeAddr))
		{
			Retval = NotFoundError;
		}
	}
    return Retval;
}

//
// Add function to locate Consumer ID package 
//		returns a pointer to the CIPD_Entry_S for the consumer ID requested
//		retursn NULL if CIDP entry not found
//
pCIDP_ENTRY FindMyConsumerArray(pTIM pTIM_h, UINT_T CID){
   pCIDP_ENTRY ConsumerArray;
   pWTP_RESERVED_AREA_HEADER pHeader;  
   pCIDP_PACKAGE pCIDP_Pack;
   unsigned int Retval = NotFoundError;
   unsigned int i;
   unsigned int *pLocation;
   if(pTIM_h){
	   //First find the main CIDP package
	   pHeader =  FindPackageInReserved (&Retval, pTIM_h, CIDPID);
	   if (	(pHeader == NULL) || (Retval != NoError) )
			Retval = NotFoundError; 
	   else{
			Retval = NotFoundError; 
			// now get my consumer id array entry
			pCIDP_Pack = (pCIDP_PACKAGE)pHeader;
			ConsumerArray = (pCIDP_ENTRY)(&pCIDP_Pack->Consumers); 
			for( i = 0; i < pCIDP_Pack->NumConsumers; i++){
				if (ConsumerArray->ConsumerID == CID){
					Retval = NoError;
				 	break;
				}
				pLocation = (unsigned int *)ConsumerArray;
				// Size of CIDP_ENTRY is converted from (# of bytes) to (# of 32-bit words)
				pLocation += (sizeof(CIDP_ENTRY)/sizeof(int)) + ConsumerArray->NumPackagesToConsume - 1;
				ConsumerArray = (pCIDP_ENTRY)pLocation; 
			}  			
	   }
	}
    if (Retval != NoError)
		ConsumerArray = NULL;
	return ConsumerArray; 
} 