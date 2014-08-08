#define __REG(x)	(*((volatile unsigned int *)(x)))

#define UART_BASE	0xD4036000 // For UART 3 FFUART

#define SerialDATA  __REG(UART_BASE+0x00)  /* Receive Buffer Register (read only) */
#define SerialFIFO  __REG(UART_BASE+0x08)  /* FIFO Control Register (write only) */
#define SerialLCR   __REG(UART_BASE+0x0c)  /* Line Control Register (read/write) */
#define SerialMCR   __REG(UART_BASE+0x10)  /* Modem Control Register (read/write) */
#define SerialLSR   __REG(UART_BASE+0x14)  /* Line Status Register (read only) */
#define SerialMSR   __REG(UART_BASE+0x18)  /* Modem Status Register (read only) */
#define SerialSCR   __REG(UART_BASE+0x1c)  /* Scratchpad Register*/
#define SerialIER   __REG(UART_BASE+0x04)  /* Interrupt Enable Register (read/write) */
#define SerialABR   __REG(UART_BASE+0x28)  

#define IER_DMAE	(1 << 7)	/* DMA Requests Enable */
#define IER_UUE		(1 << 6)	/* UART Unit Enable */
#define IER_NRZE	(1 << 5)	/* NRZ coding Enable */
#define IER_RTIOE	(1 << 4)	/* Receiver Time Out Interrupt Enable */
#define IER_MIE		(1 << 3)	/* Modem Interrupt Enable */
#define IER_RLSE	(1 << 2)	/* Receiver Line Status Interrupt Enable */
#define IER_TIE		(1 << 1)	/* Transmit Data request Interrupt Enable */
#define IER_RAVIE	(1 << 0)	/* Receiver Data Available Interrupt Enable */

#define IIR_FIFOES1	(1 << 7)	/* FIFO Mode Enable Status */
#define IIR_FIFOES0	(1 << 6)	/* FIFO Mode Enable Status */
#define IIR_TOD		(1 << 3)	/* Time Out Detected */
#define IIR_IID2	(1 << 2)	/* Interrupt Source Encoded */
#define IIR_IID1	(1 << 1)	/* Interrupt Source Encoded */
#define IIR_IP		(1 << 0)	/* Interrupt Pending (active low) */
#define LCR_DLAB	(1 << 7)	/* Divisor Latch Access Bit */
#define LCR_SB		(1 << 6)	/* Set Break */
#define LCR_STKYP	(1 << 5)	/* Sticky Parity */
#define LCR_EPS		(1 << 4)	/* Even Parity Select */
#define LCR_PEN		(1 << 3)	/* Parity Enable */
#define LCR_STB		(1 << 2)	/* Stop Bit */
#define LCR_WLS1	(1 << 1)	/* Word Length Select */
#define LCR_WLS0	(1 << 0)	/* Word Length Select */

#define LSR_FIFOE	(1 << 7)	/* FIFO Error Status */
#define LSR_TEMT	(1 << 6)	/* Transmitter Empty */
#define LSR_TDRQ	(1 << 5)	/* Transmit Data Request */
#define LSR_BI		(1 << 4)	/* Break Interrupt */
#define LSR_FE		(1 << 3)	/* Framing Error */
#define LSR_PE		(1 << 2)	/* Parity Error */
#define LSR_OE		(1 << 1)	/* Overrun Error */
#define LSR_DR		(1 << 0)	/* Data Ready */

#define BIT0        0x00000001
#define BIT1        0x00000002
#define BIT2        0x00000004
#define BIT3        0x00000008
#define BIT4        0x00000010
#define BIT5        0x00000020
#define BIT6        0x00000040
#define BIT7        0x00000080
const char digits[] = "0123456789abcdef";
void serial_init(void);
int serial_poll(void);
int serial_read(void);
int serial_write(int c);
void serial_outstr(const char *s);

void serial_init(void)
{
/*115200	unsigned int divisor = 24; */
	//unsigned int divisor = 31; // 31 for 38400 baudrate; 
	unsigned int divisor = 8;
	int i;
	
	/* switch receiver and transmitter off */
	SerialLCR = 0;
	SerialIER = 0;
	//SerialFIFO = 0;
	/* rest tx/rx/ FIFOs*/
	SerialFIFO = BIT2 | BIT1;
	SerialSCR = 0;
	
	/* read this to clear them*/
	i = SerialDATA;
	i = SerialLSR;
	i = SerialMSR;
	
	/* 1 stop bit, 8 bit character */
	SerialLCR = LCR_WLS0 | LCR_WLS1 | LCR_DLAB;

	/* Load baud rate divisor in two steps, lsb, then msb of value */
	SerialDATA = divisor & 0xff;
	SerialIER = (divisor >> 8) & 0xff;
	
	/* set the port to sensible defaults (no break, no interrupts,
	 * no parity, 8 databits, 1 stopbit, transmitter and receiver
	 * enabled), reset dlab bit:
	 */
	//SerialLCR = LCR_WLS1 | LCR_WLS0;
	/* gain access*/
	SerialLCR &= 0x7f;
	SerialMCR |= BIT3; // enable UART interrupte
	
	/*enable and clear FIFOs and set INT trigger level to be 8 bytes*/
	SerialFIFO = BIT0 | BIT3 | BIT6;

	/* turn the receiver and transmitter back on */
	/* enable UART and individual interruptes*/
	SerialIER = BIT0 | BIT2 | BIT6;
}


/* check if there is a character available to read. returns 1 if there
 * is a character available, 0 if not, and negative error number on
 * failure */
int serial_poll(void)
{
	/* check for errors */
	if(SerialLSR & (LSR_FE | LSR_PE | LSR_OE))
		return -1;

	if(SerialLSR & LSR_DR)
		return 1;
	else
		return 0;
}


/* read one character from the serial port. return character (between
 * 0 and 255) on success, or negative error number on failure. this
 * function is blocking */
int serial_read(void)
{
	int rv;

	for(;;) {
		rv = serial_poll();

		if(rv < 0)
			return rv;

		if(rv > 0)
			return SerialDATA & 0xff;
	}
}


/* write character to serial port. return 0 on success, or negative
 * error number on failure. this function is blocking
 */
int serial_write(int c)
{
	/* wait for room in the transmit FIFO */
	while((SerialLSR & LSR_TDRQ) == 0) {
	}

	SerialDATA = c & 0xff;

	return 0;
}

void serial_outstr(const char *s)
{
	while(*s != 0)
		serial_write(*s++);
	serial_write('\r');
} 

void serial_outnum(unsigned int n)
{
	//const char digits[] = "0123456789abcdef";
	int i;

	serial_write('0');
	serial_write('x');
	for(i=7; i>=0; i--)
		serial_write(digits[(n>>(i*4))&0xf]);
} 



