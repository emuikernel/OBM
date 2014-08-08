@******************************************************************************
@**
@**  COPYRIGHT (C) 2011 Marvell Corporation.
@**	 All Rights Reserved
@**
@**  FILENAME:	Platform_defs.inc
@**
@**  PURPOSE: 	Platform defines required for staturp code
@**
@******************************************************************************

@ Define Processor variant for version information
			.equ		Processor,(0x574B4E47)	@ WKNG
	.equ		ProcessorSubType,(0x00005931)	@ Y1


	.ifeq BOOTROM
@ stack start is actuall stack bottom: the lowest stack address.
@ make sure this is compatible with the settings in bl_platform.mak
		.equ		BL_STACK_SIZE,0x00008000

	.ifne DDRBASE
		.equ		BL_STACK_START,(0x01C28000-BL_STACK_SIZE)
	.else
		.equ		BL_STACK_START,(0xD1020000-BL_STACK_SIZE)
	.endif

	.equ		DDR_BL_TRANSFER_CODE_ADDR,0x01C2D400

	.endif

	
