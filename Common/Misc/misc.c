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
 *
 *
 *  FILENAME:	Misc.c
 *
 *  PURPOSE: 	Contain helpful misc. functions
 *
******************************************************************************/

#include "misc.h"


static unsigned char* 		StrPtr8;
static unsigned int   		DataWidth;

// Very simple routine to reverse bytes on up to a 512 byte buffer.
UINT_T ReverseBytes (UINT8_T* Address, UINT_T Size)
{
	UINT8_T Buffer[512];
	UINT_T i;

	if (Size > 512)
		return InvalidSizeError;

	// First copy to a Buffer
	for (i=0; (i < Size); i++)
		Buffer[i] = Address[i];

	// Next copy to Address in reverse order
	for (i=0; (i < Size); i++)
		Address[Size - 1 - i] = Buffer[i];

	return NoError;
}

//endian conversion function: self explanatory
unsigned int Endian_Convert (unsigned int in)
{
	unsigned int out;
	out = in << 24;
	out |= (in & 0xFF00) << 8;
	out |= (in & 0xFF0000) >> 8;
	out |= (in & 0xFF000000) >> 24;
	return out;
}

/**
 * Memory set function
 **/
void memset(void *Addr, unsigned char Val, unsigned long Size)
{

	unsigned long i;
	for(i=0; i < Size; i++ )
	{
		((unsigned char*)Addr)[i] = Val;
	}
}

void *memcpy(void *dest, const void *src, unsigned int n)
{
	const unsigned char *s = (const unsigned char*)src;
	unsigned char *d = (unsigned char *)dest;

	while(n-- > 0)
		*d++ = *s++;

	return dest;
}

int memcmp( const void *buffer1, const void *buffer2, int count)
{
    UINT8_T* buf1 = (UINT8_T*)buffer1;
    UINT8_T* buf2 = (UINT8_T*)buffer2;

  while(count)
  {
    if( *(buf1) != *(buf2) )
      return( (*buf1 < *buf2) ? -1 : 1 );

    buf1 += sizeof(UINT8_T);
    buf2 += sizeof(UINT8_T);
    count--;
  }

  return 0;
}

int ConvertStringToInteger8(const unsigned char* StringPtr, unsigned int Count)
{
	int  Output = 0;
	unsigned char c;
	while (*StringPtr)
	{
		Output *= Count;
		c = *StringPtr++;
		if (c >= '0' && c <= '9')
			Output += (c-'0');
		else
			Output += ((c|' ')-'a'+10);
	}
	return (Output);
}

int DivideTwoNumbers(int Numerator,int Denominator)
{
	int Tmp    = 0;
	int Output = 0;
	int Count;
    if(!Denominator)
        return Numerator;
        
	for (Count = 28; Count >= 0; Count -= 4)
	{
		Tmp    <<= 4;
		Output <<= 4;
		Tmp |= (Numerator >> Count)&0xf;
		while(Tmp >= 0)
		{
			Tmp -= Denominator;
			Output++;
		}
		Tmp += Denominator;
		Output--;
	}
	return(Output);
}

int ModOfTwoNumbers(int Numerator, int Denominator)
{
	//return(Numerator - (Denominator * DivideTwoNumbers(Numerator, Denominator)));
	int Tmp;
	Tmp = DivideTwoNumbers(Numerator, Denominator);
	Tmp = Tmp * Denominator;
	Tmp = Numerator - Tmp;
#if LINUX_BUILD
	// The GCC compiler assigns the value in R1(Quotient) to variable on the left side of the "="
	// in the calling code. So we need to move the Remainder (in Tmp) to R1 so we get the Remainder
	// returned and not the quotient.
	// Syntax below informs the compiler we modified R1 intentionally and don't mess with it.
	asm("mov r1, %[value]" :: [value] "r" (Tmp): "r1");   // this move the contents of temp into r1
#endif
	return Tmp;
}


void ConvertLongIntToBuf8(int DataValue, int Count)
{
	if (DataValue >= Count)
	{
		ConvertLongIntToBuf8(DivideTwoNumbers(DataValue, Count), Count);
		StrPtr8++;
		DataWidth--;
		DataValue = ModOfTwoNumbers(DataValue, Count);
	}
	if (DataWidth > 0)
		*StrPtr8 = (unsigned char)(DataValue + (DataValue < 10 ? '0' : ('a'-10)));
}


void ConvertIntToBuf8(unsigned char* StringPtr,unsigned int Value, int Width,unsigned int Count)
{
	int i;
	StrPtr8    = StringPtr;
	DataWidth  = Width;
	if (Value >= Count)
	{
		ConvertLongIntToBuf8(DivideTwoNumbers(Value,Count),Count);
		StrPtr8++;
		DataWidth--;
		Value=ModOfTwoNumbers(Value,Count);
	}
	if (DataWidth-->0)
		*StrPtr8=(unsigned char)(Value + (Value < 10 ? '0' : ('a'-10)));

	// If DataWidth=0, the following will do nothing, so just return here
	if (DataWidth == 0) return;

	// Shift the contents of the buffer to right by DataWidth to achieve right alignment
	for (i=Width; i > DataWidth; i--)
	{ 
		*(StrPtr8+DataWidth) = *(StrPtr8);
		StrPtr8--;
	}
	// Prepend leading zeros if necessary
	while (DataWidth-- > 0)
		*++StrPtr8 = '0';
}

void __aeabi_idiv0( void ){

	//dummy function for the compiler
	// need to investigate the in32t_divide.o and x0_a000.o libraries from XDB
	return;
}

