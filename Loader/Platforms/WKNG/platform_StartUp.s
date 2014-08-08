;currentbaseline
;/******************************************************************************
;**	(C) Copyright 2011 Marvell International Ltd.
;**	All Rights Reserved
;******************************************************************************/
   	GET bl_StartUp_ttc._S
   	AREA    Init, CODE, READONLY

	EXPORT LoadPlatformConfig
   	EXPORT SetupInterrupts

SetupInterrupts
	mov     r0, r0
	LDR		r0, =InterruptsBase
	MOV		r1, #0
	STR		r1, [r0, #IntsIcmr]
	STR		r1, [r0, #IntsIclr]
	MOV     PC, LR

;************************************************************
;* Load transfer struct that is passed in from TBR			*
;************************************************************
; Don't overwrite the following register. Their values are
; being assinged to constants in bl_StartUp_ttc.s
; R0 = Saves the TIM Location
; R1 = Saves BL transfer address.
;************************************************************
LoadPlatformConfig
    LDR     r6, [r10, #32]		
    MOV     r6, r6, LSL #3		
    ADD     r6, r6, #36			; Calculate tbr xfer str size
	mov     r4, r10				; Save tbr xfer_str address.
	MOV		r5, r6				; Save tbr xfer_str size
lbl_search_tim
    LDR     r8, [r10, #4]!
    LDR     r7, =0x54494D48     ; TIMH
    CMP     r8, r7
    BNE     lb_search_cont
    LDR     r8, [r10, #4]!
    mov     r0, r8				; R0 = Saves the TIM Location
    B       lb_tim_found
lb_search_cont
    ADD     r10, r10, #4
    SUB     r6,  r6,  #4
    CMP     r6,  #0
    BNE     lbl_search_tim
lb_tim_found
	SUB     r8,  r8, r5			; __BL_XFER_ADDR = TIM Loc - tbr xfer_str size
	mov     r1, r8				; R1 = Saves BL transfer address.
__CopyTBX						; __CopyTBX: copies tbrx struct
	ldr     r7, [r4]      		; Get tbr xfer_str address.
	str     r7, [r8]      		; Place data in load address
	add     r8, r8, #4			; get next data in the struct.
	add     r4, r4, #4			; increment tbr xfer_str address.
	sub		r5, r5, #4			; decrement size by 1 word
	cmp		r5, #0
	bgt		__CopyTBX			; continue copying until size is less than 0
	mov     pc, lr

   	END

