 @
 @******************************************************************************
 @
 @  (C)Copyright 2005 - 2011 Marvell. All Rights Reserved.
 @
 @  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MARVELL.
 @  The copyright notice above does not evidence any actual or intended
 @  publication of such source code.
 @  This Module contains Proprietary Information of Marvell and should be
 @  treated as Confidential.
 @  The information in this file is provided for the exclusive use of the
 @  licensees of Marvell.
 @  Such users have the right to use, modify, and incorporate this code into
 @  products for purposes authorized by the license agreement provided they
 @  include this notice and the associated copyright notice with any such
 @  product.
 @  The information in this file is provided "AS IS" without warranty.
 @  ******************************************************************************
 
@**
@**  FILENAME:	OBM_StartUp.s
@**
@**  PURPOSE: 	StartUp code for Monahans variant OBM code
@**
@******************************************************************************

	.include "bl_StartUp_ttc_linux._S"
	.include "Platform_defs_linux.INC"

    @NOT SUPPORTED: AREA "Init, CODE, READONLY"

    @ External Functions
    .extern BootLoaderMain 
    .extern UndefinedHandler 
    .extern SwiHandler 
    .extern PrefetchHandler 
    .extern AbortHandler 
    .extern IrqHandler 
   	.extern FiqHandler 

	@ Export functions
	.global TransferControl 
   	.global EnableIrqInterrupts 
   	.global DisableIrqInterrupts 
   	.global __main 
   	.global L1_dcache_cleaninvalid_all 
   	.global L1_dcache_clean_flush_all 
   	.global FlushCache946 
   	.global FlushDcache 
   	.global FlushIcache 
   	.global CleanDcache 

	@ Import functions
   	.extern LoadPlatformConfig 
   	.extern SetupInterrupts 

@************************************************************
@* DKB/OBM entry point										*
@************************************************************
__main:
    @NOT SUPPORTED: ENTRY""
    	.global ExceptionVectors 
ExceptionVectors:
	B ResetHandler
	B UndefinedHandler
	B SwiHandler
	B PrefetchHandler
	B AbortHandler
	NOP    	    			@ Reserved Vector
	B SaveProgramState		 @ B IRQ Interrupt SaveProgramState
    B FiqHandler			 @ FIQ interrupts not anabled
	NOP


@ Table used for version information
@ and parameters passed up from the boot ROM
	.global ExecutableVersionInfo 
    .global IniRWROMBase 
    .global IniRWRAMBase 
    .global IniRWRAMLimit 
    .global IniZIRAMBase 
    .global IniZIRAMLimit 

ExecutableVersionInfo:
	.align 2
	.word	MajorMinor, Date, Processor, BuildNumber

__OBM_Size:
	.ifdef GCC
		.align 2
		.word __data_size
	.else
		.align 2
		.word IniRWROMLimit			@ This is the size of the OBM
	.endif

__BL_XFER_STR_ADDR:					@ Here we keep the address of the Xfer Struct
									@ that is coming from the BootRom.

    .align 5

@ Redirection Vectors
@ SetXXXVector will store the address of exception handlers here...
    .global RedirectionVectors 
RedirectionVectors:
    .align 2
    .word		ResetHandler
    .align 2
    .word		UndefinedHandler
    .align 2
    .word		SwiHandler
    .align 2
    .word		PrefetchHandler
    .align 2
    .word		AbortHandler
    .align 2
    .word		0			    	    @ Reserved Vector
    .align 2
    .word		SaveProgramState		 @ B IRQ Interrupt SaveProgramState
    .align 2
    .word		FiqHandler				 @ FIQ interrupts not anabled


@************************************************************
@* Initialize INT. Cntl. Mask all Ints, and set them to IRQ
@************************************************************

@ RESET entry point
ResetHandler:	
   	BL		LoadPlatformConfig
	STR     r1, __BL_XFER_STR_ADDR
	BL	 	SetupInterrupts

@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@ turn off the MMU when using the MMU testing code
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mrc		p15, 0, r1, c1, c0, 0	@ Read CP15 Control Register (R1)
	ldr		r2, =0x3005				@ turn off MMU, I/D caches,	and
	bic		r1, r1, r2				@ map Exception Vectors to low
  	mcr     p15, 0, r1, c1, c0, 0
	MOV 	r1, r1			        @ CPWAIT
 	SUB 	pc, pc, #4
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@ Flush TLB's
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   	mov     r1, #0xFFFFFFFF
   	mcr     p15, 0, r1, c3, c0, 0	   @ set up access to domain 0
   	mcr     p15, 0, r2, c8, c7, 0	   @ flush I + D TLBs
   	mcr     p15, 0, r1, c7, c10, 4	  @ drain write and fill buffers
   	MOV 	r2, r2				        @ CPWAIT
 	SUB 	pc, pc, #4				    @ CPWAIT
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