//The below functions are pulled in from a library when built by RVCT or SDT compiler (NDT flag)
//When built by the linux compiler, we need to define the calls ourselves
#if LINUX_BUILD

void __aeabi_idiv(void){
//void __divsi3(void){
	asm("b DivideTwoNumbers");
	return;
}

void __aeabi_uidiv(void){
//void __udivsi3(void){
	asm("b DivideTwoNumbers");
	return;
}

void __aeabi_uidivmod(void){
	asm("b ModOfTwoNumbers");
	return;
}

#endif


/*
   String Compare function - case sensitive
   Will compare str1 to str2, character to character, until a NULL character is hit
   or until maxlength is reached
   If no NULL character is present, there could be serious issues

      Return values:
      -1 = str1 < str2
       0 = Match
       1 = str1 > str2

 */
int strcmpl( char* str1, char* str2, int maxlength )
{
    int length = maxlength;
    while((*str1 != NULL) && (*str2 != NULL) && --length)
    {
        if (*str1 != *str2)
        {
            // check the char that was different to determine results
            if ( *str1 > *str2 )
                return 1;
            else
                return -1;
        }
        str1++; str2++;
    }

    // strings compared so far, but did we reach the end of both?
    if ( *str1 != NULL && *str2 != NULL )
        // did not find the NULLs but have reached the maxlength to compare
        return 0;

    if ( *str1 == NULL && *str2 == NULL )
        return 0;

    if ( *str1 != NULL && *str2 == NULL )
        // "abcde" > "abcd"
        return 1;
    else
        // "abcd" < "abcde"
        return -1;
}


/*
   String Compare function - case insensitive
   Will compare str1 to str2 ignoring case, character to character, until a NULL character is hit
   or until maxlength is reached
   If no NULL character is present, there could be serious issues

      Return values:
      -1 = str1 < str2
       0 = Match
       1 = str1 > str2
 */
int stricmpl( char* str1, char* str2, int maxlength)
{
    int length = maxlength;
    while((*str1 != NULL) && (*str2 != NULL) && --length)
    {
        if ( ((*str1) & 0xDF) != ((*str2) & 0xDF) )
        {
            // check the char that was different to determine results
            if ( ((*str1) & 0xDF) > ((*str2) & 0xDF) )
                return 1;
            else
                return -1;
        }
        str1++; str2++;
    }

    // strings compared so far, but did we reach the end of both?
    if ( *str1 != NULL && *str2 != NULL )
        // did not find the NULLs but have reached the maxlength to compare
        return 0;

    if ( *str1 == NULL && *str2 == NULL )
        return 0;

    if ( *str1 != NULL && *str2 == NULL )
        // "abcde" > "abcd"
        return 1;
    else
        // "abcd" < "abcde"
        return -1;
}


INT_T strlen (const char * str)
{
    int i = 0;
    while(str[i++] != '\0')
        if(i > 1000)
            break;
    return --i;
}

INT_T memcmpFF( const void *buffer, INT_T count)
{
    UINT8_T* buf = (UINT8_T*)buffer;

    while(count--)
    {
        if( *(buf) != 0xFF )
          return -1;

        buf += sizeof(UINT8_T);
    }

    return 0;
}

void bubble(UINT16_T * ptr, UINT_T n)
{
	UINT_T i, j, temp;

	for (i = 0; i < n-1; i++)
		for (j = i + 1; j < n; j++)
			if (ptr[i] > ptr[j])
				{
					temp = ptr[j];
					ptr[j] = ptr[i];
					ptr[i] = temp;
				}
}

UINT CalcImageChecksum( UINT_T* DownloadArea, UINT_T ImageLength)
{
    UINT ImageChecksum = 0;
    UINT32* ptr32 = DownloadArea;
    UINT32* pEnd = ptr32 + (ImageLength / sizeof(UINT32));
    UINT BytesSummed = 0;

    while ( ptr32 < pEnd )
    {
        // checksum format version 2 algorithm as defined by flasher
        ImageChecksum ^= (*ptr32);
        ptr32++;
        BytesSummed += sizeof(UINT32);
    }
    return ImageChecksum;
}

UINT_T CalcMRDChecksum( UINT_T* start, UINT_T length)
{
	UINT8_T *pRD_DDR_end = NULL;
	UINT_T check_sum = 0, num = 0;
	UINT_T a = 0x0001, b = 0x0000, index = 0;
	UINT8_T * pRAMbuf;

	pRAMbuf = (UINT8_T *)start;
	pRD_DDR_end = pRAMbuf + length;
	
	do
	{
		num = (UINT_T)(*pRAMbuf++);
		a += num;
		index++;
		b += a;
		
		/*
		Algorithm is defined as mod MOD_ADLER at every modification to a and b.
		For efficiency reasons this mod can be done once in a while to keep (a) and (b) in range (32-bit signed)
		The result is invariant of MOD_LOOPS value.
		a<5550*256; b<(5551*256/2)*5550=3943430400 (in range) 
		*/
		if(index == MOD_LOOPS)
		{
			index =0;
			a %= MOD_ADLER;
			b %= MOD_ADLER;
		}
		// TBD: MISALIGNED 32-BIT ACCESS HERE AT EVERY BYTE ADDRESS, VERY INEFFICIENT & REDUNDANT
		// Kept here for now for compatibility with old version of the tool that produces RDA with DADADADA at misaligned offset

	} while( pRAMbuf < (UINT8_T *)pRD_DDR_end );

	if(index)
	{
		a %= MOD_ADLER;
		b %= MOD_ADLER;
	}

	check_sum =  (b << 16) | a;

	return check_sum;
}

