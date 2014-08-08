/******************************************************************************
 *
 *  (C)Copyright 2005 - 2008 Marvell. All Rights Reserved.
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
 *  FILENAME: xllp_dfc.c
 *
 *  PURPOSE:  XLLP interface to the data flash controller (DFC).
 *
******************************************************************************/

#include "Flash.h"
#include "xllp_dfc.h"
#include "xllp_dfc_defs.h"
#include "platform_dfc_timing.h"
#include "PlatformConfig.h"
#include "PMUA.h"

//++
//************************************************************
// Unit Test Support
#ifdef _DFC_EM
#include "em_devlib.h"
#endif
//************************************************************
//--

/*
 *  NAND Small block device ID's
 */

char Samsung[] ={ 0x71, 0x78, 0x79, 0x72,
				  0x74, 0x36, 0x76, 0x46,
				  0x56, 0x35, 0x75, 0x45,
				  0x55, 0x33, 0x73, 0x43,
				  0x53, 0x39, 0xE6, 0x49,
				  0x59, 0x00};

char ST[] = {0x73, 0x35, 0x75, 0x45,
			 0x55, 0x76, 0x36, 0x46,
			 0x56, 0x79, 0x39, 0x49,
			 0x59, 0x00};

char Micron[] = {0x00};

char Hynix[] = {0x76, 0x56, 0x36, 0x46,
				0x75, 0x55, 0x35, 0x45,
				0x73, 0x53, 0x49, 0x00};

char Toshiba[] = {0x46, 0x79, 0x75, 0x73, 0x72, 0xE6, 0x00};


/*
 * Large block NAND table
 *	This table contains the Manufacturer and Device ID's for the
 *  new MLC parts with a different Read ID formart
 */
//							Manufacturer,   Device,
char LargeBlockTable[] = {	SAMSUNG_CODE,	0xD5,
							SAMSUNG_CODE,	0xD7,
							HYNIX_CODE,		0xD5, 	//H27UAG8T2A
//	not yet known			HYNIX_CODE,		0xD7, 	//H27UCG8V5A
							0x0,			0x0};	//terminator

/*
 * Default settings:
 *   Large block:
 *       read -     CMD1 = 0x00                pg_program -     CMD1 = 0x80
 *                  CMD2 = 0x30                                 CMD2 = 0x10
 *                  ADDR_CYC = 5                                ADDR_CYC = 5
 *                  DBC = 1                                     DBC = 1
 *                  NC = 0                                      NC = 0
 *                  CMD_TYPE = 0x0                              CMD_TYPE = 0x1
 *                  CSEL = 0                                    CSEL = 0
 *                  AUTO_RS = 0                                 AUTO_RS = 0
 *        read_id - CMD1 = 0x90                blk_erase-       CMD1 = 0x60
 *                  CMD2 = 0x00                                 CMD2 = 0xD0
 *                  ADDR_CYC = 1                                ADDR_CYCLE = 0X3
 *                  DBC = 0                                     DBC = 1
 *                  NC = 0                                      NC = 0
 *                  CMD_TYPE = 0x3                              CMD_TPYE = 0X3
 *                  CSEL = 0                                    CSEL = 0
 *                  AUTO_RS = 0                                 AUTO_RS = 0
 *        read_status -     CMD1 = 0x70                         CMD2 = 0x00
 *                  ADDR_CYC = 0
 *                  DBC = 0
 *                  NC = 0
 *                  CMD_TYPE = 0x6
 *                  CSEL = 0
 *                  AUTO_RS = 0
 * Small Block
 *         read -   CMD1 = 0x00                 pg_program -     CMD1 = 0x80
 *                  CMD2 = 0x00                                  CMD2 = 0x10
 *                  ADDR_CYC = 4                                 ADDR_CYC = 5
 *                  DBC = 0                                      DBC = 1
 *                  NC = 0                                       NC = 0
 *                  CMD_TYPE = 0x0                               CMD_TYPE = 0x1
 *                  CSEL = 0                                     CSEL = 0
 *                  AUTO_RS = 0                                  AUTO_RS = 0
 *          read_id - CMD1 = 0x90                blk_erase-      CMD1 = 0x60
 *                    CMD2 = 0x00                                CMD2 = 0xD0
 *                    ADDR_CYC = 1                               ADDR_CYCLE = 0x3
 *                    DBC = 0                                    DBC = 1
 *                    NC = 0                                     NC = 0
 *                    CMD_TYPE = 0x3                             CMD_TPYE = 0x3
 *                    CSEL = 0                                   CSEL = 0
 *                    AUTO_RS = 0                                AUTO_RS = 0
 *          read_status -     CMD1 = 0x70
 *                    CMD2 = 0x00
 *                    ADDR_CYC = 0
 *                    DBC = 0
 *                    NC = 0
 *                    CMD_TYPE = 0x6
 *                    CSEL = 0
 *                    AUTO_RS = 0
 *
 *
 */

