/*
LzmaDecode.h
LZMA Decoder interface
LZMA SDK 4.01 Copyright (c) 1999-2004 Igor Pavlov (2004-02-15)
*/

#ifndef __LZMADECODE_H
#define __LZMADECODE_H

//#define _LZMA_IN_CB
/* Use callback for input data */

/*#define _LZMA_OUT_READ*/
/* Use read function for output data */

#define _LZMA_PROB32
/* It can increase speed on some 32-bit CPUs,
   but memory usage will be doubled in that case */

#define _LZMA_LOC_OPT
/* Enable local speed optimizations inside code */

#ifndef UInt32
#ifdef _LZMA_UINT32_IS_ULONG
#define UInt32 unsigned long
#else
#define UInt32 unsigned int
#endif
#endif

#ifdef _LZMA_PROB32
#define CProb UInt32
#else
#define CProb unsigned short
#endif

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1
#define LZMA_RESULT_NOT_ENOUGH_MEM 2

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_INTERNAL_BUFF 		0x01000000 // 0x07000000~~~0x07F00000 is not used when OBM loads images

#if MIFI_V2R1 || MIFI_V3R0 || MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3 || NEZHA_MIFI_V4R1 || NEZHA_MIFI_V4R1R1
#define DECOMPRESS_TEMP_ADDR	0x1300000
#define DECOMPRESS_LENGTH		0x900000
#else
#define DECOMPRESS_TEMP_ADDR	0x1700000
#define DECOMPRESS_LENGTH		0x600000
#endif

/*
bufferSize = (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (lc + lp)))* sizeof(CProb)
bufferSize += 100 in case of _LZMA_OUT_READ
by default CProb is unsigned short,
but if specify _LZMA_PROB_32, CProb will be UInt32(unsigned int)
*/

#ifdef _LZMA_OUT_READ
int LzmaDecoderInit(
    unsigned char *buffer, UInt32 bufferSize,
    int lc, int lp, int pb,
    unsigned char *dictionary, UInt32 dictionarySize,
  #ifdef _LZMA_IN_CB
    ILzmaInCallback *inCallback
  #else
    unsigned char *inStream, UInt32 inSize
  #endif
);
#endif

#ifdef _LZMA_IN_CB
int LzmaReadCompressed(void *object, unsigned char **buffer, unsigned int *size);
#endif

int LzmaDecode(
    unsigned char *buffer,
  #ifndef _LZMA_OUT_READ
    UInt32 bufferSize,
    int lc, int lp, int pb,
  #ifdef _LZMA_IN_CB
    ILzmaInCallback *inCallback,
  #else
    unsigned char *inStream, UInt32 inSize,
  #endif
  #endif
    unsigned char *outStream, UInt32 outSize,
    UInt32 *outSizeProcessed);


 unsigned long LZMA_Decompress( void *pDst, unsigned long *pulDstLen, void *pSrc, unsigned long ulSrcLen );

#endif
