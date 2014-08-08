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
#ifndef Misc_prototypes_h
#define Misc_prototypes_h

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
void bubble(UINT16_T * ptr, UINT_T n);
UINT CalcImageChecksum( UINT_T* DownloadArea, UINT_T ImageLength);
UINT_T CalcMRDChecksum( UINT_T* start, UINT_T length);

#endif // Misc_h
