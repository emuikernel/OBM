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
 *  FILENAME:	RegInstructions.c
 *
 *  PURPOSE: 	Contain code to handle register manipulations from 
 *				op codes contained in the TIM header.
 *
******************************************************************************/

#include "RegInstructions.h"



unsigned int ScratchMemoryArray[MAX_SCRATCH_MEMORY_ID];



int ValidAddress(UINT_T Address){

	return TRUE;
} 

// Get a pointer to Scratch Memory
UINT_T* GetSMPtr(UINT_T ScratchMemoryId)
{
	UINT_T* Retptr = NULL;
	if (ScratchMemoryId <= MAX_SCRATCH_MEMORY_ID)
	{
		Retptr = &ScratchMemoryArray[ScratchMemoryId];
	}
	return Retptr; 
}

void WriteInstruction(UINT_T Address, UINT_T Value){
	
	if( ValidAddress(Address))
		*(UINT_T*)Address = Value;
	return;

}

void ReadInstruction(UINT_T Address, UINT_T NumReads){
	unsigned int bitbucket;
	if( ValidAddress(Address)){
		while(NumReads){
			bitbucket = *(volatile UINT_T*)Address;
			NumReads--;
		}
	}
 	return;
}


//
//	This routine will poll the address and check to see if the bit(s) as specified by Mask are set.
//
//		Address - register address to poll
//		Mask	- bit(s) to check
//		Timeout - time out value in milli seconds
//		IgnoreTimeoutFlag - if set, function returns NoError even if a timeout occurs

UINT_T  WaitForBitSetInstruction(UINT_T Address, UINT_T Mask, UINT_T Timeout,UINT_T IgnoreTimeoutFlag){
	unsigned int bitbucket;
	unsigned int Retval = NoError;
	if( ValidAddress(Address))
	{
	   	// Wait for (Address & Mask) == Value
 		Retval = WaitForOperationComplete(Timeout, // Timeout in msec 
 										WAITINMILLISECONDS, 
 										(volatile UINT_T*)Address, 
 										Mask, // Value 
 										Mask // Mask
 										);		   
	   	if((Retval == NoError) || (IgnoreTimeoutFlag !=0))
			{Retval = NoError;}
		else
			{Retval = INSTR_Timeout;}
	}
 	return Retval;
}

//
//	This routine will poll the address and check to see if the bit(s) as specified by Mask are zero.
//
//		Address - register address to poll
//		Mask	- bit(s) to check
//		Timeout - time out value in milli seconds
//		IgnoreTimeoutFlag - if set, function returns NoError even if a timeout occurs

UINT_T WaitForBitClearInstruction(UINT_T Address, UINT_T Mask, UINT_T Timeout,UINT_T IgnoreTimeoutFlag){
	unsigned int bitbucket; 
	unsigned int Retval = NoError;
	if( ValidAddress(Address))
	{
	   	// Wait for (Address & Mask) == 0
 		Retval = WaitForOperationComplete(Timeout, // Timeout in msec 
 										WAITINMILLISECONDS, 
 										(volatile UINT_T*)Address, 
 										0,    // Value 
 										Mask // Mask
 										);		   
	   	if((Retval == NoError) || (IgnoreTimeoutFlag !=0))
			{Retval = NoError;}
		else
			{Retval = INSTR_Timeout;}
 	}
 	return Retval;
}

void AndInstruction(UINT_T Address, UINT_T Value)
{
	unsigned int RegVal;
	if( ValidAddress(Address))
	{
			RegVal = *(volatile UINT_T*)Address;
			RegVal = RegVal & Value;
			*(volatile UINT_T*)Address = RegVal;
	}
 	return;
}

void OrInstruction(UINT_T Address, UINT_T Value)
{
	unsigned int RegVal;
	if( ValidAddress(Address))
	{
			RegVal = *(volatile UINT_T*)Address;
			RegVal = RegVal | Value;
			*(volatile UINT_T*)Address = RegVal;
	}
 	return;
}
//----------------------------------------------------------------------------
UINT_T Set_Bitfield_Instruction (UINT_T Address, UINT_T Mask, UINT_T Value)
{
	UINT_T Temp;
	UINT_T Retval = NoError;
	if( ValidAddress(Address))
	{
			Temp = *(volatile UINT_T*)Address;
			// Clear bitfield, retain all other bits
			Temp = Temp & ~Mask;
			if ((~Mask & Value)==0)
			{
			    // Set new bitfield value
				Temp = Temp | (Mask & Value);
				// Write to register
				*(volatile UINT_T*)Address = Temp;
			}
			else 
			{
				Retval = INSTR_InvalidMaskOrValue;
			}
	}
	else
	{
			Retval = INSTR_InvalidAddress;
	}
 	return Retval;
}

