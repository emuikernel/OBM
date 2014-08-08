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

 ******************************************************************************
**
**  FILENAME:	security.h
**
**  PURPOSE: 	Holds security related common definitions
**
******************************************************************************/

#ifndef _security_h
#define _security_h

// Key Lengths
#define WordLengthOf_PKCS1024			  	32
#define WordLengthOf_PKCS2048				64
#define WordLengthOf_ECDSA256				8
#define WordLengthOf_ECDSA521				17
#define WordLengthOf_SHA1		 			5
#define WordLengthOf_SHA256		 			8
#define WordLengthOf_SHA512		 			16

#define ByteLengthOf_SHA1		 			20
#define ByteLengthOf_SHA256		 			32
#define ByteLengthOf_SHA512		 			64


#define MAXRSAKEYSIZEWORDS	 	64				// 2048 bits
#define MAXECCKEYSIZEWORDS 		17				// 521 bits+

#endif
