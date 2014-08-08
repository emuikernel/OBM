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

// mcu_extras.h:
// these are definitions that should have been in mcu.h.
// remove these from here if they appear in updates to mcu.h


#ifndef __MCU_EXTRAS__
#define __MCU_EXTRAS__

#include "MCU.h"

// FIXME: missing from mcu.h:

#define MCU_BASE 0xB0000000
#define	MCU_SDRAM_CONFIG_2			0x0040	/* SDRAM Config	Register 2  */
#define	MCU_PHY_CONTROL_11			0x0210	/* PHY Control Register 11 */
#define	MCU_SDRAM_CONTROL_6			0x0760	/* SDRAM Control Register 6 */
#define	MCU_SDRAM_CONTROL_7			0x0770	/* SDRAM Control Register 7 */
#define MCU_SDRAM_PAD_CALIBRATION	0x00A0	/* SDRAM Pad Calibration Register */
#define	MCU_MMAP2					0x0130	/* Memory Address Map  Register 2 */
#define	MCU_MCB_CONTROL_3			0x0530	/* MCB Control Register 3 */
#define	MCU_SDRAM_CONTROL_14		0x07e0	/* SDRAM Control Register 14 */
#define	MCU_PHY_DLL_CONTROL_2		0x0E20	/* PHY DLL Control Register 2 */
#define	MCU_PHY_DLL_CONTROL_3		0x0E30	/* PHY DLL Control Register 3 */

// end of missing from mch.h

#define MCU_REG_SDRAM_CONFIG_0				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONFIG_0         ))
#define MCU_REG_SDRAM_CONFIG_1				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONFIG_1         ))
#define MCU_REG_SDRAM_CONFIG_2				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONFIG_2         ))
#define MCU_REG_SDRAM_TIMING_1				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_TIMING_1         ))
#define MCU_REG_SDRAM_TIMING_2				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_TIMING_2         ))
#define MCU_REG_SDRAM_CONTROL_1				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONTROL_1        ))
#define MCU_REG_SDRAM_CONTROL_2				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONTROL_2        ))
#define	MCU_REG_SDRAM_CONTROL_14			((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONTROL_14		  ))
#define MCU_REG_PHY_CONTROL_3				((unsigned volatile long*)( MCU_BASE + MCU_PHY_CONTROL_3          ))
#define MCU_REG_SDRAM_TIMING_3				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_TIMING_3         ))
#define MCU_REG_SDRAM_CONTROL_3				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONTROL_3        ))
#define MCU_REG_SDRAM_CONTROL_4				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONTROL_4        ))
#define MCU_REG_SDRAM_TIMING_4				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_TIMING_4         ))
#define MCU_REG_PHY_CONTROL_7				((unsigned volatile long*)( MCU_BASE + MCU_PHY_CONTROL_7          ))
#define MCU_REG_PHY_CONTROL_8				((unsigned volatile long*)( MCU_BASE + MCU_PHY_CONTROL_8          ))
#define MCU_REG_PHY_CONTROL_9				((unsigned volatile long*)( MCU_BASE + MCU_PHY_CONTROL_9          ))
#define	MCU_REG_PHY_CONTROL_10				((unsigned volatile long*)( MCU_BASE + MCU_PHY_CONTROL_10         ))
#define MCU_REG_PHY_CONTROL_11				((unsigned volatile long*)( MCU_BASE + MCU_PHY_CONTROL_11         ))
#define MCU_REG_PHY_CONTROL_13				((unsigned volatile long*)( MCU_BASE + MCU_PHY_CONTROL_13         ))
#define MCU_REG_PHY_CONTROL_14				((unsigned volatile long*)( MCU_BASE + MCU_PHY_CONTROL_14         ))
#define MCU_REG_SDRAM_CONTROL_5				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONTROL_5        ))
#define MCU_REG_MCB_CONTROL_1				((unsigned volatile long*)( MCU_BASE + MCU_MCB_CONTROL_1          ))
#define MCU_REG_MCB_CONTROL_2				((unsigned volatile long*)( MCU_BASE + MCU_MCB_CONTROL_2		  ))
#define MCU_REG_MCB_CONTROL_3				((unsigned volatile long*)( MCU_BASE + MCU_MCB_CONTROL_3		  ))
#define MCU_REG_MCB_CONTROL_4				((unsigned volatile long*)( MCU_BASE + MCU_MCB_CONTROL_4		  ))
#define MCU_REG_SDRAM_TIMING_5				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_TIMING_5         ))
#define MCU_REG_SDRAM_CONTROL_6				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONTROL_6        ))
#define MCU_REG_SDRAM_CONTROL_7				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_CONTROL_7        ))
#define MCU_REG_PHY_DLL_CONTROL_1			((unsigned volatile long*)( MCU_BASE + MCU_PHY_DLL_CONTROL_1      ))
#define MCU_REG_PHY_DLL_CONTROL_2			((unsigned volatile long*)( MCU_BASE + MCU_PHY_DLL_CONTROL_2      ))
#define MCU_REG_PHY_DLL_CONTROL_3			((unsigned volatile long*)( MCU_BASE + MCU_PHY_DLL_CONTROL_3      ))
#define MCU_REG_MMU_MMAP0					((unsigned volatile long*)( MCU_BASE + MCU_MMAP0                  ))
#define MCU_REG_MMU_MMAP1					((unsigned volatile long*)( MCU_BASE + MCU_MMAP1                  ))
#define	MCU_REG_MMU_MMAP2					((unsigned volatile long*)( MCU_BASE + MCU_MMAP2				  ))
#define MCU_REG_USER_INITIATED_COMMAND		((unsigned volatile long*)( MCU_BASE + MCU_USER_INITIATED_COMMAND ))
#define	MCU_REG_DRAM_STATUS					((unsigned volatile long*)( MCU_BASE + MCU_DRAM_STATUS            ))

#define	MCU_REG_DECODE_ADDR			((unsigned volatile long*)( MCU_BASE + MCU_CONFIG_REG_DECODE_ADDR ))
#define MCU_REG_PAD_CAL				((unsigned volatile long*)( MCU_BASE + MCU_SDRAM_PAD_CALIBRATION ))

// end of extra defines

//#define TTC_DEFAULT_DCLK					156000000
//#define TTC_DEFAULT_FCLK					156000000		// for refresh cycle calculations


#endif