ContinueBoot:
@************************************************************
@* Assign SP locations for all Modes of operation 			*
@* Reserve space for image that is downloaded by reducing   *
@* the size of internal RAM
@************************************************************
	LDR		r0, =BL_STACK_START		@ from platform_defs.inc
	LDR 	r1, =BL_STACK_SIZE

   	MOV    	r2, r0			 		@ Get memory for stack pointer
   	ADD    	r2, r2, r1			 	@ Go length of that memory
  	SUB    	r2, r2, #8			   @ Back off by two words

	@ Enter IRQ mode and set up the IRQ stack pointer
    MOV     R0, #Mode_IRQ	          @ IRQ mode
    MSR     CPSR_c, r0
	@ Initialize register r8-r14 in this mode with zeros
    MOV		r1, #0
    MOV		r8, r1
    MOV		r9, r1
    MOV		r11, r1
    MOV		r12, r1
    MOV		r13, r1
    MOV		r14, r1
    MOV     SP, r2	                 @ Assign stack pointer for IRQ handler

	@ Enter SVC mode and set up the SVC stack pointer
    MOV     r0, #Mode_SVC	          @ SVC mode, No interrupts
    MSR     CPSR_c, r0
	@ Initialize register r8-r14 in this mode with zeros
    MOV		r1, #0
    MOV		r8, r1
    MOV		r9, r1
    MOV		r11, r1
    MOV		r12, r1
    MOV		r13, r1
    MOV		r14, r1
    SUB     r2, r2, #IrqStackSize
    MOV     SP, r2	                 @ Assign stack pointer for SVC handler


@***********************************************************************************
@  Enable all coprocessor registers
@***********************************************************************************
   	ORR		r0, r0, #0x3f00
	MCR 	p15, 0, r0, c15, c1, 0
	MOV 	r0, r0
   	MOV 	r0, r0
    MOV 	r0, r0
    MOV 	r0, r0


@*********************************************************************************
@ Initialize memory required by C code
@
@	r0 = Start of the RW section in ROM (where the RW resides inside the image itself)
@	r1 = Start location to put RW in RAM (runtime location of RW section)
@	r2 = End location of RW section in RAM (runtime location of RW section)
@	r3 = Start location of zero initialized section in RAM
@	r4 = End location of zero initialized section in RAM
@
@	r5 is used when copying RW and when zeroing out BSS section
@**********************************************************************************
__JUMP_TO_C_CODE:

    @ GCC memory init
	.ifdef GCC

    LDR     r0, =__egot         @ Get pointer to ROM data characteristics
    LDR     r1, =__data_start   @ Get pointer to RAM data characteristics
    LDR		r2, =__data_size	@ RAM section length
	add		r2, r2, r1			@ Add start + size to get end of RW section
    LDR		r3, =__bss_start	@ get the start of the BSS section
	LDR		r4, =__bss_end		@ get the end of the BSS section

	@ RVCT memory init
	.else

	LDR		r0, =IniRWROMBase	@ Start Local of RW data in ROM
	LDR		r0, [r0]
	LDR		r1, =IniRWRAMBase	@ Start Local of RW data in RAM
	LDR		r1, [r1]
	LDR		r2, =IniRWRAMLimit	@ End Local of RW data in RAM
	LDR		r2, [r2]
	LDR		r3, =IniZIRAMBase	@ Start Local of ZI data in RAM
	LDR		r3, [r3]
	LDR		r4, =IniZIRAMLimit	@ End Local of ZI data in RAM
	LDR		r4, [r4]

	.endif

	@ 1. Copy RW data
MoveRWSection:
   	CMP	    r1, r2    			@ have we reached the end?
	BGE		ClearBSS		 	@ branch if the start is greater than end
	LDR		r5, [r0], #4		 @ move ROM copy source
	STR		r5, [r1], #4		 @ to RAM target
	B		MoveRWSection			 

	@ 2. Clear out BSS
ClearBSS:
	MOV	    r5, #0
WriteZeroes:
	STR	    r5, [r3], #4		@ write a zero, then increment 'start address' by 4
	CMP	    r3, r4				@ compare the 'start' and 'end' addresses
	BLT	    WriteZeroes			@ continue writing zeros till we finish clearing the area


