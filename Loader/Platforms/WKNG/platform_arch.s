
		AREA TEXT, CODE, READONLY
		CODE32						; enable this line to generate ARM instructions
	   ;CODE16						; enable this line to generate thumb instructions
		ALIGN 4

		ENTRY
		EXPORT	 getCPUMode

getCPUMode	FUNCTION


;
; switch to ARM mode so the mrc can be used...
; (this syntax works even if already in ARM mode.)

		mov	r2, pc					; puts address of mrc instruction into pc.*
		bx	r2						; go to that address. since the low order bit
									; is 0, this will force execution in ARM mode.

; need to be in ARM mode because coprocessor
; registers are not accessible from thumb mode.
		CODE32						; select ARM mode.

		mrc	p15, 0, r2, c0, c0, 0	; get cpu info into r2
		ldr r1, [r1]				; get the mask value from the input parameter.
		and r1, r2, r1				; clear any 0 bits, retain the 1 bits.
		str	r1, [r0]				; store result in caller's buffer.
		mov	r0, r2					; use original cpu info as return value.


		bx lr						; execution will resume in the correct mode
									; (ARM vs. thumb) by virtue of the lower
									; order bit of LR.
		ENDFUNC

;* in this syntax (mov r2,pc), the 'pc' really has the adress of the instruction
;  two insructions away. that is, 8 bytes in arm mode, 4 bytes in thumb mode.

	END
