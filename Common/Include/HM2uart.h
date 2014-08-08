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
 
 ******************************************************************************/



 /******************** (c) Marvell Semiconductor, Inc., 2002 *******************
 *
 * $Header: /VOIP8618/HM2_Bootrom/Download/HM2uart.h,v 1.1.1.1 2007/09/11 00:56:21 sswain Exp $
 *
 * Purpose:
 *    This file contains the function prototypes and definitions for the
 *    UART setup code.
 *
 * Public Procedures:
 *    get_char                 Read a char from the 16550 UART block. 
 *                            When one is avail.
 *    put_char                 Write a u8 to the 16550.
 *    setup_uart              Prepare the 16550 for 38400 NP 1stop also
 *                            set the pin mux to UART from USB
 *
 * Notes:
 *    None.
 *
 *****************************************************************************/

#ifndef _HS35UART_H_
#define _HS35UART_H_


#define	bit(x)			(0x0001<<x)

// fl: no doc #define SI_DIV			0x8000c008

#define UART_BASE		0x8000c840 	/* voip */ 

#define Rx_BUFFER_REG		(UART_BASE + (4*0))
#define Tx_HOLDING_REG		(UART_BASE + (4*0))
#define DIVISOR_LOW		(UART_BASE + (4*0)) /* When LINE_CONTROL[7] == 1 */
#define INTR_ENABLE		(UART_BASE + (4*1))
#define DIVISOR_HIGH		(UART_BASE + (4*1)) /* When LINE_CONTROL[7] == 1 */
#define IRQ_STATUS		(UART_BASE + (4*2))
#define LINE_CONTROL		(UART_BASE + (4*3))
#define  DLAB 			bit(7)		    /* turns on access to baud rate divisor */
#define  ASSERT_BREAK	        bit(6)
#define  STICKY_PARITY		bit(5)
#define  EPS 			bit(4)
#define  PEN 			bit(3)
#define  TWO_STOP_BITS		bit(2)
#define  FIVE_BIT	        0	                
#define  SIX_BIT	        bit(0)
#define  SEVEN_BIT	        bit(1)
#define  EIGHT_BIT	        (bit(1) | bit(0))

#define MODEM_CONTROL		(UART_BASE + (4*4))
#define LOOP_BACK			bit(4)
#define LINE_STATUS		(UART_BASE + (4*5))
#define  Rx_FIFO_ERROR		bit(7) 	
#define  Tx_EMPTY		bit(6) 
#define  Tx_HOLD_EMPTY		bit(5) 
#define  BREAK_DETECT		bit(4) 
#define  FRAMING_ERROR		bit(3) 
#define  PARITY_ERROR		bit(2) 
#define  OVERRUN_ERROR		bit(1) 
#define  DATA_READY		bit(0) 
#define MODEM_STATUS		(UART_BASE + (4*6))
#define SCRATCH_REG		(UART_BASE + (4*7))

#define CPU_SPEED    		44E6    // Make SURE this is correct!!!
#define SERIAL_SPEED  		(176E6/6.0)    // Make SURE this is correct!!!

/* retry loops before we resend the NAK. 10sec. */
/* 4131e-9 is the delay time of the read loop */
#define NAK_TIMEOUT_VAL    	(int)((10.0 / (4131.0e-9 * 65536.0)) + 0.5)  

#define BAUDRATE     		115200 // 38400
/* Check here. If 8Mhz && 38400 -> DIV :=: 13 */
#define BAUDRATE_DIV 		( (SERIAL_SPEED / (16 * BAUDRATE)) + 0.5) 



/******************************************************************************
 *
 * Name: put_char
 *
 * Description:
 *   Put a char into the UART transmit buffer. Wait until that buffer
 *   is ready. No timeout here, assume we can always be ready to send
 *   after a wait. 
 *
 * Conditions For Use:
 *   GPIO Clock should be enabled before attempting to use. (In PAU).
 *
 * Arguments:
 *   Arg1( u8 ): 8 bits of data to be placed in the 16550 xmit buffer.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
int put_char (int u8c);

/******************************************************************************
 *
 * Name: setup_uart
 *
 * Description:
 *   Place the 16550 into a useable state for us. This means the the
 *   baud rate needs to be 38400, no parity, one stop bit. Also the
 *   pad mux needs to get flipped from the default of USB to UART. 
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/

void change_baudrate( unsigned long baudrate );


//quick watch for the in and out byte functions:
void outbyte(int c);


#endif /* _SETUP_UART_H_ */
