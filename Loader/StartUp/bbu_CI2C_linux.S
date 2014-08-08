@*********************************************************************************
@
@     COPYRIGHT (C) 2006 Marvell International Ltd. All Rights Reserved.
@
@   The information in this file is furnished for informational use only,
@   is subject to change without notice, and should not be construed as
@   a commitment by Marvell. Marvell assumes no responsibility or liability
@   for any errors or inaccuracies that may appear in this software or any
@   documenation that may be provided in association with this software.
@
@*********************************************************************************
@
@                                     I2C_CS.s
@                           Subroutines for the I2C bus
@
@        INCLUDE xlli_TTC_defs.inc       ; TTC specific definitions (xlli)
@        INCLUDE bbu_TTC_defs.inc        ; BBU specific defs for the TTC processor

        .global BBU_CI2C_Init      @ Init the seleted I2C bus for use
        .global BBU_getCI2C        @ Read from I2C bus
        .global BBU_putCI2C        @ Write to  I2C bus

        
        .extern Delay        @ uS delay routine

@
@ I2C BUS INTERFACE UNIT base address and register offsets from the base address
@ 

  .equ     bbu_TWSI_PHYSICAL_BASE,0xD4011000
	.equ  APBcontrol_BASE,0xD403B000
	.equ	 APBcontrol_ICER,0x28

     .equ     bbu_I2C_IMBR_offset,0X00    @ I2C Bus Monitor Register
     .equ     bbu_I2C_IDBR_offset,0X08    @ I2C Data Buffer Register
      .equ     bbu_I2C_ICR_offset,0X10    @ I2C Control Register
          .equ     TWSI_SR_offset,0X18    @ I2C Status Register
     .equ     bbu_I2C_ISAR_offset,0X20    @ I2C Slave Address Register
     .equ     bbu_I2C_ILCR_offset,0x28    @ load Count Register


@; in all platform, we set the I2C as 100kbps standard mode, 
@; many device can't support 400kbps fast mode
@; set 400kbps for OLED
@bbu_ICR_MODE		EQU	0x0000 ;100kbps standard mode
		.equ	bbu_ICR_MODE,0x8000 @;400kbps fast mode


              .equ     bbu_ICR_UR,0x4000  @ Unit Reset bit
             .equ     bbu_ICR_IUE,0X0040  @ ICR I2C Unit enable bit
            .equ     bbu_ICR_SCLE,0X0020  @ ICR I2C SCL Enable bit
              .equ     bbu_ICR_TB,0X0008  @ ICR Transfer Byte bit
          .equ     bbu_ICR_ACKNAK,0x0004  @ ICR ACK bit
            .equ     bbu_ICR_STOP,0x0002  @ ICR STOP bit
           .equ     bbu_ICR_START,0X0001  @ ICR START bit

             .equ     bbu_ISR_IRF,0x0080  @ ISR Receive full bit
             .equ     bbu_ISR_ITE,0x0040  @ ISR Transmit empty bit
             .equ     bbu_PI2C_EN,0x0040  @ Power I2C enable bit
             .equ     bbu_ISR_BED,0x0400  @ Bus Error Detect bit

         .equ     BBU_I2C_TimeOut,0x4000  @ I2C bus timeout loop counter value

      .equ     bbu_PMRCREGS_PHYSICAL_BASE,0x40F50000      @ Power manager base address

         .equ     bbu_PCFR_offset,0x00C   @ Offset to Power Manager general config register
       .equ     bbu_PI2DBR_offset,0x188   @ Power I2C Data Buffer Register
        .equ     bbu_PI2CR_offset,0x190   @ Power I2C Control Register
        .equ     bbu_PI2SR_offset,0x198   @ Power I2C Status Register
       .equ     bbu_PI2SAR_offset,0x1A0   @ Power I2C Slave Address Register

    .equ     APB_CLOCK_UNIT_PHYSICAL_BASE,0xD4015000      @ APB Clock unit registers base address
        .equ     APBC_TWSI_CLK_RST_offset,0x2C            @ Clock/Reset Control register for TWSI

        .equ     bbu_BBUART_PHYSICAL_BASE,0xD4017000 
        .equ     bbu_UASPR_offset,0x1C    @ UART Scratch Pad Register offset

          @NOT SUPPORTED: AREA    "I2C, CODE, READONLY"