UINT_T Wait_For_Bit_Pattern_Instruction (UINT_T Address, UINT_T Mask, 
										UINT_T Value, UINT_T Timeout,UINT_T IgnoreTimeoutFlag )
{
	UINT_T bitbucket; 
	UINT_T Retval = NoError;
	if( ValidAddress(Address))
	{
		bitbucket = *(volatile UINT_T*)Address;
		if ((~Mask & Value)==0)
		{
		 	// Wait for (Address & Mask) == Value
	 		Retval = WaitForOperationComplete(Timeout, // Timeout in msec 
	 										WAITINMILLISECONDS, 
	 										(volatile UINT_T*)Address, 
	 										Value, // Value 
	 										Mask // Mask
	 										);		   
		   	if((Retval == NoError) || (IgnoreTimeoutFlag !=0))
				{Retval = NoError;}
			else
				{Retval = INSTR_Timeout;}
		}
		else 
		{
			Retval = INSTR_InvalidMaskOrValue;
		}
	}
	else
	{
		Retval = INSTR_InvalidAddress;
	}
 	return Retval;
}

UINT_T Test_If_Zero_And_Set (UINT_T TestAddress, UINT_T TestMask, 
							UINT_T SetAddress, UINT_T SetMask,	UINT_T Value )
{
	UINT_T temp; 
	UINT_T Retval = NoError;
	if( ValidAddress(TestAddress) && ValidAddress(SetAddress) )
	{
		if ((~SetMask & Value)==0)
		{
			// SetMask and Value are consistent
			temp = *(volatile UINT_T*)TestAddress;
			if ((temp & TestMask) == 0)
			{
				// TestMask bits are zero
				temp = *(volatile UINT_T*)SetAddress;
				// Clear bitfield, retain all other bits
				temp = temp & ~SetMask;
			    // Set new bitfield value
				temp = temp | (SetMask & Value);
				// Write to register
				*(volatile UINT_T*)SetAddress = temp;
			}
		}//Endif
		else 
		{
			Retval = INSTR_InvalidMaskOrValue;
		}//EndifElse
	}//Endif
	else
	{
		Retval = INSTR_InvalidAddress;
	}//EndifElse
 	return Retval;
}

UINT_T Test_If_Not_Zero_And_Set (UINT_T TestAddress, UINT_T TestMask, 
							UINT_T SetAddress, UINT_T SetMask,	UINT_T Value )
{
	UINT_T temp; 
	UINT_T Retval = NoError;
	if( ValidAddress(TestAddress) && ValidAddress(SetAddress) )
	{
		if ((~SetMask & Value)==0)
		{
			// SetMask and Value are consistent
			temp = *(volatile UINT_T*)TestAddress;
			if ((temp & TestMask) != 0)
			{
				// TestMask bits are zero
				temp = *(volatile UINT_T*)SetAddress;
				// Clear bitfield, retain all other bits
				temp = temp & ~SetMask;
			    // Set new bitfield value
				temp = temp | (SetMask & Value);
				// Write to register
				*(volatile UINT_T*)SetAddress = temp;
			}
		}//Endif
		else 
		{
			Retval = INSTR_InvalidMaskOrValue;
		}//EndifElse
	}//Endif
	else
	{
		Retval = INSTR_InvalidAddress;
	}//EndifElse
 	return Retval;
}

UINT_T Load_SM_Addr_Instruction (UINT_T ScratchMemoryId, UINT_T Address)
{
 	UINT_T temp;
 	UINT_T *pSM = 0; 
	UINT_T Retval = NoError;

	if(ValidAddress(Address))
	{
		pSM = GetSMPtr(ScratchMemoryId);
		if (pSM != 0)
		{
			*pSM = *(volatile UINT_T*)Address;
		}
		else
		{
			Retval = INSTR_InvalidScratchMemoryId;
		}	
	}
	else
	{
		Retval = INSTR_InvalidAddress;
	}
	return Retval;
}


