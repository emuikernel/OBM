;------------------------------------------------------------
; (C) Copyright [2006-2008] Marvell International Ltd.
; All Rights Reserved
;------------------------------------------------------------

;-----------------------------------------------------------------------------------;
;   Copyright (C) 2004 Intel Corporation												;
;												                                    ;
;	Author 		:																	;
;												                                    ;
;   Purpose		:Holds ARM946E CP15 assembler code									;
;												                                    ;
;	Date Created:6/21/2007 8:37:58 AM												;
;	 File: asm_arm946e_sCP15_code.s 				                                ;
;		   to be included in the sCP15 file for DDR/TCM								;
;
;    HISTORY:
;		YK 21 March 2007	duplicated APIs to reside on both DDR (for init) and TCM (for D2 exit)
;			file is saved in CC and symbolic name with extension _TCS is created too,
;			thus same code is compiled twice (different API names) to be on DDR and TCM
;			server at system init and at D2-exit.
;
; NOTE:
;  APIs with _DS _TCS mean they are duplicated twice, to run at init from DDR
;	and to run from TCM on D2 exit. The source files are duplicated in CC and controlled
;	by compilation switch
; 
; asm_arm946e_sCP15_code.s is the implementation file included in both asm_arm946e_sCP15.s & asm_arm946e_sCP15_TCS.s 
;
;-----------------------------------------------------------------------------------;

; extern unsigned long OBM_asm_Arm9ReadCP15_DS(unsigned long add_r0);
; extern unsigned long OBM_asm_Arm9ReadCP15_DI(unsigned long add_r0);

 EXPORT OBM_asm_Arm9ReadCP15_DS
OBM_asm_Arm9ReadCP15_DS
	bic r0,r0,#0xFFFFFF00
	mov r2,#0x33
	cmp r0,r2
	bgt  OBM_asm_arbel_cp15_read_end
	mov r0,r0,lsl #0x3
	ldr r2,=OBM_asm_arbel_cp15_read_base
	add r2,r2,r0
	ldr r3,=OBM_asm_arbel_cp15_read_end
	BX  R2
	LTORG

OBM_asm_arbel_cp15_read_end
	BX LR