// Read		ReadID		ReadStat   Program	  Erase		Reset
static CMD_BLOCK CMD_ARRAY[DEVICE_TYPE_SIZE] =
{
//LARGE
// Read		,ReadID	   ,ReadStat  ,Program	 ,Erase		,Reset     ,ReadONFIParameter
{ 0x000D3000,0x00610090,0x00800070,0x002D1080,0x004BD060,0x00A000ff,0x100100EC},
//SMALL
// Read		,ReadID	   ,ReadStat  ,Program	 ,Erase		,Reset     ,Reserved
{ 0x00040000,0x00610090,0x00800070,0x002D1080,0x004BD060,0x00A000ff,0x00000000}
};

static unsigned int DFCSetupCount = 0;
/***********************************************************************
////////////
/////////////
//////////       D F C   I N T E R F A C E   R O U T I N E S
////////////
///////////
/***********************************************************************/
UINT_T XllpDfcInit(unsigned int width, P_NAND_Properties_T pNAND_Prop)
{
	NDCR_REG  		dfc_control_reg;
	NDTR0CS0_REG 	dfc_timing0_reg;
	NDTR1CS0_REG	dfc_timing1_reg;
	NDECCCTRL_REG   dfc_ecc_ctrl_reg;
    unsigned int  buf[2]; // Read ID data should not be more than 5 bytes.
    unsigned int Retval = 0;
	unsigned int count = 0;
	
	unsigned int OnfiSignature[2];
	int status = NoError;

	FlashBootType_T FlashBootType;

	/*
    ** Why Are we here?
	** Assume the first time, we're attempting to Boot and
	** will configure the DFC to the default timing and
	** specified bus width as passed in via the NAND properties structure.
	** If it's not the first time, but the current FlashBootType
	** is BOOT_FLASH, then adjust the bus width to that passed in (probably from 16 bit to 8 bit),
	** becasue the probe order was wrong the first time.
	** If it's not the first time, and the current FlashBootType is
	** SAVE_STATE_FLASH, don't do anyting. We're assuming that the SaveStateFlash
	** is identical to the BootFlash and the DFC doesn't need to be setup again.
	*/

	FlashBootType = GetCurrentFlashBootType();
 	/*
	 * Setup Multifunction pins for NAND
	 */

	ChipSelectDFC();

	/*
	 * Initial DFC state:
	 * See values in xllp_dfc_defs.h
	 * 	- Disable all interrupts
	 * 	- Read_ID count = 2
	 *  - SPARE and ECC enabled
	 *  - DMA disabled
	 *  - DWIDTH is x16
	 *	- Small block PAGE_SZ
	 *
	 *	Set timing to the worst (i.e. slowest) possible timing
	 *		This can be adjusted after we determine the Flash type
	 */


	dfc_control_reg.value = DFC_INITIAL_CONTROL;
	dfc_control_reg.bits.DMWIDTH_M = (width == 8)? 0 : 1;
    dfc_control_reg.bits.DMWIDTH_C = (width == 8)? 0 : 1;
	dfc_timing0_reg.value = DFC_INITIAL_TIMING0;
	dfc_timing1_reg.value = DFC_INITIAL_TIMING1;


	// Write out Timing and Control registers
	DFC_REG_WRITE(DFC_DREDEL, 0xFFFFFFFF);
	DFC_REG_WRITE(DFC_TIMING_0, dfc_timing0_reg.value);
    DFC_REG_WRITE(DFC_TIMING_1, dfc_timing1_reg.value);
	DFC_REG_WRITE(DFC_CONTROL, dfc_control_reg.value);

	//make sure BCH is off
	DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );
	dfc_ecc_ctrl_reg.bits.BCH_EN = 0;
	DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );

 	// Only issue a reset on the first call
    if (!DFCSetupCount)
	{	/*
		 * First, issue a reset command
		*/
		Retval = xdfc_reset(pNAND_Prop);
		if ( Retval != NoError )
			return Retval;

		DFCSetupCount++;
	}

	// not use ONFI anymore
    //Legacy Read ID code

	/*
	 *  Get the device ID first and check if it is small block
	 *	 or large block.  If large block go back and read all 4
	 *   bytes from the read id to get device parameters.
	 */
	 
	buf[0] = 0; buf[1] = 0;
	Retval = xdfc_readID(&buf[0], SMALL);
	CheckErrorReturn(Retval);

	if ( Retval == 0 )
	{
		pNAND_Prop->NAND_type = XllpCheckDeviceType(&buf[0], pNAND_Prop);
		/*
		*  Once we determine Large or Small block set up the flash
		*  parameters structure and adjust the DFC Control register settings
		*/
		if(pNAND_Prop->NAND_type == LARGE)
		{
			Retval = xdfc_readID(&buf[0], LARGE);
			if ( Retval != 0 )
				return Retval;
			if ((buf[0] == 0) && (buf[1] == 0))
				return DFCInitFailed;
			pNAND_Prop->device_cmds = &CMD_ARRAY[LARGE];
			XllpLoadProperties(&buf[0], pNAND_Prop);

			//dfc_control_reg.value = *DFC_CONTROL;
			DFC_REG_READ(DFC_CONTROL, dfc_control_reg.value);

			dfc_control_reg.bits.PAGE_SZ = 1;
			dfc_control_reg.bits.RA_START = 1;

			//need to figure this out from PagesPerBlock param in NAND struct
			//remember DFC funny, where 32 -> 0, 64-> 2, 128 -> 1, 256 -> 3
		    switch (pNAND_Prop->PagesPerBlock)
			{
				case(32):
					dfc_control_reg.bits.PG_PER_BLK = 0;
					break;
				case(128):
					dfc_control_reg.bits.PG_PER_BLK = 1;
					break;
				case(256):
					dfc_control_reg.bits.PG_PER_BLK = 3;
					break;
				case(64):
				default:	//default to 64, just in case
					dfc_control_reg.bits.PG_PER_BLK = 2;
					break;
			}
		}
		else	// Its Small Block
		{
			pNAND_Prop->device_cmds = &CMD_ARRAY[SMALL];
			pNAND_Prop->PageSize = 512;
			pNAND_Prop->SpareAreaSize = 16;
			//dfc_control_reg.value = *DFC_CONTROL;
			DFC_REG_READ(DFC_CONTROL, dfc_control_reg.value);

			dfc_control_reg.bits.PAGE_SZ = 0;
			dfc_control_reg.bits.PG_PER_BLK = 0;		   // Assume 32 PG_PER_BLK
			dfc_control_reg.bits.RA_START = 0;
			pNAND_Prop->BlockSize = 512 * 32;
		}

		//*DFC_CONTROL = dfc_control_reg.value;
		DFC_REG_WRITE(DFC_CONTROL, dfc_control_reg.value);
	}//End xdfc_readID retval OK
    
    /*	Overwrite Some Device Specific Data
	*	NumOfBlocks and Timing are done by the DKB/OBM only since DKB/OBM needs to support more than
	*  	what BootROM needs. BootROM does not use ScanNANDForBadBlocks routine
	*/
    #if !(BOOTROM)
    XllpDfcDeviceSpecificInit(pNAND_Prop);
    #endif
    
	return Retval;
}//End XllpDfcInit