UINT_T Store_SM_Addr_Instruction (UINT_T ScratchMemoryId, UINT_T Address)
{
 	UINT_T temp;
 	UINT_T *pSM = 0; 
	UINT_T Retval = NoError;

	if(ValidAddress(Address))
	{
		pSM = GetSMPtr(ScratchMemoryId);
		if (pSM != 0)
		{
			*(volatile UINT_T*)Address = *pSM;
		}
		else
		{
			Retval = INSTR_InvalidScratchMemoryId;
		}	
	}
	else
	{
		Retval = INSTR_InvalidAddress;
	}

	return Retval;
}


UINT_T Load_SM_Val_Instruction (UINT_T ScratchMemoryId, UINT_T Value)
{
 	UINT_T temp;
 	UINT_T *pSM = 0; 
	UINT_T Retval = NoError;

   	pSM = GetSMPtr(ScratchMemoryId);
	if (pSM != 0)
	{
		*pSM = Value;
	}
	else
	{
		Retval = INSTR_InvalidScratchMemoryId;
	}	
	return Retval;
}
 
UINT_T Mov_SM_SM_Instruction(UINT_T DestinationScratchMemoryId,
							UINT_T SourceScratchMemoryId) 
{
	UINT_T *pDestSM, *pSrcSM;
	UINT_T Retval = NoError;

	pDestSM = GetSMPtr(DestinationScratchMemoryId);
	pSrcSM = GetSMPtr(SourceScratchMemoryId);
	if ((pDestSM != 0)&&(pSrcSM !=0))
	{
		*pDestSM = *pSrcSM;
	}
	else
	{
		Retval = INSTR_InvalidScratchMemoryId;
	}	
	return Retval;
}

UINT_T Rshift_SM_Val_Instruction(UINT_T ScratchMemoryId, UINT_T Value)
{
 	UINT_T *pSM = 0; 
	UINT_T Retval = NoError;

   	pSM = GetSMPtr(ScratchMemoryId);
	if (pSM != 0)
	{
		*pSM = (*pSM >> Value);
	}
	else
	{
		Retval = INSTR_InvalidScratchMemoryId;
	}	
	return Retval;
}

UINT_T Lshift_SM_Val_Instruction(UINT_T ScratchMemoryId, UINT_T Value)
{
 	UINT_T *pSM = 0; 
	UINT_T Retval = NoError;

   	pSM = GetSMPtr(ScratchMemoryId);
	if (pSM != 0)
	{
		*pSM = (*pSM << Value);
	}
	else
	{
		Retval = INSTR_InvalidScratchMemoryId;
	}	
	return Retval;
}

UINT_T And_SM_Val_Instruction (UINT_T ScratchMemoryId, UINT_T Value)
{
 	UINT_T temp;
 	UINT_T *pSM = 0; 
	UINT_T Retval = NoError;

   	pSM = GetSMPtr(ScratchMemoryId);
	if (pSM != 0)
	{
		*pSM = *pSM & Value;
	}
	else
	{
		Retval = INSTR_InvalidScratchMemoryId;
	}	
	return Retval;
}

UINT_T Or_SM_Val_Instruction (UINT_T ScratchMemoryId, UINT_T Value)
{
 	UINT_T temp;
 	UINT_T *pSM = 0; 
	UINT_T Retval = NoError;

   	pSM = GetSMPtr(ScratchMemoryId);
	if (pSM != 0)
	{
		*pSM = *pSM | Value;
	}
	else
	{
		Retval = INSTR_InvalidScratchMemoryId;
	}	
	return Retval;
}

UINT_T And_SM_SM_Instruction(UINT_T DestinationScratchMemoryId,
							 UINT_T SourceScratchMemoryId) 
{
	UINT_T *pDestSM, *pSrcSM;
	UINT_T Retval = NoError;

	pDestSM = GetSMPtr(DestinationScratchMemoryId);
	pSrcSM = GetSMPtr(SourceScratchMemoryId);
	if ((pDestSM != 0)&&(pSrcSM !=0))
	{
		*pDestSM = *pDestSM & *pSrcSM;
	}
	else
	{
		Retval = INSTR_InvalidScratchMemoryId;
	}	
	return Retval;
}

