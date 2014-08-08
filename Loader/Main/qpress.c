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

/*
qpress - portable high-speed file archiver

An archive file consists of "D" and "U" characters which instruct the decompressor to traverse up and down in 
directories to create a directory three. The "F" character instructs it to create a file:

ARCHIVE =        ARCHIVEHEADER + (1 or more of UPDIR | DOWNDIR | FILE)
ARCHIVEHEADER =  "qpress10" + (ui64)(chunk size of decompressed packets)
DOWNDIR =        "D" + (ui32)(size of directory name) + (directory name) + (char)0
UPDIR =          "U"
FILE =           FILEHEADER + (0 or more of DATABLOCK) + FILETRAILER
FILEHEADER =     "F" + (ui32)(size of file name) + (file name) + (char)0
DATABLOCK =      "NEWBNEWB" + (ui64)(recovery information) + (ui32)(adler32 of compressed block) + (compressed packet)
FILETRAILER =    "ENDSENDS" + (ui64)(recovery information)

The values (ui32) and (ui64) are stored in little endian format. Example of compressing following directory structure:

FOO             directory
	c.txt       file containing "hello"
	BAR         empty directory
d.txt           file containing "there"

Two digit values are in hexadecimal, remaining values are printable and represented by their character:

          0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
000000    q   p   r   e   s   s   1   0  00  00  01  00  00  00  00  00
000010    F  05  00  00  00   c   .   t   x   t  00   N   E   W   B   N
000020    E   W   B  00  00  00  00  00  00  00  00  eb  02   %  0d   E
000030   0c  05  00  00  00  80   h   e   l   l   o   E   N   D   S   E
000040    N   D   S  00  00  00  00  00  00  00  00   D  03  00  00  00
000050    F   O   O  00   D  03  00  00  00   B   A   R  00   F  05  00
000060   00  00   d   .   t   x   t  00   N   E   W   B   N   E   W   B
000070   00  00  00  00  00  00  00  00  ef  02   Z  0d   E  0c  05  00
000080   00  00  80   t   h   e   r   e   E   N   D   S   E   N   D   S
000090   00  00  00  00  00  00  00  00   U   U

Offsets 2f - 3a and 7c - 87 are compressed packets. You see "hello" and "there" in plaintext because input is too small 
to compress.

When decompressing, decompress_directory() takes the user given destination path as argument and begins extracting 
files into it. When a DOWNDIR is read it appends its sub directory to the path. When an UPDIR is read it removes the 
deepest sub directory from the path. When a FILE is read, it calls decompress_file() which starts threads that each, in
a loop, reads a chunk, decompresses and writes to destination file:

decompress_directory() --> decompress_file() --> decompress_file_thread()
                                             +-> decompress_file_thread()
                                             +-> decompress_file_thread()

When compressing, compress_directory() searches for files in the path given as argument. For each file found, it calls 
compress_file(). When a directory is found, it appends it to the path, outputs a DOWNDIR, calls itself with the new path 
and finally outputs an UPDIR:

+-> compress_directory() +-> compress_file() --> compress_file_thread()
|                        |                   +-> compress_file_thread()
+------------------------+                   +-> compress_file_thread()                                           

*/

// Larger chunk size gives better compression ratio. However, on Windows Vista, 64 KiB or smaller
// improves parallel disk I/O when data is both read and written.
#define DEFAULT_COMPRESS_CHUNK_SIZE (64*1024)

// Data compressed with a specific chunk size requires 2 * (chunk size) * (thread count) memory to
// decompress (32 MiB requires 128 MiB with the default 2 threads), so we should limit it for 
// archives to be able to decompress on systems with little memory
#define MAX_COMPRESS_CHUNK_SIZE (32*1024*1024)

// How much qlz_compress() can expand input data with (some incompressible data may grow in size). This
// is 400 bytes regardless of input size, even for gigabyte range.
#define QLZ_SIZE_OVERHEAD 400


#if QLZ_STREAMING_BUFFER != 0
#error QLZ_STREAMING_BUFFER must be 0
#endif