void XllpDfcDeviceSpecificInit(P_NAND_Properties_T pNAND_Prop)
{
    FLASH_TIMING* pFT;		  // For use in timing override
    
	switch (pNAND_Prop->ManufacturerCode)
    {
		case SAMSUNG_CODE:
		{
                    if (pNAND_Prop->FlashID == 0xAA)         // LARGE Block - Part K9K2G08R0A
                    {
                        pNAND_Prop->NumOfBlocks = 2048;
                        #if !(BOOTROM)
                            pFT = &SAMSUNG_MFG_TIMING[3];                 // Override Timing
                            xdfc_setTiming(pFT);
                        #endif
                    }
					else if (pNAND_Prop->FlashID == 0xBA)		 // JIL board
					{
						pNAND_Prop->NumOfBlocks = 2048;
						#if !(BOOTROM)
					   		pFT = &SAMSUNG_MFG_TIMING[4];				  // Override Timing
					   		xdfc_setTiming(pFT);
						#endif
					}
					else if (pNAND_Prop->FlashID == 0xBC)		 // JIL 1.5 board
					{
						pNAND_Prop->NumOfBlocks = 4096;
						#if !(BOOTROM)
					   		pFT = &SAMSUNG_MFG_TIMING[4];				  // Override Timing
					   		xdfc_setTiming(pFT);
						#endif
					}
                    else    // SMALL Block - All Supported
                    {
                        pNAND_Prop->NumOfBlocks = 4096;
                        #if !(BOOTROM)
                            pFT = &SAMSUNG_MFG_TIMING[1];                 // Override Timing
                            xdfc_setTiming(pFT);
                        #endif
                    }
				break;
		}
		case TOSHIBA_CODE:
				if (pNAND_Prop->FlashID == 0xBA)          // LARGE Block
	            {
	                pNAND_Prop->NumOfBlocks = 2048;
	                pNAND_Prop->PageSize = 2048;
	            }
				else if ((pNAND_Prop->FlashID == 0xB1) || (pNAND_Prop->FlashID == 0xA1))
	            {
	                pNAND_Prop->NumOfBlocks = 1024;
	                pNAND_Prop->PageSize = 2048;
					pFT = &TOSHIBA_MFG_TIMING[0];                 // Override Timing
                    xdfc_setTiming(pFT);
	            } 
				break;
		case HYNIX_CODE:
				if (pNAND_Prop->FlashID == 0xBC)
				{
					pNAND_Prop->NumOfBlocks = 4096;
				}
				else if ((pNAND_Prop->FlashID == 0xBA) || (pNAND_Prop->FlashID == 0xAA))
				{
					pNAND_Prop->NumOfBlocks = 2048;				
				}
				else if (pNAND_Prop->FlashID == 0xA1)
				{
					pNAND_Prop->NumOfBlocks = 1024;
				}
				
				break;
		case ST_CODE:
				break;
		case MICRON_CODE:
            {
                    // Cover Parts #MT29F1GXXABA LB
                    if ((pNAND_Prop->FlashID == 0xA1) || (pNAND_Prop->FlashID == 0xB1))
                    {
                        pNAND_Prop->NumOfBlocks = 1024;
                        #if !(BOOTROM)
                            pFT = &MICRON_MFG_TIMING[1];                  // Override Timing
                            xdfc_setTiming(pFT);
                        #endif
                    }
					if(pNAND_Prop->FlashID == 0xBA)
                    {
                        pNAND_Prop->NumOfBlocks = 2048;
                        #if !(BOOTROM)
                            pFT = &MICRON_MFG_TIMING[1];                  // Override Timing
                            xdfc_setTiming(pFT);
                        #endif
                    }
					if(pNAND_Prop->FlashID == 0xBC)
                    {
                        pNAND_Prop->NumOfBlocks = 4096;
                        #if !(BOOTROM)
                            pFT = &MICRON_MFG_TIMING[2];                  // Override Timing
                            xdfc_setTiming(pFT);
                        #endif
                    }

                    break;
            }
		case EMST_CODE:
				if ((pNAND_Prop->FlashID == 0xA1) || (pNAND_Prop->FlashID == 0xB1))
	            {
	                pNAND_Prop->NumOfBlocks = 1024;
					pFT = &EMST_MFG_TIMING[0];                 // Override Timing
                    xdfc_setTiming(pFT);
	            }
				
				if (pNAND_Prop->FlashID == 0xAA)
	            {
	                pNAND_Prop->NumOfBlocks = 2048;
	            }
				
				break;
		case SPANSION_CODE:
				if (pNAND_Prop->FlashID == 0xA1)
	            {
	                pNAND_Prop->NumOfBlocks = 1024;
	            }
				break;
            default:
            //assume that there are only 1K blocks
                    pNAND_Prop->NumOfBlocks = 1024;
				break;
	 }//End switch (pNAND_Prop->ManufacturerCode)
}