OBM_asm_arbel_cp15_read_base
	;; Read
	MRC p15, 0, R0, c0, c0, 0 ; return ID register
    BX  R3
	MRC p15, 0, R0, c0, c0, 1 ; returns chache details
    BX  R3
	MRC p15, 0, R0, c0, c0, 2 ; return TCM size register
    BX  R3
	MRC p15, 0, R0, c1, c0, 0 ; read control register
    BX  R3
	MRC p15, 0, R0, c2, c0, 0 ; read data chachable bits;
    BX  R3
	MRC p15, 0, R0, c2, c0, 1 ; read instruction chachable bits;
    BX  R3
	MRC p15, 0, R0, c3, c0, 0 ; read data bufferable bits
    BX  R3
	MRC p15, 0, R0, c5, c0, 2 ; read data access permission bits
    BX  R3
	MRC p15, 0, R0, c5, c0, 3 ; read instruction access permission bits
    BX  R3
	MRC p15, 0, R0, c6, c0, 0 ; read memory region 0
    BX  R3
	MRC p15, 0, R0, c6, c1, 0 ; read memory region 1
    BX  R3
	MRC p15, 0, R0, c6, c2, 0 ; read memory region 2
    BX  R3
	MRC p15, 0, R0, c6, c3, 0 ; read memory region 3
    BX  R3
	MRC p15, 0, R0, c6, c4, 0 ; read memory region 4
    BX  R3
	MRC p15, 0, R0, c6, c5, 0 ; read memory region 5
    BX  R3
	MRC p15, 0, R0, c6, c6, 0 ; read memory region 6
    BX  R3
	MRC p15, 0, R0, c6, c7, 0 ; read memory region 7
    BX  R3
	nop ; Flush instruction cache
    BX  R3
	nop ; Flush instruction cache single entry
    BX  R3
	nop ; Prefetch instructio cache line
    BX  R3
	nop ; Flush data cache
    BX  R3
	nop ; Flush data cache single entry
    BX  R3
	nop ; Clean data cache entry
    BX  R3
	nop ; Clean and flush data cache entry
    BX  R3
	nop ; Clean data cache entry
    BX  R3
	nop ; Clean and flush data cache entry
    BX  R3
	nop ; drain write buffer
    BX  R3
	nop ; wait for interrupt (low power state)
    BX  R3
	MRC p15, 0, R0, c9, c0, 0 ; read data lockdown control
    BX  R3
	MRC p15, 0, R0, c9, c0, 1 ; read instruction lockdown control
    BX  R3
	MRC p15, 0, R0, c9, c1, 0 ; read data TCM region registers
    BX  R3
	MRC p15, 0, R0, c9, c1, 1 ; read instruction TCM region registers
    BX  R3
	MRC p15, 0, R0, c13, c0, 1 ; read PID register
    BX  R3
	MRC p15, 0, R0, c15, c0, 1 ; TAG BIST Control register
    BX  R3
	MRC p15, 1, R0, c15, c0, 1 ; TCM BIST Control register
    BX  R3
	MRC p15, 2, R0, c15, c0, 1 ; Cache RAM BIST Control register
    BX  R3
	MRC p15, 0, R0, c15, c0, 2 ; Instruction TAG BIST Address Register
    BX  R3
	MRC p15, 0, R0, c15, c0, 3 ; Instruction TAG BIST General Register
    BX  R3
	MRC p15, 0, R0, c15, c0, 6 ; Data TAG BIST Address Register
    BX  R3
	MRC p15, 0, R0, c15, c0, 7 ; Data TAG BIST General Register
    BX  R3
	MRC p15, 1, R0, c15, c0, 2 ; Instruction TCM BIST Address Register
    BX  R3
	MRC p15, 1, R0, c15, c0, 3 ; Instruction TCM BIST General Register
    BX  R3
	MRC p15, 1, R0, c15, c0, 6 ; Data TCM BIST Address Register
    BX  R3
	MRC p15, 1, R0, c15, c0, 7 ; Data TCM BIST General Register
    BX  R3
	MRC p15, 2, R0, c15, c0, 2 ; Instruction Cache RAM BIST Address Register
    BX  R3
	MRC p15, 2, R0, c15, c0, 3 ; Instruction Cache RAM BIST General Register
    BX  R3
	MRC p15, 2, R0, c15, c0, 6 ; Data Cache RAM BIST Address Register
    BX  R3
	MRC p15, 2, R0, c15, c0, 7 ; Data Cache RAM BIST General Register
    BX  R3
	MRC p15, 0, R0, c15, c0, 0 ; test state register
    BX  R3
	MRC p15, 3, R0, c15, c0, 0 ; read cp15 cach debug index register
    BX  R3
	MRC p15, 3, R0, c15, c1, 0 ; Instruction TAG read
    BX  R3
	MRC p15, 3, R0, c15, c2, 0 ; Data TAG read
    BX  R3
	MRC p15, 3, R0, c15, c3, 0 ; Instruction cache read
    BX  R3
	MRC p15, 3, R0, c15, c4, 0 ; Data cache read
    BX  R3
	MRC p15, 1, R0, c15, c1, 0 ; Trace control register
    BX  R3

;/*************************************************************************/
;extern unsigned long OBM_asm_Arm9WriteCP15_TCS(unsigned long val_r0 ,unsigned long add_r1);
;extern unsigned long OBM_asm_Arm9WriteCP15_DS (unsigned long val_r0 ,unsigned long add_r1);

 EXPORT OBM_asm_Arm9WriteCP15_DS
