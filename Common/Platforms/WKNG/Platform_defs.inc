;******************************************************************************
;**
;**  COPYRIGHT (C) 2011 Marvell Corporation.
;**	 All Rights Reserved
;**
;**  FILENAME:	Platform_defs.inc
;**
;**  PURPOSE: 	Platform defines required for staturp code
;**
;******************************************************************************

; Define Processor variant for version information
Processor			EQU		(0x574B4E47)	; WKNG
ProcessorSubType	EQU		(0x00005931)	; Y1


	IF BOOTROM=0
; stack start is actuall stack bottom: the lowest stack address.
; make sure this is compatible with the settings in bl_platform.mak
BL_STACK_SIZE		EQU		0x00008000

	IF DDRBASE=1
BL_STACK_START		EQU		(0x01C28000-BL_STACK_SIZE)
	ELSE
BL_STACK_START		EQU		(0xD1020000-BL_STACK_SIZE)
	ENDIF

DDR_BL_TRANSFER_CODE_ADDR	EQU		0x01C2D400

	ENDIF

	END