DEVICE_TYPE XllpCheckDeviceType( P_DFC_BUFFER buffer, P_NAND_Properties_T pNAND_Prop)
{
    char maker, device;
    DEVICE_TYPE type = LARGE;
	int match = 0;
	char * search_list;

	maker  = pNAND_Prop->ManufacturerCode = (buffer[0]  & 0xFF);
    device = pNAND_Prop->FlashID = ((buffer[0] >> 8) & 0xFF);

	/*
	 *  Check to see if this is a known small block device
	 */
    switch (maker){
		case SAMSUNG_CODE:
				search_list = &Samsung[0];
				break;
		case TOSHIBA_CODE:
				search_list = &Toshiba[0];
				break;
		case HYNIX_CODE:
				search_list = &Hynix[0];
				break;
		case ST_CODE:
				search_list = &ST[0];
				break;
		case MICRON_CODE:
				search_list = &Micron[0];
				break;
		default:
			return (type);
	}
	while (match == 0)
	{
		if(*search_list == device)
		{
			match = 1;
			type = SMALL;
		}
		if(*search_list == 0)
			match = 1;
		search_list++;
	}

	return (type);
}

void XllpLoadProperties(P_DFC_BUFFER buffer, P_NAND_Properties_T pNAND_Prop)
{
    char maker, device, properties, planes, plane_size;
	char pg_size, blk_size, spare_size, cell_type, organization;
	int i;

    maker  = buffer[0]  & 0xFF;
	device = (buffer[0] >> 0x8) & 0xFF;
	properties = (buffer[0] >> 24) & 0xFF;
	pg_size = properties & 0x3;

	/* initialize properties using legacy Read ID format */
	//bits 2 and 3 of the 3rd ID byte tell us if its SLC or MLC
	cell_type = (buffer[0] >> 18) & 0x3;

	blk_size = (properties >> 4) & 0x3;
	organization = (properties >> 6) & 0x1;

	pNAND_Prop->PageSize = 1024 << pg_size;
	pNAND_Prop->BlockSize = (64*1024) << blk_size;


	/* now check for new MLC Read ID formats and overwrite properties if found */
	for(i = 0; ; i+=2)
	{
		//check termination case
		if(LargeBlockTable[i] == 0)
			break;

		//check for match
		if((LargeBlockTable[i] == maker) && (LargeBlockTable[i+1] == device))
		{
			pNAND_Prop->PageSize = 2048 << pg_size;
			//blk size indication bits in 4th byte: 0xB0 (bits 7, 5 and 4)
			blk_size = ((properties >> 5) & 0x4) | ((properties >> 4) & 0x3);
			pNAND_Prop->BlockSize = (128*1024) << blk_size;
			cell_type = 2; //just used to indicate MLC
			break;
		}
	}

	switch(maker)
	{
		case MICRON_CODE:
			if(cell_type == 0x0) //SLC
				pNAND_Prop->SpareAreaSize = 64;  //only documented size
			else //MLC
				if(pNAND_Prop->PageSize == 2048)
					pNAND_Prop->SpareAreaSize = 112;
				else
					pNAND_Prop->SpareAreaSize = 218;
			break;
		case SAMSUNG_CODE:
			if(cell_type != 0x0) //MLC
			{
				if((properties & 0x4) == 0x4)
					pNAND_Prop->SpareAreaSize = 128;
				if((properties & 0x8) == 0x8)
					pNAND_Prop->SpareAreaSize = 218;
				break;
			}//let SLC parts fall thru to default statement
		case TOSHIBA_CODE:
		case HYNIX_CODE:
		case ST_CODE:
		default:
			spare_size = (properties >> 2) & 0x1;
			if(spare_size == 0)
			{
				pNAND_Prop->SpareAreaSize = (pNAND_Prop->PageSize / 512) * 8;
			}
			else
			{
				pNAND_Prop->SpareAreaSize = (pNAND_Prop->PageSize / 512) * 16;
			}
	}

	//lastly, figure out how many pages there are per block
	pNAND_Prop->PagesPerBlock = pNAND_Prop->BlockSize / pNAND_Prop->PageSize;

}