UINT_T Or_SM_SM_Instruction(UINT_T DestinationScratchMemoryId,
							UINT_T SourceScratchMemoryId) 
{
	UINT_T *pDestSM, *pSrcSM;
	UINT_T Retval = NoError;

	pDestSM = GetSMPtr(DestinationScratchMemoryId);
	pSrcSM = GetSMPtr(SourceScratchMemoryId);
	if ((pDestSM != 0)&&(pSrcSM !=0))
	{
		*pDestSM = *pDestSM | *pSrcSM;
	}
	else
	{
		Retval = INSTR_InvalidScratchMemoryId;
	}	
	return Retval;
}

UINT_T Test_SM_If_Zero_And_Set (UINT_T TestScratchMemoryId, UINT_T TestMask, 
							UINT_T SetScratchMemoryId, UINT_T SetMask,	UINT_T Value )
{
	UINT_T temp;
	UINT_T *pTestSM, *pSetSM;
	UINT_T Retval = NoError;

	pTestSM = GetSMPtr(TestScratchMemoryId);
	pSetSM = GetSMPtr(SetScratchMemoryId);

	if ((pTestSM != 0)&&(pSetSM !=0))
	{
		if ((~SetMask & Value)==0)
		{
			// SetMask and Value are consistent
			temp = *pTestSM;
			if ((temp & TestMask) == 0)
			{
				// TestMask bits are zero
				temp = *pSetSM;
				// Clear bitfield, retain all other bits
				temp = temp & ~SetMask;
			    // Set new bitfield value
				temp = temp | (SetMask & Value);
				// Write to register
				*pSetSM = temp;
			}
		}//Endif
		else 
		{
			Retval = INSTR_InvalidMaskOrValue;
		}//EndifElse
	}//Endif
	else
	{
		Retval = INSTR_InvalidAddress;
	}//EndifElse
 	return Retval;
}

UINT_T Test_SM_If_Not_Zero_And_Set (UINT_T TestScratchMemoryId, UINT_T TestMask, 
							UINT_T SetScratchMemoryId, UINT_T SetMask,	UINT_T Value )
{
	UINT_T temp;
	UINT_T *pTestSM, *pSetSM;
	UINT_T Retval = NoError;

	pTestSM = GetSMPtr(TestScratchMemoryId);
	pSetSM = GetSMPtr(SetScratchMemoryId);

	if ((pTestSM != 0)&&(pSetSM !=0))
	{
		if ((~SetMask & Value)==0)
		{
			// SetMask and Value are consistent
			temp = *pTestSM;
			if ((temp & TestMask) != 0)
			{
				// TestMask bits are zero
				temp = *pSetSM;
				// Clear bitfield, retain all other bits
				temp = temp & ~SetMask;
			    // Set new bitfield value
				temp = temp | (SetMask & Value);
				// Write to register
				*pSetSM = temp;
			}
		}//Endif
		else 
		{
			Retval = INSTR_InvalidMaskOrValue;
		}//EndifElse
	}//Endif
	else
	{
		Retval = INSTR_InvalidAddress;
	}//EndifElse
 	return Retval;
}