@
@*******************************************************************************
@
@       ****************
@       *              * 
@       * BBU_CI2C_Init * Subroutine
@       *              *
@       ****************
@
@       This subroutine is used to initialize (and/or reset) the regular or the
@       power I2C bus
@
@ PARAMETER PASSING:
@
@       Subroutine looks at the first BBU command switch and will init the power
@       I2C bus if a "P" is found. Otherwise the regular I2C bus is initialized.
@        
@

@NOT SUPPORTED: BBU_CI2C_Init:    FUNCTION
BBU_CI2C_Init:
        stmfd   sp!,    {r0-r4, lr}     @ Save used registers

@        ldr     r3,     =0xD4051024     ; Address of PMUM_ACGR
@        ldr     r2,     [r3]            ; Fetch the current value
@	 ldr     r4,     =0x600040    ; Set bit22,23 6
@        orr     r2,     r2,  r4     
@        str     r2,     [r3]            ; write this back to PMUM_ACGR

@
@       Enable I2C controller clock
@
        ldr     r3,     =APBcontrol_BASE  @ Clock Register base
        mov     r2,     #4
        str   r2,     [r3, #APBcontrol_ICER] @ NO  - Enable I2C controller clock

@
@       Reset the selected controller
@
        mov     r2,     #7                              @ Set bit 2 (reset bit)
        strne   r2,     [r3, #APBcontrol_ICER] @ Enable I2C controller clock
@        streq   r2,     [r3, #0x28]                     ; Enable Power I2C controller clock

        mov     r0,     #5      @ Set up for delay
        bl      Delay      @ delay
        ldr     r3,     =APBcontrol_BASE  @ Clock Register base

        mov     r2,     #3              @ Clear reset bit

        str   r2,     [r3, #APBcontrol_ICER] @ NO  - Enable I2C controller clock


        mov     r0,     #20     @ Set up for delay
        bl      Delay      @ delay
@
@       Load the base address for the I2C bus selected by the user. The regular I2C
@       bus is the default. If the user used /P for the first command switch, the
@       Power I2C bus is selected instead.
@

        ldr   r3,     =bbu_TWSI_PHYSICAL_BASE @ NO - Load TWSI controller base address

@
@       Reset the controller
@
        mov     r2,     #bbu_ICR_UR                             @ Set the unit reset bit
        orr     r2,     r2,     #(bbu_ICR_IUE | bbu_ICR_SCLE )    @ OR in the IUE and SCLE bits
	    orr	r2,	r2,	#bbu_ICR_MODE
        str     r2,     [r3, #bbu_I2C_ICR_offset]               @ Reset the unit
@
@       Short delay with reset bit set, then clear the bit.
@
        mov     r1,     #0x10                           @ Loop count
bb10:      subs    r1,     r1,     #1                      @ decrement loop count
        bne     bb10                                    @ loop until done
        bic     r2,     r2,     #bbu_ICR_UR             @ Clear the unit reset bit 
        str     r2,     [r3, #bbu_I2C_ICR_offset]       @ and write the TWSI Control Register
@
@       Give the controller some time to insure the reset is done.
@
        mov     r0,     #50     @ Set up for delay
        bl      Delay      @ delay
        ldr   r3,     =bbu_TWSI_PHYSICAL_BASE @ NO - Load TWSI controller base address

@	set the load count register to tune the I2C speed performance
	    ldr	r1,	=0x082c1da1@ this make standard mode at ~92kbps, fast mode at ~384kbps
	    str	r1,	[r3, #bbu_I2C_ILCR_offset]
  
        mov     r1,     #0                              @ Set host controller slave address
        str     r1,     [r3, #bbu_I2C_ISAR_offset]      @ Set host slave address register
        str     r1,     [r3, #bbu_I2C_ICR_offset]       @ Clear interrupts in ICR

       ldmfd   sp!,    {r0-r4, pc}     @ Return to caller
@NOT SUPPORTED:         ENDFUNC
@
@*******************************************************************************
@
@       **************
@       *            * 
@       * BBU_getCI2C * Subroutine
@       *            *
@       **************
@
@ This subroutine is used to read data from the Regular or Power I2C bus
@
@ PARAMETER PASSING:
@
@       r0 = address in target device where data is to be read from (preserved)
@       
@       NOTE: If read address equals 0x100, the code just returns after the device
@             address is sent. This can be used to discover if an address is used
@             without reading or writing any data to the device.
@
@             If read address equals 0x200, the code will read data back from
@             certain types of GPIO expanders used on some platforms.
@
@       POWER I2C BUS: This routine checks to see if the user entered a
@       /P= switch for the register to read from to deturmine if the read
@       is to occour from the regular I2C bus or the Power I2C bus.
@
@ RETURNED VALUES:
@
@       r1 = data read from the register pointed to by r0
@       r2 = non zero if device was detected
@       r2 = zero if device not detected (usually a time out issue)  
@
@NOT SUPPORTED: BBU_getCI2C:     FUNCTION
BBU_getCI2C:
        stmfd   sp!,    {r3-r7, lr}     @ Save used registers
        mov     r7,     r0              @ Save off r0 in r7
@
@       Load the base address for the I2C bus selected by the user. The regular I2C
@       bus is the default. If the user used /P for the first command switch, the
@       Power I2C bus is selected instead.
@

        ldr   r4,     =bbu_TWSI_PHYSICAL_BASE @ NO - Load TWSI controller base address

@
@       Get the slave's address (saved in UART scratch pad register)
@
        ldr    r5,  =bbu_BBUART_PHYSICAL_BASE       @ Fetch base address of the BBUART
        ldrb   r6,  [r5, #bbu_UASPR_offset]         @ Get contents of the scratch pad register
        str    r6,  [r4, #bbu_I2C_IDBR_offset]      @ Load Data Buffer Register
@
@       Send 1st byte. BBU uses poling with a loop timeout to monitor when the
@       byte has been sent or there is a controller timeout.
@
        mov     r3,     #(bbu_ICR_IUE | bbu_ICR_SCLE)           @ Set the IUE and SCLE bits
        orr     r3,     r3,     #(bbu_ICR_TB | bbu_ICR_START)   @ Set transfer and start bits
	    orr	    r3,	    r3,	#bbu_ICR_MODE
        str     r3,     [r4, #bbu_I2C_ICR_offset]               @ This will start the transfer
        mov     r2,     #BBU_I2C_TimeOut                @ Set up time out value
b2:       ldr     r3,     [r4, #bbu_I2C_ICR_offset]       @ Get Control register contents
        subs    r2,     r2,     #1                      @ Decrement time out value
        beq     f12                            @ Exit path if timed out
        ands    r3,     r3,     #bbu_ICR_TB     @ Has the byte been sent yet?
        bne     b2                             @ No - still being transmitted
@
@       Check I2C bus status
@
        ldr     r3,     [r4, #TWSI_SR_offset]   @ Get status register contents
        ands    r3,     r3,     #bbu_ISR_BED    @ Get status of Bus Error bit
        moveq   r2,     #0xFF                   @ Insure non-zero time out if no error
        movne   r2,     #0      @ Error - assume no ACK from device
        bne     f12            @ ...and take the exit path
        cmp     r7,     #0x100  @ Special case - Just seeing if device is there?
        moveq   r6,     #0x10                    @ YES - Set the MASTER ABORT bit
        streq   r6,     [r4, #bbu_I2C_ICR_offset]@ YES - Insure SCL and SDA go high
        beq     f12                             @ ...and exit
        cmp     r7,     #0x200  @ Special case - for reading from certain GPIO expanders?
        beq     f5             @ Yes - jump to this code
@
@       The delay is to make it a lot easier to decode transactions on the
@       bus using a logic analyzer or scope.
@
@        mov     r0,     #30     ; Set up for delay
@        bl      Delay      ; delay
@
@       Set up and then 2nd byte (Register Address)
@
        str     r7,     [r4, #bbu_I2C_IDBR_offset]              @ Set register address in the IDBR
        mov     r3,     #(bbu_ICR_TB | bbu_ICR_IUE | bbu_ICR_SCLE | bbu_ICR_STOP)
	    orr	r3,	r3,	#bbu_ICR_MODE
        str     r3,     [r4, #bbu_I2C_ICR_offset]               @ Set TB bit to start transfer

        mov     r2,     #BBU_I2C_TimeOut                @ Set up time out value
b4:       ldr     r3,     [r4, #bbu_I2C_ICR_offset]       @ Get control register contents
        subs    r2,     r2,     #1                      @ Decrement time out value
        beq     f12                                    @ Exit path if timed out
        ands    r3,     r3,  #bbu_ICR_TB                @ Was the byte sent yet?
        bne     b4                                     @ No - still being transmitted
@
@       Short delay (again, to make it easy to scope/decode the signals)
@
@5       mov     r0,     #30     ; Set up for delay
@        bl      Delay      ; delay
@
@       Set up and then send 3rd byte - Slave read address
@
f5:        orr    r6,  r6,  #1                               @ Turn the address into a slave address
        str    r6,  [r4, #bbu_I2C_IDBR_offset]            @ Load Data Buffer Register
        mov    r3,  #(bbu_ICR_TB | bbu_ICR_IUE | bbu_ICR_SCLE | bbu_ICR_START)
	orr	r3,	r3,	#bbu_ICR_MODE
        str    r3,  [r4, #bbu_I2C_ICR_offset]             @ Set TB bit to start transfer

        mov     r2,     #BBU_I2C_TimeOut        @ Set up time out value
b8:       ldr    r3,  [r4, #bbu_I2C_ICR_offset]   @ Get control register contents
        subs    r2,     r2,     #1              @ Decrement counter
        beq     f12                            @ Exit if zero
        ands   r3,  r3,  #bbu_ICR_TB            @ Was the byte sent yet?
        bne    b8                              @ No - still being transmitted
        str    r3,  [r4, #TWSI_SR_offset]       @ Write the ITE & IRF bits to clear them (sticky)
@
@       Delay (for scope analysis of I2C bus)
@
@9       mov     r0,     #30     ; Set up for delay
@        bl      Delay      ; delay
@
@       Send STOP signal (this also sends the clock signal needed to fetch the data)
@
b9:        mov    r3,  #(bbu_ICR_TB | bbu_ICR_IUE | bbu_ICR_SCLE | bbu_ICR_STOP | bbu_ICR_ACKNAK)
	orr	r3,	r3,	#bbu_ICR_MODE
        str    r3,  [r4, #bbu_I2C_ICR_offset]            @ Set TB bit to start transfer
        mov    r2,  #BBU_I2C_TimeOut                     @ Set up time out value
@
@       Loop to wait for data transfer from slave device
@
        mov     r2,     #BBU_I2C_TimeOut                @ Set up time out value
b10:      ldr     r3,     [r4, #bbu_I2C_ICR_offset]       @ Get control register contents
        subs    r2,     r2,     #1                      @ Decrement time out counter
        moveq   r2,     #1                              @ Reset to 1 if this transfer timed out
        beq     f12                                    @ Exit path if timed out
        ands    r3,     r3,     #bbu_ICR_TB             @ Was the byte received yet?
        bne     b10                                    @ no - keep looping
@
@       Some processors appear to need a little extra delay before reading the
@       the data so the next two lines of code may (or may not) be required.
@
@11      mov     r0,     #30     ; Set up for delay
@        bl      Delay      ; delay

@        ldr     r1,     [r4, #bbu_I2C_IDBR_offset]      ; Fetch data byte from the IDBR
        ldr     r0,     [r4, #bbu_I2C_IDBR_offset]      @ Fetch data byte from the IDBR
f12:      ldr     r3,     [r4, #TWSI_SR_offset]           @ Fetch status register
        str     r3,     [r4, #TWSI_SR_offset]           @ Clear any sticky bits
        ldmfd   sp!,    {r3-r7, pc}     @ Return to caller

@NOT SUPPORTED:         ENDFUNC






@
@       ******************
@       **              **
@       ** LITERAL POOL **     LOCAL DATA STORAGE
@       **              **
@       ******************
@
        .ltorg



@
@*******************************************************************************
@
@       **************
@       *            * 
@       * BBU_putCI2C * Subroutine
@       *            *
@       **************
@
@ This subroutine is used to write data to the Regular or Power I2C bus
@
@ PARAMETER PASSING:
@
@       r0 = address in target device where data is to be sent
@       r1 = data to be loaded into the register pointed to by r0
@
@ RETURNED VALUE:
@
@       r2 = non zero if no I2C bus time out
@       r2 = zero if I2C bus time out 
@
@NOT SUPPORTED: BBU_putCI2C:      FUNCTION
BBU_putCI2C:
        stmfd   sp!,    {r3-r7, lr}     @ Save used registers
        mov     r7,     r0              @ Save a copy of the slave's register
@
@       Load the base address for the I2C bus selected by the user. The regular I2C
@       bus is the default. If the user used /P for the first command switch, the
@       Power I2C bus is selected.
@


        ldr   r3,     =bbu_TWSI_PHYSICAL_BASE @ NO - Load TWSI controller base address


        mov     r2,     #0                              @ Set host controller slave address
        str     r2,     [r3, #bbu_I2C_ISAR_offset]      @ Set slave address register
        str     r2,     [r3, #bbu_I2C_ICR_offset]       @ Clear interrupts in ICR
        mov     r4,     #(bbu_ICR_IUE | bbu_ICR_SCLE)   @ Set IUE and SCLE bits
	orr	r4,	r4,	#bbu_ICR_MODE
        str     r4,     [r3, #bbu_I2C_ICR_offset]       @ Enable the I2C in ICR
@
@       Get the slave's address
@
        ldr    r5,  =bbu_BBUART_PHYSICAL_BASE       @ Fetch base address of BBUART
        ldrb   r6,  [r5, #bbu_UASPR_offset]         @ Get contents of the scratch pad register
        str    r6,  [r3, #bbu_I2C_IDBR_offset]      @ Load Data Buffer Register
@
@       Send 1st byte
@
        orr     r4,     r4,  #(bbu_ICR_TB | bbu_ICR_START)
        str     r4,     [r3, #bbu_I2C_ICR_offset]       @ Set TB and START bits (in addition to IUE & SCLE)
        mov     r2,     #BBU_I2C_TimeOut                @ Set up time out loop

a2:       ldr     r4,     [r3, #TWSI_SR_offset]           @ Get status register contents
        subs    r2,     r2,     #1                      @ Decrement time out count
        beq     a8                                     @ Timed out - return to caller
        ands    r4,     r4,     #bbu_ISR_ITE            @ Was the byte sent yet?
        beq     a2                                     @ No - still being transmitted
        str     r4,     [r3, #TWSI_SR_offset]           @ Write the ITE bit to clear it (sticky)
@
@       Set up and then send 2nd byte (ADDRESS)
@
  @      mov     r0,     #100    ; Set up for delay
  @      bl      Delay      ; delay

        str    r7,  [r3, #bbu_I2C_IDBR_offset]            @ Set register address in the IDBR
        mov    r4,  #(bbu_ICR_TB | bbu_ICR_IUE | bbu_ICR_SCLE)
	orr	r4,	r4,	#bbu_ICR_MODE
        str    r4,  [r3, #bbu_I2C_ICR_offset]             @ Set TB bit to start transfer
        mov    r2,  #BBU_I2C_TimeOut                      @ Set up time out value

a4:       ldr     r4,  [r3, #TWSI_SR_offset]      @ Get status register contents
        subs    r2,     r2,     #1              @ Decrement time out value
        beq     a8                             @ Return if there was a time out
        ands    r4,     r4,     #bbu_ISR_ITE    @ Was the byte sent yet?
        beq     a4                             @ No - still being transmitted
        str     r4,  [r3, #TWSI_SR_offset]      @ Write the ITE bit to clear it (sticky)
@
@       Set up and then send 3rd byte (DATA) and STOP signal
@
  @     mov     r0,     #100    ; Set up for delay
  @      bl      Delay      ; delay

        str     r1,     [r3, #bbu_I2C_IDBR_offset]      @ Place data byte into the IDBR
        mov     r4,     #(bbu_ICR_TB | bbu_ICR_IUE | bbu_ICR_SCLE | bbu_ICR_STOP)
	    orr	    r4,	r4,	#bbu_ICR_MODE
        str     r4,     [r3, #bbu_I2C_ICR_offset]       @ Set TB bit to start transfer

a6:       ldr     r4,     [r3, #TWSI_SR_offset]           @ Get status register contents
        ands    r4,     r4,  #bbu_ISR_ITE               @ Was the byte sent yet?
        beq     a6                                     @ No - still being transmitted
        str     r4,     [r3, #TWSI_SR_offset]           @ Write the ITE bit to clear it (sticky)

a8:      mov r0, r2
		ldmfd   sp!,    {r3-r7, pc}     @ Return to caller

@NOT SUPPORTED:         ENDFUNC


        

