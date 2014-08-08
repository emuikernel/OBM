
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
 *
 *
 *  FILENAME:	DDR_Init.c
 *
 *  PURPOSE: 	Contain code to handle DDR configuration from 
 *				op codes contained in the TIM header.
 *
******************************************************************************/

#include "DDR_Cfg.h"
#if BOOTROM
#include "bootrom.h"
#else
#include "BootLoader.h"
#endif


// CheckMemoryReliability:
// Do a simple read/modify/write/read-back/validate across the specified range.
// Input:
//	startaddr: address to start checking
//	length:    number of bytes to check. 
//  faultaddr: address of any read-mismatch
// Return value:
//   0  = no errors
//   !0 = error
unsigned long
CheckMemoryReliability(
	unsigned long	startaddr,
	unsigned long	length,
	unsigned long	*faultaddr
	)
{
	unsigned long			original, modified, readverify;
	volatile unsigned long *pMem;

	pMem = (volatile unsigned long*)startaddr;
	while(length)
	{
		original=*pMem;					// get what's currently there.
		modified=original^0xffffffff;	// invert every bit
		*pMem = modified;				// write it back
		readverify=*pMem;				// read it back for comparison with what was written.

		if(readverify!=modified)
		{
			*faultaddr = (unsigned long)pMem;
			return DDR_MemoryTest_Failed;					// error code
		}

		++pMem;							// increases by sizeof(unsigned long)
		length-=sizeof(unsigned long);
	}
	return 0;
}

//
//ConfigureDDRMemeory routine
//
//	 This routine will process a ddr configuration package from the TIM header
//
//
//
//
unsigned int ConfigureDDRMemory(pDDR_PACKAGE DDR_PID){
	DDR_OPS_S Operations;
	pDDR_OPS_S pOperations;
	pINSTRUCTION_S pDDR_Instructions;
	unsigned int Retval = NoError;
	unsigned long faultaddr;
	unsigned int *ptemp = NULL;

   	pOperations = &Operations;
	pDDR_Instructions = NULL;
	memset(pOperations, 0, sizeof( DDR_OPS_S ));
	 
	if( DDR_PID == NULL )
		return GeneralError;
		 
	if(((DDR_PID->WRAH.Identifier & 0xFFFFFF00 )>> 8) !=  DDRTYPE)
		return UnknownReservedPackage; 	

	if( (DDR_PID->WRAH.Identifier ==	DDRID)  ||
		(DDR_PID->WRAH.Identifier ==	DDRGID) || 
		(DDR_PID->WRAH.Identifier ==	DDRTID) ||
		(DDR_PID->WRAH.Identifier ==	DDRCID))
		return DDR_Package_Obsolete;

	//Get pointer to start of instructions array
	ptemp = (unsigned int *)(&DDR_PID->DDR_Operations);
	ptemp += (DDR_PID->NumberOperations * 2);
	pDDR_Instructions = (pINSTRUCTION_S)ptemp; 
    Retval = ProcessDDROps(&DDR_PID->DDR_Operations, DDR_PID->NumberOperations, pOperations);
	// Check some defaults here in case they are not set by user
	if(	pOperations->Loops <= 0)
		pOperations->Loops = 1;
	// allow for a 64MB window (BIT26) for mem testing default to 2K (BIT11), if not set by user
	if(	pOperations->MemTest == 1 ){
		if ( (pOperations->MemTestStartAddr < DDR_PHY_ADDR) || 
			 (pOperations->MemTestStartAddr > DDR_PHY_ADDR + BIT26))
			  pOperations->MemTestStartAddr =  DDR_PHY_ADDR; 
		if ( (pOperations->MemTestSize + pOperations->MemTestStartAddr ) > (DDR_PHY_ADDR + BIT26)||
			  (pOperations->MemTestSize <= 0 ))
			  pOperations->MemTestSize =  BIT11; 
	 }		  	

	if (pOperations->Init == 1 )
	{
		// Start loop to intialize DDR Memory
		while( Operations.Loops ){
		   	Retval = ProcessInstructions(pDDR_Instructions, DDR_PID->NumberInstructions, Operations.IgnoreTimeOuts);
			if(Retval == NoError){
				// DDR OK, Check if we do a Memory Test
			  	if( Operations.MemTest ){
			 		// Memory Test Enabled
			 		Retval = CheckMemoryReliability( (unsigned long)Operations.MemTestStartAddr, Operations.MemTestSize, &faultaddr);
					if(Retval != NoError){
				  		// Memory Test failed, try again
				  		Operations.Loops--;
				  		continue;
			  		}
			  		else {
					  	// Memory Test OK
						Operations.Loops = 0;
					  	break;
					}
				}
				else {
					// Memory Test Disabled
					Operations.Loops = 0;
					break;	
				}
			}
			else {
				// DDR Init Error - Exit with error code
				// returned by ProcessInstructions()above
				Operations.Loops=0;
				break;
			}//End DDR Init
		}//Endwhile
	}
	else
	{
	   Retval = DDR_InitDisabled;
	}  
  	return Retval;
}
		
