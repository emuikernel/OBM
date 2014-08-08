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
 /*************************************************************
 * Misc.h
 *
 * Contents:
 *      Definitions and functions declarations used fin the 
 *      Boot ROM development
 *
 *************************************************************/
#ifndef Misc_h
#define Misc_h

#include "tim.h"
#include "Errors.h"
#include "timer.h"
#include "misc_prototypes.h"
#include "downloader.h"

// a structure useful to contain all the info required to control gpio pins:
typedef struct GPIO_CFG_S
{
	unsigned long			 gpio_num;		// used to create masks for gpdr, gpsr, gpcr.
	volatile unsigned long	*mfpr_addr;		// used to select gpio alternate function
	unsigned long			 mfpr_val;		// used to select gpio alternate function
	volatile unsigned long	*gpio_addr;		// used to find gplr, gpdr, gpsr and gpcr.
	unsigned long			 gpio_dir;		
} GPIO_CFG_T;

#define GPLR_WORD			0				// word offset from gpio base of gpio level reg
#define	GPDR_WORD			3				// word offset from gpio base of gpio direction reg
#define	GPSR_WORD			6				// word offset from gpio base of gpio set reg
#define	GPCR_WORD			9				// word offset from gpio base of gpio clear reg

#define MOD_ADLER		65521	
#define MOD_LOOPS       5550

/* moved to misc_prototypes.h
int strlen(const char *String);
void memset(void *Addr, unsigned char Val, unsigned long Size); 
void *memcpy(void *dest, const void *src, unsigned int n);
int memcmp( const void *buffer1, const void *buffer2, int count);
int strcpy(char *dest, char *src);
UINT_T ReverseBytes (UINT8_T* Address, UINT_T Size);
unsigned int Endian_Convert (unsigned int in);
int strlen(const char *String);
int strcmpl( char* str1, char* str2, int maxlength );
int stricmpl( char* str1, char* str2, int maxlength );
void ConvertIntToBuf8(unsigned char* StringPtr,unsigned int Value, int Width,unsigned int Count);
INT_T memcmpFF( const void *buffer, INT_T count);
*/
#endif // Misc_h