OBM_asm_Arm9WriteCP15_DS
	bic r1,r1,#0xFFFFFF00
	mov r2,#0x33
	cmp r1,r2
	bgt  OBM_asm_arbel_cp15_write_end
	mov r1,r1,lsl #0x3
	ldr r2,=OBM_asm_arbel_cp15_write_base
	add r2,r2,r1
	ldr r3,=OBM_asm_arbel_cp15_write_end
	BX  R2
	LTORG

OBM_asm_arbel_cp15_write_end
	BX LR

OBM_asm_arbel_cp15_write_base

	;; Write
	nop ; read only
    BX  R3
	nop ; read only
    BX  R3
	nop ; read only
    BX  R3
	MCR p15, 0, R0, c1, c0, 0 ; write control register
    BX  R3
	MCR p15, 0, R0, c2, c0, 0 ; write data chachable bits
    BX  R3
	MCR p15, 0, R0, c2, c0, 1 ; write instruction chachable bits
    BX  R3
	MCR p15, 0, R0, c3, c0, 0 ; write data bufferable bits
    BX  R3
	MCR p15, 0, R0, c5, c0, 2 ; write data access permission bits
    BX  R3
	MCR p15, 0, R0, c5, c0, 3 ; write instruction access permission bits
    BX  R3
	MCR p15, 0, R0, c6, c0, 0 ; write memory region 0
    BX  R3
	MCR p15, 0, R0, c6, c1, 0 ; write memory region 1
    BX  R3
	MCR p15, 0, R0, c6, c2, 0 ; write memory region 2
    BX  R3
	MCR p15, 0, R0, c6, c3, 0 ; write memory region 3
    BX  R3
	MCR p15, 0, R0, c6, c4, 0 ; write memory region 4
    BX  R3
	MCR p15, 0, R0, c6, c5, 0 ; write memory region 5
    BX  R3
	MCR p15, 0, R0, c6, c6, 0 ; write memory region 6
    BX  R3
	MCR p15, 0, R0, c6, c7, 0 ; write memory region 7
    BX  R3
	MCR p15, 0, R0, c7, c5, 0 ; Flush instruction cache
    BX  R3
	MCR p15, 0, R0, c7, c5, 1 ; INVALIDATE(!!!) instruction cache single entry
    BX  R3
	MCR p15, 0, R0, c7, c13, 1 ; Prefetch instructio cache line
    BX  R3
	MCR p15, 0, R0, c7, c6, 0 ; Flush data cache
    BX  R3
	MCR p15, 0, R0, c7, c6, 1 ; INVALIDATE(!!!) data cache single entry   ARM946E_CP15_INVALIDATE_DATA_CACHE_SINGLE_ENTRY_REG
    BX  R3
	MCR p15, 0, R0, c7, c10, 1 ; Clean data cache entry										ARM946E_CP15_CLEAN_DATA_CACHE_ENTRY_REG
    BX  R3
	MCR p15, 0, R0, c7, c14, 1 ; Clean and flush data cache entry
    BX  R3
	MCR p15, 0, R0, c7, c10, 2 ; Clean data cache entry
    BX  R3
	MCR p15, 0, R0, c7, c14, 2 ; Clean and flush data cache entry
    BX  R3
	MCR p15, 0, R0, c7, c10, 4 ; drain write buffer
    BX  R3
	MCR p15, 0, R0, c7, c0, 4 ; wait for interrupt (low power state)
    BX  R3
	MCR p15, 0, R0, c9, c0, 0 ; write data lockdown control
    BX  R3
	MCR p15, 0, R0, c9, c0, 1 ; write instruction lockdown control
    BX  R3
	MCR p15, 0, R0, c9, c1, 0 ; write data TCM region registers
    BX  R3
	MCR p15, 0, R0, c9, c1, 1 ; write instruction TCM region registers
    BX  R3
	MCR p15, 0, R0, c13, c0, 1 ; write PID register
    BX  R3
	MCR p15, 0, R0, c15, c0, 1 ; TAG BIST Control register
    BX  R3
	MCR p15, 1, R0, c15, c0, 1 ; TCM BIST Control register
    BX  R3
	MCR p15, 2, R0, c15, c0, 1 ; Cache RAM BIST Control register
    BX  R3
	MCR p15, 0, R0, c15, c0, 2 ; Instruction TAG BIST Address Register
    BX  R3
	MCR p15, 0, R0, c15, c0, 3 ; Instruction TAG BIST General Register
    BX  R3
	MCR p15, 0, R0, c15, c0, 6 ; Data TAG BIST Address Register
    BX  R3
	MCR p15, 0, R0, c15, c0, 7 ; Data TAG BIST General Register
    BX  R3
	MCR p15, 1, R0, c15, c0, 2 ; Instruction TCM BIST Address Register
    BX  R3
	MCR p15, 1, R0, c15, c0, 3 ; Instruction TCM BIST General Register
    BX  R3
	MCR p15, 1, R0, c15, c0, 6 ; Data TCM BIST Address Register
    BX  R3
	MCR p15, 1, R0, c15, c0, 7 ; Data TCM BIST General Register
    BX  R3
	MCR p15, 2, R0, c15, c0, 2 ; Instruction Cache RAM BIST Address Register
    BX  R3
	MCR p15, 2, R0, c15, c0, 3 ; Instruction Cache RAM BIST General Register
    BX  R3
	MCR p15, 2, R0, c15, c0, 6 ; Data Cache RAM BIST Address Register
    BX  R3
	MCR p15, 2, R0, c15, c0, 7 ; Data Cache RAM BIST General Register
    BX  R3
	MCR p15, 0, R0, c15, c0, 0 ; test state register
    BX  R3
	MCR p15, 3, R0, c15, c0, 0 ; write cp15 cach debug index register
    BX  R3
	MCR p15, 3, R0, c15, c1, 0 ; Instruction TAG write
    BX  R3
	MCR p15, 3, R0, c15, c2, 0 ; Data TAG write
    BX  R3
	MCR p15, 3, R0, c15, c3, 0 ; Instruction cache write
    BX  R3
	MCR p15, 3, R0, c15, c4, 0 ; Data cache write
    BX  R3
	MCR p15, 1, R0, c15, c1, 0 ; Trace control register
    BX  R3
 

 

