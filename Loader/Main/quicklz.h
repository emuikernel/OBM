#ifndef QLZ_HEADER
#define QLZ_HEADER

/******************************************************************************
 *
 *  (C)Copyright 2013 Marvell. All Rights Reserved.
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

// You can edit following user settings. Data must be decompressed with the same 
// setting of QLZ_COMPRESSION_LEVEL and QLZ_STREAMING_BUFFER as it was compressed
// (see manual). If QLZ_STREAMING_BUFFER > 0, scratch buffers must be initially
// zeroed out (see manual). First #ifndef makes it possible to define settings from 
// the outside like the compiler command line or from higher level code.

// Version 1.4.1 final - april 2010

#ifndef QLZ_COMPRESSION_LEVEL
	//#define QLZ_COMPRESSION_LEVEL 1
	//#define QLZ_COMPRESSION_LEVEL 2
	#define QLZ_COMPRESSION_LEVEL 3

	#define QLZ_STREAMING_BUFFER 0
	//#define QLZ_STREAMING_BUFFER 100000
	//#define QLZ_STREAMING_BUFFER 1000000

	//#define QLZ_MEMORY_SAFE
#endif

#define QLZ_VERSION_MAJOR 1
#define QLZ_VERSION_MINOR 4
#define QLZ_VERSION_REVISION 1

// Using size_t, memset() and memcpy()
#include "misc.h"
#include "Typedef.h"

typedef unsigned int            size_t;

// Public functions of QuickLZ
size_t qlz_size_decompressed(const char *source);
size_t qlz_size_compressed(const char *source);
size_t qlz_decompress(const char *source, void *destination, char *scratch_decompress);
size_t qlz_compress(const void *source, char *destination, size_t size, char *scratch_compress);
int qlz_get_setting(int setting);

// Verify compression level
#if QLZ_COMPRESSION_LEVEL != 1 && QLZ_COMPRESSION_LEVEL != 2 && QLZ_COMPRESSION_LEVEL != 3
#error QLZ_COMPRESSION_LEVEL must be 1, 2 or 3
#endif

// Compute QLZ_SCRATCH_COMPRESS and QLZ_SCRATCH_DECOMPRESS
#if QLZ_COMPRESSION_LEVEL == 1
#define QLZ_POINTERS 1
#define QLZ_HASH_VALUES 4096
#elif QLZ_COMPRESSION_LEVEL == 2
#define QLZ_POINTERS 4
#define QLZ_HASH_VALUES 2048
#elif QLZ_COMPRESSION_LEVEL == 3
#define QLZ_POINTERS 16
#define QLZ_HASH_VALUES 4096
#endif

typedef struct 
{
#if QLZ_COMPRESSION_LEVEL == 1
	unsigned int cache[QLZ_POINTERS];
#endif
	const unsigned char *offset[QLZ_POINTERS];
} qlz_hash_compress;

typedef struct 
{
	const unsigned char *offset[QLZ_POINTERS];
} qlz_hash_decompress;

typedef struct
{
	UINT8_T qlz_string[8]; // "qpress10"
	UINT64	qlz_chunk_size; // must be 64KB
}qlz_header, *p_qlz_header;

typedef struct
{
	UINT8_T qlz_image[5];
}qlz_image, *p_qlz_image;

typedef struct
{
	UINT8_T qlz_chunk[29];
	//UINT8_T qlz_chunk_string1; // "N"
	//UINT8_T qlz_chunk_string2[7]; // "EWBNEWB"
	//UINT8_T	qlz_reserved[8]; // reserved as 0
	//UINT_T qlz_chunk_crc; // crc
	//UINT8_T qlz_chunk_size[9]; // chunk size
}qlz_chunk_header, *p_qlz_chunk_header;


#define QLZ_ALIGNMENT_PADD 8
#define QLZ_BUFFER_COUNTER 8

#define QLZ_SCRATCH_COMPRESS (QLZ_ALIGNMENT_PADD + QLZ_BUFFER_COUNTER + QLZ_STREAMING_BUFFER + sizeof(qlz_hash_compress[QLZ_HASH_VALUES]) + QLZ_HASH_VALUES)

#if QLZ_COMPRESSION_LEVEL < 3
	#define QLZ_SCRATCH_DECOMPRESS (QLZ_ALIGNMENT_PADD + QLZ_BUFFER_COUNTER + QLZ_STREAMING_BUFFER + sizeof(qlz_hash_decompress[QLZ_HASH_VALUES]) + QLZ_HASH_VALUES)
#else
	#define QLZ_SCRATCH_DECOMPRESS (QLZ_ALIGNMENT_PADD + QLZ_BUFFER_COUNTER + QLZ_STREAMING_BUFFER)
#endif

#endif