// 
//	This function will procees and array of Instruction Operations
//
//	Instructions - pointer to start of the array must be pInstruction_S type
//  Num - number op operations in the array
//
UINT_T ProcessInstructions(pINSTRUCTION_S pInstructions, unsigned int Num, unsigned int TimeoutFlag)
{
	int i;
	unsigned int Retval = NoError;
	unsigned int *pParam;
    

	if(pInstructions == NULL)
		return NULLPointer;
	// Set pointer to 1st parameter  
	pParam = &pInstructions->Parameters;
	for(i = 0;i < Num; i++)
	{
		switch(pInstructions->InstructionId)
		{
			case INSTR_NOP:
				break;
			case INSTR_WRITE:
				//WriteInstruction( (unsigned int *)pParam[0], pParam[1]);
				WriteInstruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_READ:
				ReadInstruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_DELAY:
				Delay(pParam[0]);
				pInstructions = (pINSTRUCTION_S)&pParam[1];
				break;
			case INSTR_WAIT_FOR_BIT_SET:
				Retval = WaitForBitSetInstruction(pParam[0], pParam[1], pParam[2],TimeoutFlag); 
				pInstructions = (pINSTRUCTION_S)&pParam[3];
				if ((Retval == INSTR_Timeout) && (TimeoutFlag == 0))
					Retval = NoError; //ignore error				 
				break;
			case INSTR_WAIT_FOR_BIT_CLEAR:
				Retval = WaitForBitClearInstruction(pParam[0], pParam[1], pParam[2],TimeoutFlag); 
				pInstructions = (pINSTRUCTION_S)&pParam[3];
				if ((Retval == INSTR_Timeout) && (TimeoutFlag == 0))
					Retval = NoError; //ignore error				 
				break;
			case INSTR_AND_VAL:
				AndInstruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_OR_VAL:
				OrInstruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_SET_BITFIELD:
				// Parameters = Address, Mask, Value
				Retval = Set_Bitfield_Instruction (pParam[0],pParam[1],pParam[2]);
				pInstructions = (pINSTRUCTION_S)&pParam[3];
				break;
			case INSTR_WAIT_FOR_BIT_PATTERN:
				// Parameters = Address, Mask, Value, Timeout
			 	Retval = Wait_For_Bit_Pattern_Instruction (pParam[0],pParam[1],pParam[2], pParam[3],TimeoutFlag);
				pInstructions = (pINSTRUCTION_S)&pParam[4];
				if ((Retval == INSTR_Timeout) && (TimeoutFlag == 0))
					Retval = NoError; //ignore error				 
				break;
			case INSTR_TEST_IF_ZERO_AND_SET:
				// Parameters = AddressToTest, TestMask, AddressToSet, SetMask, Value
				Retval = Test_If_Zero_And_Set (pParam[0],pParam[1],pParam[2],pParam[3], pParam[4]);
				pInstructions = (pINSTRUCTION_S)&pParam[5];
				break;
			case INSTR_TEST_IF_NOT_ZERO_AND_SET:
				// Parameters = AddressToTest, TestMask, AddressToSet, SetMask, Value
				Retval = Test_If_Not_Zero_And_Set (pParam[0],pParam[1],pParam[2],pParam[3],pParam[4]);
				pInstructions = (pINSTRUCTION_S)&pParam[5];
				break;
			case INSTR_LOAD_SM_ADDR:
				// Parameters =  Address, ScratchMemoryId
				Retval = Load_SM_Addr_Instruction (pParam[0],pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_LOAD_SM_VAL:
				// Parameters =  ScratchMemoryId, Value
				Retval = Load_SM_Val_Instruction (pParam[0],pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_STORE_SM_ADDR:
				// Parameters =  Address, ScratchMemoryId
				Retval = Store_SM_Addr_Instruction (pParam[0],pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_MOV_SM_SM:
			   	// Parameters =  DestinationScratchMemoryId, SourceScratchMemoryId
				Retval = Mov_SM_SM_Instruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_RSHIFT_SM_VAL:
				// Parameters =  ScratchMemoryId, ShiftValue
				Retval = Rshift_SM_Val_Instruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_LSHIFT_SM_VAL:
				// Parameters =  ScratchMemoryId, ShiftValue
				Retval = Lshift_SM_Val_Instruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_AND_SM_VAL:
				// Parameters =  ScratchMemoryId, Value 
				Retval = And_SM_Val_Instruction (pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_OR_SM_VAL:
				// Parameters =  ScratchMemoryId, Value 
				Retval = Or_SM_Val_Instruction (pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_OR_SM_SM:
				// Parameters =  DestinationScratchMemoryId, ScratchMemoryIdContainingValueToOR 
				Retval = Or_SM_SM_Instruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
			case INSTR_AND_SM_SM:
				// Parameters =  DestinationScratchMemoryId, ScratchMemoryIdContainingValueToAND 
				Retval = And_SM_SM_Instruction(pParam[0], pParam[1]);
				pInstructions = (pINSTRUCTION_S)&pParam[2];
				break;
 			case INSTR_TEST_SM_IF_ZERO_AND_SET:
				// Parameters = AddressToTest, TestMask, AddressToSet, SetMask, Value
				Retval = Test_SM_If_Zero_And_Set (pParam[0],pParam[1],pParam[2],pParam[3], pParam[4]);
				pInstructions = (pINSTRUCTION_S)&pParam[5];
				break;
			case INSTR_TEST_SM_IF_NOT_ZERO_AND_SET:
				// Parameters = AddressToTest, TestMask, AddressToSet, SetMask, Value
				Retval = Test_SM_If_Not_Zero_And_Set (pParam[0],pParam[1],pParam[2],pParam[3],pParam[4]);
				pInstructions = (pINSTRUCTION_S)&pParam[5];
				break;
	
 			default:
				Retval = INSTR_UnknownInstruction;
		 }
		 if(Retval != NoError)
			break;
		pParam = &pInstructions->Parameters; 		
	 }
	return Retval;
}
 