;/*************************************************************************/
;extern void CPUCleanDCacheLine(UINT32);

    EXPORT asm_arm946e_CleanDCacheLine      ;original name
    EXPORT CPUCleanDCacheLine               ;generic name
    EXPORT XsCleanDCacheLine                ;temp name for compilation purposes
 
asm_arm946e_CleanDCacheLine
CPUCleanDCacheLine
XsCleanDCacheLine
    MCR p15, 0, R0, c7, c10, 1                ; Clean data cache entry (MVA)
    MCR p15, 1, R0, c7, c11, 3                ; Clean L2 Cache line fot Seagull
    BX LR

;/*************************************************************************/
;extern void CPUInvalidateDCacheLine(UINT32);

    EXPORT asm_arm946e_InvalidateDCacheLine ;original name
    EXPORT CPUInvalidateDCacheLine          ;generic name
    EXPORT XsInvalidateDCacheLine           ;temp name for compilation purposes
     
asm_arm946e_InvalidateDCacheLine
CPUInvalidateDCacheLine
XsInvalidateDCacheLine
    MCR p15, 0, R0, c7, c6, 1               ;INVALIDATE(!!!) data cache single entry
    MCR p15, 1, R0, c7, c7, 3                ; INVALIDATE L2 Cache line fot Seagull
    BX LR

    ;----------------------------------------------------------------------------------------
    ;DDR Only functions (TCM not currently supported) - Yossi Gabay (YG)
    ;----------------------------------------------------------------------------------------
    END