#include "quicklz.h"
#include "Typedef.h"
#include "loadoffsets.h"

// command line flags

unsigned long long compress_chunk_size = DEFAULT_COMPRESS_CHUNK_SIZE;

// QuickLZ scratch buffers
UINT8_T *scratch = (UINT_T *)DDR_QPRESS_DDR;

UINT_T adler(UINT8_T *data, size_t len, UINT_T crc) 
{
    UINT_T a = crc >> 16;
    UINT_T b = crc & 0xffff;
    while (len > 0)
    {
        size_t tlen = len > 5550 ? 5550 : len;
        len -= tlen;
        do 
        {
            a += *data++;
            b += a;
        } while (--tlen);
 
        a %= 65521;
        b %= 65521;
    } 
    return (b << 16) | a;
}

UINT_T quickLZ_decompress(UINT8_T *pSrc, UINT8_T *pDest)
{
	UINT8_T compressed_length[9], flag;
	UINT_T length, chunk_start, chunk_length, qlz_image_length;
	UINT64 my_chunk;
	UINT_T crc_r, crc_original;
	size_t decomp_size;
	UINT8_T temp[4], i;
	UINT8_T end_of_file = FALSE;

	p_qlz_header p_QLZD_h;
	p_qlz_image p_QLZD_i;
	p_qlz_chunk_header p_QLZDC_h;

	p_QLZD_h = (p_qlz_header)pSrc;

	if ((memcmp(p_QLZD_h->qlz_string, "qpress10", 1) != 0) || (p_QLZD_h->qlz_chunk_size != 0x10000))
		return QPRESS_ERROR_HEADER_FORMAT;

	p_QLZD_i = (p_qlz_image)(pSrc + sizeof(qlz_header));

	if (p_QLZD_i->qlz_image[0] != 'F')
		return QPRESS_ERROR_IMAGE_FORMAT;

	for (i = 0; i < 4; i++)
	{
		temp[i] = p_QLZD_i->qlz_image[i + 1];
	}

	length = ((temp[0]) | (temp[1] << 8) | (temp[2] << 16) | (temp[3] << 24));
	qlz_image_length = 1 + 4 + length + 1;
	p_QLZDC_h = (p_qlz_chunk_header)((UINT8_T *)p_QLZD_i + qlz_image_length);

	for(;;)
	{

		if (end_of_file)
		{
		    break;
		}

		if (p_QLZDC_h->qlz_chunk[0] == 'N') // NEWBNEWB
		{
		    ;
		}
		else if(p_QLZDC_h->qlz_chunk[0] == 'E') // ENDSENDS
		{
		    end_of_file = TRUE;
		    break;
		}
		else
		{
			serial_outstr("Unsupport flag\n");
			return QPRESS_NOT_SUPPORT_FLAG;
		}

		for (i = 0; i < 4; i++)
		{
			temp[i] = p_QLZDC_h->qlz_chunk[16 + i];
		}

		crc_original = ((temp[0]) | (temp[1] << 8) | (temp[2] << 16) | (temp[3] << 24));
		if (qlz_size_compressed(&p_QLZDC_h->qlz_chunk[20]) > compress_chunk_size + QLZ_SIZE_OVERHEAD)
			return QPRESS_DATA_ERROR;

		//chunk_start = (UINT_T)((UINT8_T *)p_QLZDC_h + sizeof(qlz_decompressed_chunk_header));
		chunk_start = (UINT_T)((UINT8_T *)p_QLZDC_h + 20);
		chunk_length = qlz_size_compressed(&p_QLZDC_h->qlz_chunk[20]);

		crc_r = adler(chunk_start, chunk_length, 0x00010000);
		if (crc_r != crc_original)
		{
		    return QPRESS_DATA_CRC_ERROR;
		}

		decomp_size = qlz_decompress(chunk_start, pDest, scratch);

		chunk_start += chunk_length;
		pDest += decomp_size;
		p_QLZDC_h = (p_qlz_chunk_header)((UINT8_T *)p_QLZDC_h + 20 + chunk_length);
	}

	return NoError;
}