@************************************************************
@* Branch to Boot Loader Main program						*
@************************************************************
	LDR     r1, =__BL_XFER_STR_ADDR			@ Restore Xfer Struct Address passed in by the BootROM
	LDR		r0, [r1]						@ r0 = Address of the Xfer Struct.
	B	 	BootLoaderMain					

LOOPFOREVER:
    B		LOOPFOREVER		    			@ forever loop


@***********************************************************************************
@  1st step - Move Transfer Control to a Non-MMU translated area of the DDR
@
@  r0 = Boot Image Load Address
@  r1 = Boot Image Size
@  r2 = Boot Image Current Address
@
@***********************************************************************************

@ force the assembler to emit any constants that it has accumulated up til now.
@ otherwise those constants will unnecessarily be included in the relocation.
	.ltorg

TransferControl:
	ldr		r4,	=0xFFFFFFF8
	and		r7,	r0,	r4						@ Save the boot image load address in r7 (make sure it is word aligned)
	mov		r8, r1							@ Save the boot image size in r8.
	and		r9,	r2,	r4						@ Save the boot image current address in r9 (make it word aligned)

	ldr     r4, =__TransferControlCode		@ source address: start of function
    ldr		r2,	=__TransferControlCodeEnd	@ used to compute length of function
    sub		r2,	r2,	r4						@ end - start = len. r4=start, r2=len

    ldr     r1, =DDR_BL_TRANSFER_CODE_ADDR	@ Destination address in DDR
    mov		r5,	r1

@ Copy the TransferControlCode to a temporary location in DDR. 
@ We need this in case downloaded OBM image overwrites the current DKB image.
@ r4 = start, r2 = length, r1 = destination

CopyTransferCode:
	ldr     r3, [r4], #4				      @ Get data in source address
    str     r3, [r1], #4				      @ Place data in destination address
    subs    r2, r2, #4				      @ Decrement doubleword count
    bne     CopyTransferCode		           @ Copy data until it is all moved
	MOV  	pc, r5							@ Jump to 1:1 Mapped portion of DDR area

	.ltorg									@ Clean up the constants used so far.


@************************************************************
@* 2nd Step - Turn OFF MMU, Flush TLB's
@************************************************************
__TransferControlCode:
@ turn off the MMU when using the MMU testing code
	mrc		p15, 0, r1, c1, c0, 0		@ Read CP15 Control Register (R1)
	mov		r2, #0x3000	    			@ turn off MMU, I/D caches, and
	orr		r2, r2, #0x0005				@ turn off MMU, I/D caches, and
	bic		r1, r1, r2	    			@ map Exception Vectors to low
  	mcr     p15, 0, r1, c1, c0, 0
	MOV 	r1, r1				        @ CPWAIT
 	SUB 	pc, pc, #4

@ Flush TLB's
   	mov      r1, #0xFFFFFFFF
   	mcr      p15, 0, r1, c3, c0, 0	  @ set up access to domain 0
   	mcr      p15, 0, r2, c8, c7, 0	  @ flush I + D TLBs
   	mcr      p15, 0, r1, c7, c10, 4	 @ drain write and fill buffers
   	MOV 	r2, r2				        @ CPWAIT
 	SUB 	pc, pc, #4				    @ CPWAIT


@************************************************************
@* 3rd Step - Copy the boot image to it's load address
@************************************************************
	cmp		r7,	r9				@ compare the load address (LA) to current address (CA) of image
	beq		__JumpToBootImage	@ if they are the same, we don't need to move any data
	bls		__CopyFromStart		@ if (LA < CA) CopyFromStart
	add		r1,	r9,	r8			@
	cmp		r7,	r1				@ if (LA > CA + Size) CopyFromStart
	bhi		__CopyFromStart		@
								@ else CopyFromEnd
__CopyFromEnd:					@ __CopyFromEnd : copies images starting at end
	sub		r8, r8, #4			@ decrement size by 1 word
	ldr     r3, [r9, r8]	      @ Get data in current address
	str     r3, [r7, r8]	     @ Place data in load address
	cmp		r8, #0
	bgt		__CopyFromEnd		@ continue copying until size is less than 0
	b		__JumpToBootImage

__CopyFromStart:					@ __CopyFromStart: copies images starting at beginning
	ldr     r3, [r9], #4	      @ Get data in current address
	str     r3, [r7], #4	      @ Place data in load address
	sub		r8, r8, #4			@ decrement size by 1 word
	cmp		r8, #0
	bgt		__CopyFromStart		@ continue copying until size is less than 0
	b		__JumpToBootImage