//
//ProcessDDROps routine
//
//	 This routine will process a ddr operations structure in the TIM
//
//
//
//
unsigned int ProcessDDROps(pDDR_OPERATION pDDR_OPS, unsigned int Num, pDDR_OPS_S Operations){
	unsigned int i;
	unsigned int Retval = NoError;

	if(pDDR_OPS == NULL)
		return NULLPointer; 
	for(i = 0; i < Num; i++){
		switch(pDDR_OPS->OpId){
			case DDR_NOP:
				break;
			case DDR_INIT_ENABLE:
				 Operations->Init =  pDDR_OPS->OpValue;				
				 break;
			case DDR_MEMTEST_ENABLE:
				 Operations->MemTest =  pDDR_OPS->OpValue;				
				break;
			case DDR_MEMTEST_START_ADDR:
				 Operations->MemTestStartAddr =  pDDR_OPS->OpValue;				
				break;
			case DDR_MEMTEST_SIZE:
				 Operations->MemTestSize =  pDDR_OPS->OpValue;				
				break;
			case DDR_INIT_LOOP_COUNT:
				 Operations->Loops =  pDDR_OPS->OpValue;				
				break;
			case DDR_IGNORE_INST_TO:
				 Operations->IgnoreTimeOuts =  pDDR_OPS->OpValue;				
				break;
 			default:
				Retval = DDR_Unknown_Operation; 
		 }
		 if (Retval != NoError)
			break;
		 pDDR_OPS++;		
	
	}	
	return Retval;
}
/* This routine checks to see if a DDR entry is present in the Reserved Area of the TIM
*  If present it will attempt to configure it.
*/

unsigned int CheckAndConfigureDDR(pTIM pTIM_h, pFUSE_SET pFuses)
{
	UINT_T Retval = NoError;
	pWTP_RESERVED_AREA_HEADER pWRAH = NULL;

	pCIDP_ENTRY  MyConsumerEntry = NULL;
	unsigned int *pPackageID = NULL;
	int count;
    
    pFuses->bits.DDRInitialized = FALSE; // Initializing to false;

	// first check for the new CIDP entry
	MyConsumerEntry = FindMyConsumerArray(pTIM_h, MY_CONSUMER_ID);
	if(MyConsumerEntry != NULL)
	{
	   count = MyConsumerEntry->NumPackagesToConsume;
	   pPackageID = (unsigned int *)(&MyConsumerEntry->PackageIdentifierList);	
	   while(count)
	   {
	   		if (((*pPackageID & TYPEMASK)>> 8) == DDRTYPE )
	   		{
				pWRAH = FindPackageInReserved (&Retval, pTIM_h, *pPackageID);
				if (Retval != NoError)
					return DDR_PackageNotFound;
				Retval = ConfigureDDRMemory((pDDR_PACKAGE) pWRAH);
	   			if (Retval == NoError)
				{
					pFuses->bits.DDRInitialized = TRUE;
					count = 0;
					break;
				}
				else
				{
					// Errors that are not real errors
					if (Retval == DDR_InitDisabled)
					{
						// This was requested in the DDR package so it is not an error
						// pFuses->bits.DDRInitialized = FALSE (already set as default value above)
						Retval = NoError;
						count = 0;
						break;
					}//Endif

				}//Endifelse
			}//Endif
			pPackageID++;	// Increment to the next package!
			count --; // Even if we do not find the package type decrement count to avoid infinite loop
	   }//Endwhile count
	}
    
    if (Retval == DDR_NotInitializedError) // Disabling DDR Initialization through TIM is not an Error.
        Retval = NoError;
    
	return Retval;
}

unsigned long ReadBitField
(
	unsigned volatile long * pReg,		
	unsigned long bf_lsb,		// lsb location of bit field
	unsigned long bf_len		// number of bits in the fields
)
{
	unsigned long	val;
	
	val = *pReg;
	val = val<<(32-(bf_lsb+bf_len));
	val = val>>(32-bf_len);

	return val;
}

UINT_T GetDDRBaseAddress()
{
    // This supports CS0 only
	UINT_T baseAddress;
	
	// Read Memory Address Map Register 0 (MMU_MMAP0)- CS0 (offset 0x010)
	// Keep the upper 9 bits (Start Address)
   	baseAddress = (*(unsigned volatile long *)MCU_REG_MMU_MMAP0)& MMU_MMAP0_START_ADDRESS_MSK;
	return baseAddress;
}

UINT_T GetDDRSize()
{
    // This supports CS0 only
	UINT_T size;
	unsigned volatile long * pReg;
	UINT_T bfs;		// lsb location of bit field
	UINT_T bfl;		// number of bits in the fields
	// Read Memory Address Map Register 0 - CS0 (offset 0x010)
    // (MEMORY_ADDRESS_MAP)
    pReg = (unsigned volatile long *)MCU_REG_MMU_MMAP0;
	bfs = 16;
	bfl = 4;
	size = ReadBitField(pReg, bfs, bfl);
    switch(size)
	{
		case(0xF): {size = 0x80000000; break;} //(UINT)(2048*(1024*1024))
		case(0xE): {size = 1024*(1024*1024); break;}
		case(0xD): {size =  512*(1024*1024); break;}
		case(0xC): {size =  256*(1024*1024); break;}
		case(0xB): {size =  128*(1024*1024); serial_outstr("DDR size: 128MB\n");break;}
		case(0xA): {size =   64*(1024*1024); serial_outstr("DDR size: 64MB\n");break;}
		case(0x9): {size =   32*(1024*1024); serial_outstr("DDR size: 32MB\n");break;}
		case(0x8): {size =   16*(1024*1024); break;}
		case(0x7): {size =    8*(1024*1024); break;}
		default:   {size =    0; break;}
	}
    return size;
}


