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
#include "Typedef.h"
#include "Flash.h"
#include "general.h"
#include "Errors.h"
#include "I2C.h"
#include "tim.h"
#include "platform_setup.h"
#include "PlatformConfig.h"
 
/**************************************************

Add Platform specific code here

  At this point, the the flash is not initialized,
  and the TIM is may or may not be present.
  
  No other images have been loaded at this point

***************************************************/
UINT_T PlatformPreFlashSetup(void)
{
	UINT_T Retval = NoError;

	return Retval;
}
 
/**************************************************

Add Platform specific code here

  At this point, the OBM has initialized the flash,
  initialized ports and loaded/validated the TIM.

  The OBM has *not* loaded any other images

***************************************************/
UINT_T PlatformPreCodeSetup(pTIM pTIM_h, P_FlashProperties_T pFlashProp)
{
	return NoError;
}


/****************************************************

Add Platform specific code here

  At this point, the OBM has loaded the next image
  into memory and finished ALL validation.

  The OBM has *not* turns off the ports

*****************************************************/
UINT_T PlatformPostCodeSetup(pTIM pTIM_h, P_FlashProperties_T pFlashProp)
{
	return NoError;
}


/**************************************************************************************/
/**************************************************************************************/
I2C_ReturnCode PMICSetup ( void )
{
	I2C_ReturnCode		rc = I2C_RC_OK;

	return rc;

}

 /**************************************************************************************/
/**************************************************************************************/
I2C_ReturnCode ResetULPI ( void )
{
	I2C_ReturnCode		rc = I2C_RC_OK;
				
  	return rc;

}