__JumpToBootImage:
	MOV  	pc, r0				@ Jump to next image after DKB/OBM (ie. OBM, OS Loader)

	.ltorg
	@ the LTORG directive will cause any constants implicitly created by the code
	@ above to be generated right here. that way the function length calculation
	@ will include the space taken up by these constants. for example, a
	@ ldr r2, =0x3005 instruction will implicitly create the constant 0x3005.
__TransferControlCodeEnd:


@************************************************************
@ IRQ Vector Handler - Save Program State
@************************************************************
    .global SaveProgramState 
SaveProgramState:
	SUB		R14, R14, #4	@ Adjust return address before saving it
	STMFD	SP!, {R14}
	MRS		R14, SPSR
	STMFD   SP!, {R0-R12,R14}

	BL 		IrqHandler

	@ Modify CPS register
	MRS		r12, CPSR
	ORR    	r12,r12,#0x80    @ Disable IRQ interrupts
	MSR		CPSR_c, r12

	LDMFD 	SP!, {R0-R12,R14}
	MSR		SPSR_cxsf, R14
	LDMFD 	SP!, {PC}^


@************************************************************
@ VectorRedirector: 
@ jump to a location stored exactly 0x18 bytes from current pc.
@ as long as the redirected vector table immediately follows the
@ actual exception handlers, this instruction can be used for all.
@************************************************************
VectorRedirector:
	ldr		pc, [pc,#(RedirectionVectors-ExceptionVectors-0x08)]	@ vector is 0x18. pc=current_addr+8. irq ptr is at 0x38.


@************************************************************
@ SetInterruptVector
@ input args:
@
@ r0 = exception base address
@
@ save the IrqHandler address right after the vector table.
@ assumes address 0 is mapped.
@************************************************************
	.global SetInterruptVector 
SetInterruptVector:
	STMFD	SP!, {R0-R9, LR}

	ldr	r2, =UndefinedHandler
	ldr	r3, =SwiHandler
	ldr	r4, =PrefetchHandler
	ldr	r5, =AbortHandler
	ldr	r6, =SaveProgramState
	ldr r7, =FiqHandler

@ first save the address to the execption handlers
@ so the vector redirector can find it
	str		r2, [r0, #(RedirectionVectors-ExceptionVectors+0x04) ]
	str		r3, [r0, #(RedirectionVectors-ExceptionVectors+0x08) ]
	str		r4, [r0, #(RedirectionVectors-ExceptionVectors+0x0c) ]
	str		r5, [r0, #(RedirectionVectors-ExceptionVectors+0x10) ]
	str		r6, [r0, #(RedirectionVectors-ExceptionVectors+0x18) ]
	str		r7, [r0, #(RedirectionVectors-ExceptionVectors+0x1c) ]

@ now set up the vector redirector
@ the vector redirector will exist immediately after the location of the execption handlers: 0x40
	ldr		r8, =VectorRedirector
	ldr		r9, [r8]
	str		r9, [r0, #(0x00+0x04) ]
	str		r9, [r0, #(0x00+0x08) ]
	str		r9, [r0, #(0x00+0x0c) ]
	str		r9, [r0, #(0x00+0x10) ]
	str		r9, [r0, #(0x00+0x18) ]
	str		r9, [r0, #(0x00+0x1c) ]

	LDMFD   SP!,    {R0-R9, PC}


@************************************************************
@ EnableIrqInterrupts
@************************************************************
EnableIrqInterrupts:
	STMFD	SP!, {R0-R4, LR}
   	MRS    	r0,CPSR        @ get the processor status
   	BIC    	r0,r0,#0x80    @ Enable IRQ interrupts
   	MSR    	cpsr_cf,r0     @ SVC 32 mode with interrupts disabled
	LDMFD   SP!, {R0-R4, PC}


@************************************************************
@ DisableIrqInterrupts
@************************************************************
DisableIrqInterrupts:
	STMFD	SP!, {R0-R4, LR}
   	MRS    	r0,CPSR        @ get the processor status
   	ORR    	r0,r0,#0x80    @ Disable IRQ interrupts
   	MSR    	cpsr_cf,r0     @ SVC 32 mode with interrupts disabled
	LDMFD   SP!, {R0-R4, PC}


@************************************************************
@ raise: for handling divide exceptions. called from libgcc
@************************************************************
    .global raise 
raise:
    b		raise


@******************************************************************************
@ UINT32 ObmXsGetProcessorVersion (void)
@
@ Get processor version info
@
@ Input:   None
@ Return:  R0:  ID Register contents
@
@ Note: For first silicon of Cotulla, should be 0x69052000
@
@ Nonsymbolic core instruction:
@     MRC p15, 0, Rd, 0, 0, 0
@******************************************************************************
    .align 2
    .global ObmXsGetProcessorVersion  
ObmXsGetProcessorVersion:
    MRC 	p15, 0, r0, c0, c0, 0
	BX  	LR


@******************************************************************************
@ UINT32 ObmXsGetProcessorSubVersion (void)
@******************************************************************************
    .align 2
    .global ObmXsGetProcessorSubVersion  
ObmXsGetProcessorSubVersion:
@ IDs if MP1 or MP2 
    mrc 	p15, 0,  r0, c0, c0, 5 
    BX  	LR
    

L1_dcache_cleaninvalid_all:
   STMFD   sp!,{r0,lr}       @ save context
	
          MOV     r0, #0                    
cache1:
          MCR     p15, 0, r0, c7, c10, 2   @ clean D-cline

          ADD     r0, r0, #1<<4            @ +1 set index
          TST     r0, #1<<((12-2-5)+4)     @ test index overflow
          beq     cache1 

          BIC     r0, r0, #1<<((12-2-5)+4)  @ clear index overflow
          ADDS    r0, r0, #1<<30            @ +1 victim pointer
          BCC     cache2                        @ test way overflow
          MOV     r0, #0                    
cache2:
          MCR     p15, 0, r0, c7, c14, 2   @ clean D-cline

          ADD     r0, r0, #1<<4            @ +1 set index
          TST     r0, #1<<((12-2-5)+4)     @ test index overflow
          BEQ     cache2 

          BIC     r0, r0, #1<<((12-2-5)+4)  @ clear index overflow
          ADDS    r0, r0, #1<<30            @ +1 victim pointer
          BCC     cache2                        @ test way overflow

   LDMFD   sp!,{r0,pc}       @ return
   
L1_dcache_clean_flush_all:
   STMFD   sp!,{r0,lr}       @ save context
          MOV     r0, #0                   @
cache3:
          MCR     p15, 0, r0, c7, c14, 2   @ clean D-cline

          ADD     r0, r0, #1<<4            @ +1 set index
          TST     r0, #1<<((12-2-5)+4)     @ test index overflow
          BEQ     cache3 

          BIC     r0, r0, #1<<((12-2-5)+4) @ clear index overflow
          ADDS    r0, r0, #1<<30           @ +1 victim pointer
          BCC     cache3                       @ test way overflow
          MOV     r0, #0                   @
   LDMFD   sp!,{r0,pc}       @ return

FlushCache946:
        mov r0, #0
        mcr     p15,0,r0,c7,c6,0    @ flush D-cache
        mcr     p15,0,r0,c7,c5,0    @ flush I-cache
        mov pc, lr
        
FlushIcache:
	mov r0, #0
	mcr     p15,0,r0,c7,c5,0    @ flush I-cache
	mcr	p15,0,r1,c7,c5,6    @ BPU flsuh
	mcr	p15,0,r2,c7,c5,4    @ prefect flush
	mov pc, lr

FlushDcache:
	mov r0, #0
	mcr     p15,0,r0,c7,c6,0    @ flush D-cache
	mov pc, lr
	
CleanDcache:
	mov r0, #0
	mcr	p15,0,r0,c7,c14,0
	mov pc, lr
	


@******************************************************************************
@ RVCT based setup
@******************************************************************************
	.ifndef  GCC

	.extern ||Load$$ER_RW$$Base||  		@ ROM address - base of RW data
	.extern ||Image$$RO$$Limit||  		@ size of Code section (Image - RW)
	.extern ||Image$$ER_RW$$Base||  		@ ISRAM address - base of RW data
	.extern ||Image$$ER_RW$$Limit||  		@ ISRAM address - end of RW data
	.extern ||Image$$ZI$$Base||  			@ zero init start
	.extern ||Image$$ZI$$Limit||  		@ zero init end

	.align 2
IniRWROMBase:	.word ||Load$$ER_RW$$Base||
   .align 2
IniRWROMLimit:   .word ||Image$$RO$$Limit||
	.align 2
IniRWRAMBase:	.word ||Image$$ER_RW$$Base||
	.align 2
IniRWRAMLimit:	.word ||Image$$ER_RW$$Limit||
	.align 2
IniZIRAMBase:	.word ||Image$$ZI$$Base||
	.align 2
IniZIRAMLimit:	.word ||Image$$ZI$$Limit||

    .ltorg
	.endif

   	@ END for ENTRY

