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
 
 ******************************************************************************/#ifndef __usbPal_h__
#define __usbPal_h__ 1

//#include "mv88w8xx8.h"
//#include "misc.h"     // for memcpy
#include "tim.h"
#include "Typedef.h"
#include "misc_prototypes.h"
#include "Errors.h"



#define _PTR_      *
#define _CODE_PTR_ *

typedef char _PTR_                    char_ptr;    /* signed character       */

typedef signed   char  int_8, _PTR_   int_8_ptr;   /* 8-bit signed integer   */
typedef unsigned char  uint_8, _PTR_   uint_8_ptr;  /* 8-bit signed integer   */

typedef          short int_16, _PTR_   int_16_ptr;  /* 16-bit signed integer  */
typedef unsigned short uint_16, _PTR_  uint_16_ptr; /* 16-bit unsigned integer*/

typedef          int   int_32, _PTR_   int_32_ptr;  /* 32-bit signed integer  */
typedef unsigned int   uint_32, _PTR_  uint_32_ptr; /* 32-bit unsigned integer*/

typedef unsigned long  boolean;  /* Machine representation of a boolean */

typedef void _PTR_     pointer;  /* Machine representation of a pointer */

/*--------------------------------------------------------------------------*/
/*
**                          STANDARD CONSTANTS
**
**  Note that if standard 'C' library files are included after types.h,
**  the defines of TRUE, FALSE and NULL may sometimes conflict, as most
**  standard library files do not check for previous definitions.
*/

#ifndef _ASSERT_
   #define ASSERT(X,Y)
#else
   #define ASSERT(X,Y) if(Y) { USB_printf(X); exit(1);}
#endif

#ifndef  MIN
#   define MIN(a,b)   ((a) < (b) ? (a) : (b))
#endif

//#define USB_MEM_ALIGN(n, align)            ((n) + (-(n) & (align-1)))

/* Macros for aligning the EP queue head to 32 byte boundary */
#define USB_MEM2_ALIGN(n) USB_MEM_ALIGN( n, 2)
#define USB_MEM4_ALIGN(n) USB_MEM_ALIGN( n, 4)
#define USB_MEM8_ALIGN(n) USB_MEM_ALIGN( n, 8)
#define USB_MEM16_ALIGN(n) USB_MEM_ALIGN( n, 16)
#define USB_MEM32_ALIGN(n) USB_MEM_ALIGN( n, 32)
#define USB_MEM64_ALIGN(n) USB_MEM_ALIGN( n, 64)
#define USB_MEM128_ALIGN(n) USB_MEM_ALIGN( n, 128)
#define USB_MEM256_ALIGN(n) USB_MEM_ALIGN( n, 256)
#define USB_MEM512_ALIGN(n) USB_MEM_ALIGN( n, 512)
#define USB_MEM1024_ALIGN(n) USB_MEM_ALIGN( n, 1024)
#define USB_MEM2048_ALIGN(n) USB_MEM_ALIGN( n, 2048)
#define USB_MEM4096_ALIGN(n) USB_MEM_ALIGN( n, 4096)

#define PSP_CACHE_LINE_SIZE                 32
#define USB_CACHE_ALIGN(n)                  USB_MEM_ALIGN(n, PSP_CACHE_LINE_SIZE)

#define USB_uint_16_low(x)                  ((x) & 0xFF)
#define USB_uint_16_high(x)                 (((x) >> 8) & 0xFF)


#ifndef INLINE
#   if defined(MV_VXWORKS)
#       define INLINE   __inline
#   else
#       define INLINE   inline
#   endif /* MV_VXWORKS */
#endif /* INLINE */

/* 16bit byte swap. For example 0x1122 -> 0x2211                            */
static uint_16 USB_BYTE_SWAP_16BIT(uint_16 value)
{
    return ( ((value & 0x00ff) << 8) |
             ((value & 0xff00) >> 8) );
}

/* 32bit byte swap. For example 0x11223344 -> 0x44332211                    */
static uint_32 USB_BYTE_SWAP_32BIT(uint_32 value)
{
    return ( ((value & 0x000000ff) << 24) |
             ((value & 0x0000ff00) << 8)  |
             ((value & 0x00ff0000) >> 8)  |
             ((value & 0xff000000) >> 24));
}


/* Endianess macros.                                                        */
#define USB_16BIT_LE(X)  (X)
#define USB_32BIT_LE(X)  (X)

#define MV_USB_REG_WRITE(rEG,dATA)  *(volatile uint_32 *)(rEG) = dATA
#define MV_USB_REG_WRITE8(rEG,dATA) ((*(volatile uint_8 *)(rEG)) = dATA & 0xff )
#define MV_USB_REG_READ(rEG)        *(volatile uint_32 *)(rEG)




#define USB_BASE    						0xD4208000  // TTC base address
#define USB_CORE_CAP_LENGTH_REG          	(USB_BASE + 0x100)
#define PU_REF_REG							0xd4207004
#define ANA_GRP_BIT_POSITION				20



#define USB_sprintf(frmt, x...)
#if defined MV_USB_TRACE_PRINT
    #define USB_printf(frmt, x...)      armprintf(frmt, ##x)
#else
    #define USB_printf(frmt, x...)
#endif /* MV_USB_TRACE_PRINT */
#define USB_virt_to_phys(pVirt)     (uint_32)(pVirt)
#define USB_get_cap_reg_addr(dev)     _bsp_get_usb_capability_register_base(dev)
#define USB_uncached_memalloc(size, pPhyAddr)   K_malloc(size)
#define USB_uncached_memfree(pVirt, size, physAddr)  K_free(pVirt)
#define USB_memalloc(size)          K_malloc(size)
#define USB_memfree(ptr)            K_free(ptr)
#define USB_memzero(ptr, n)         memset(ptr, 0, n)
#define USB_memcopy(src, dst, n)    memcpy(dst, src, n)
#define USB_dcache_inv(ptr, size)
#define USB_dcache_flush(ptr, size)

// disable this functionality for Bootrom at present
//--------------------------------------------------
#if 0
  #define USB_lock()                  disableInterrupts();
  #define USB_unlock(key)             restoreInterrupts(key);
#else
  #define USB_lock() 1
  #define USB_unlock(x) x
#endif

// implemented in arm.c/s
void *K_malloc(int size);
void K_free(void *block);
void K_initDynamicPool();
pointer _bsp_get_usb_capability_register_base(uint_8);

void restoreInterrupts(unsigned long ir);
unsigned long disableInterrupts();

void bspUsbHWInit();
void bspEnableUsbInterrupt();
void bspDisableUsbInterrupt();
uint_32 USB_MEM_ALIGN(  uint_32 memLoc, uint_32 alignSize);
#if defined MV_USB_TRACE_PRINT
int armprintf(const char *fmt, ...);
#endif /* MV_USB_TRACE_PRINT */


#endif /* __usbPal_h__ */
/* EOF */


