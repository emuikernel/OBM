/******************************************************************************
 *
 *  (C)Copyright 2013 Marvell Hefei Branch. All Rights Reserved.
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
 ******************************************************************************/

#include "spi.h"
#include "GPIO.h"
#include "Typedef.h"
#include "Errors.h"
#include "PlatformConfig.h"
#include "xllp_dmac.h"
#include "timer.h"
#include "giga.h"
#include "macronix.h"

__attribute__ ((aligned(16)))XLLP_DMAC_DESCRIPTOR_T	TX_Desc[SSP_READ_DMA_DESC];	
		// [0]    used to send the command 
		// [1:XX] used for getting the data - just keeps the clock running.
__attribute__ ((aligned(16)))XLLP_DMAC_DESCRIPTOR_T	RX_Desc[SSP_READ_DMA_DESC];
	 	// [0]	  place holder - garbage word obtains from transmit command
		// [1:XX] used for getting the data

__attribute__ ((aligned(16)))UINT_T	 tx1_command, tx1_command2, tx_4bytes_read[2], rx_4bytes_read[2];		// contains command opcode and address
__attribute__ ((aligned(16)))UINT_T	 rx1_fromcommand;	// receive garbage clocked in while command is transmitted

__attribute__ ((aligned(16)))UINT_T	 tx2_runclocks = 0xb2b4b6ba;	// word to transmit to keep clocks running while pulling in data

__attribute__ ((aligned(16)))UINT8_T	 tx_1bytes[5], rx_1bytes[5];
UINT_T tx_DMA_cmd[2], rx_DMA_cmd[2];

UINT_T DMA_Read_Buffer= DDR_SPI_DMA_BUFFER_ADDR;
extern UINT_T MPUFlag;
extern UINT_T Flash_size;
extern UINT_T giga_32MB;

UINT_T InitializeSPIDevice(UINT8_T FlashNum, UINT8_T* P_DefaultPartitionNum)
{
	UINT_T Retval = NoError;
	UINT_T i, ID, status;
	UINT_T status_value, config_value;

	P_FlashProperties_T pFlashP = GetFlashProperties(BOOT_FLASH);

	ChipSelectSPI();

	SPINOR_Reset();

	SPINOR_ReadId(&ID);

	serial_outstr("spi id\n");
	serial_outnum(ID);
	serial_outstr("\n");

	//setup Flash Properties info
	pFlashP->BlockSize = 0x10000; // 64KB
    pFlashP->PageSize = 0x100; // 256B
	pFlashP->NumBlocks = 0x100;	// 256

	SetCurrentFlashBootType(BOOT_FLASH);

	switch (ID)
	{
		case 0x010219:
		case 0xc22539:
			Flash_size = 0x02000000; // 32MB

			pFlashP->ReadFromFlash 	= &MX_SPINOR_Read;
			pFlashP->WriteToFlash 	= &MX_SPINOR_Write;
			pFlashP->EraseFlash 	= &MX_SPINOR_Erase;

			MX_SPINOR_UnProtectBlocks();
			break;

		case 0xc86019:
			Flash_size = 0x02000000; // 32MB
			giga_32MB = 1;

			pFlashP->ReadFromFlash 	= &Giga_SPINor_Read;
			pFlashP->WriteToFlash 	= &Giga_SPINor_Write;
			pFlashP->EraseFlash 	= &Giga_SPINor_Erase;
			
			Giga_Enable4BytesMode();
			Giga_SPINor_UnProtectBlocks();
			break;

		default:

			pFlashP->ReadFromFlash 	= &SPINOR_Read;
			pFlashP->WriteToFlash 	= &SPINOR_Write;
			pFlashP->EraseFlash 	= &SPINOR_Erase;
			break;
	}
   
    pFlashP->ResetFlash = NULL;
    pFlashP->FlashSettings.UseBBM = 0;
    pFlashP->FlashSettings.UseSpareArea = 0;
    pFlashP->FlashSettings.SASize = 0;
    pFlashP->FlashSettings.UseHwEcc = 0;
	pFlashP->StreamingFlash = FALSE;
	pFlashP->StagedInitRequired = FALSE;
	pFlashP->FlashType = SPI_FLASH;
	pFlashP->FinalizeFlash = NULL;
    //---------------------------------------
	pFlashP->TimFlashAddress = 0;
	*P_DefaultPartitionNum = 0;

	return Retval;
}

void ROW_DELAY(UINT_T x)
{
	while (x > 0)
	{
		x--;
	}
}

void SPINOR_DisableSSP(void)
{
	//make sure SSP is disabled
	reg_bit_clr(SSP_CR0, SSP_CR0_SSE);
}

void SPINOR_WaitSSPComplete(void)
{
	while (*SSP_SR & (SSP_SSSR_BSY | SSP_SSSR_TFL))
	{
		ROW_DELAY(DEFAULT_TIMEOUT);
	}
}

void SPINOR_Reset(void)
{
	UINT_T temp;
	
	reg_write(SSP_CR0, SSP_CR0_DSS_8);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);
	reg_bit_set(SSP_CR0, SSP_SSCR0_SSE);
	
	reg_write(SSP_DR, SPI_CMD_RELEASE_POWER_DOWN);

	SPINOR_WaitSSPComplete();

	temp = *SSP_DR;

	SPINOR_DisableSSP();

	ROW_DELAY(1000);

	return;
}

void SPINOR_ReadStatus(UINT_T Wait)
{
	UINT_T i=0;
	UINT_T read, ready, dummy, status;

	status = NoError;
	read = FALSE;	//this flag gets set when we read first entry from fifo
	//if the caller waits to 'Wait' for the BUSY to be cleared, start READY off as FALSE
	//if the caller doesn't wait to wait, set READY as true, so we don't wait on the bit
	ready = (Wait) ? FALSE : TRUE;

	do{
		//make sure SSP is disabled
		reg_bit_clr(SSP_CR0, SSP_CR0_SSE);
		//reset SSP CR's
		reg_write(SSP_CR0, SSP_CR0_INITIAL);
		reg_write(SSP_CR1, SSP_CR1_INITIAL);
		//need to use 32bits data
		reg_bit_set(SSP_CR0, SSP_CR0_DSS_16);
		//fire it up
		reg_bit_set(SSP_CR0, SSP_CR0_SSE);

		//load the command + 1 dummy byte
		*SSP_DR = SPI_CMD_READ_STATUS << 8;

		//wait till the TX fifo is empty, then read out the status
		while((*SSP_SR & 0xF10) != 0x0);

		dummy = *SSP_DR;

		//set the READ flag, and read the status
		read = TRUE;
		status = dummy & 0xFF;	//the status will be in the second byte

		//set the READY flag if the status wait bit is cleared
		if((status & 1) == 0)		// operation complete (eg. not busy)?
			ready = TRUE;

		//make sure SSP is disabled
		reg_bit_clr(SSP_CR0, SSP_CR0_SSE);
		//reset SSP CR's
		reg_write(SSP_CR0, SSP_CR0_INITIAL);
		reg_write(SSP_CR1, SSP_CR1_INITIAL);

		//we need to wait until we read at least 1 valid status entry
		//if we're waiting for the Write, wait till WIP bits goes to 0
	}while ((!read) || (!ready));


	//return last known status
	return;
}

void SPINOR_WriteEnable(void)
{
	UINT32 temp;

	reg_write(SSP_CR0, SSP_CR0_DSS_8);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);
		
	//load the command
	reg_write(SSP_DR, SPI_CMD_WRITE_ENABLE);

	//wait till TX fifo is empty
	SPINOR_WaitSSPComplete();
	
	temp = *SSP_DR;
	
	//make sure SSP is disabled
	SPINOR_DisableSSP();
 
	return;
}


#if ZIMI_PB05
void zimi_SPIReadSpansion(UINT_T *m_temp)
{
	UINT_T temp;
	
	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL | 0x300cc3);

	//setup in 32bit mode
	reg_bit_set(SSP_CR0, SSP_CR0_DSS_32);

	//fire SSP up
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);

	__attribute__ ((aligned(16)))UINT_T	  command;

	command = (0x65<<24) | 0x04;
	reg_write(SSP_DR, command);

	SPINOR_WaitSSPComplete();
	temp = *SSP_DR;

	SPINOR_DisableSSP();

	*m_temp = temp;
}
#endif

void SPINOR_ReadId(UINT_T *pID)
{
	UINT_T ID;
	
	reg_write(SSP_CR0, SSP_CR0_DSS_32);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);
	reg_bit_set(SSP_CR0, SSP_SSCR0_SSE);
		
	reg_write(SSP_DR, SPI_CMD_JEDEC_ID << 24);
	
	SPINOR_WaitSSPComplete();
	
	ID = *SSP_DR;
	
	//make sure SSP is disabled
	//it must be executed lastly,
	//because disable SP will result in RXFIFO/TXFIFO reset.
	SPINOR_DisableSSP();

	*pID = ID;

	#if ZIMI_PB05
	extern UINT_T gChipID,isUniformSector;
	gChipID = ID;
	if(gChipID == SPANSION_16M_ID)
	{
		UINT_T m_temp;

		zimi_SPIReadSpansion(&m_temp);
		if(m_temp & (1<<3))
		{
			isUniformSector = 1;
			serial_outstr("zimi# spansion is UniformSector\n");
		}
			
	}
	#endif
	
	return;
}

UINT_T SPINOR_Page_Program_DMA(UINT_T Address, UINT_T Buffer, UINT_T Size)
{
	DMA_CMDx_T TX_Cmd, RX_Cmd;
	UINT_T start_time, Retval, i;
	UINT_T *ptr;
	unsigned char temp[260] = {0};

	UINT_T DMA_PP[66] = {0};

	//turn off UINT_Terrupts during the Read
	DisableIrqInterrupts();
	
	//because the SSP controller is crap, we need to
	//use DMA to input the five bytes of data:
	TX_Cmd.value = 0;
	TX_Cmd.bits.IncSrcAddr = 1;
	TX_Cmd.bits.IncTrgAddr = 0;
	TX_Cmd.bits.FlowSrc = 0;
	TX_Cmd.bits.FlowTrg = 1;
	TX_Cmd.bits.Width = 3;
	TX_Cmd.bits.MaxBurstSize = 2;
	//TX_Cmd.bits.Length = 8; // 8 bytes

	RX_Cmd.value = 0;
	RX_Cmd.bits.IncSrcAddr = 0;
	RX_Cmd.bits.IncTrgAddr = 0;
	RX_Cmd.bits.FlowSrc = 0;
	RX_Cmd.bits.FlowTrg = 0;
	RX_Cmd.bits.Width = 3;
	RX_Cmd.bits.MaxBurstSize = 2;
	//RX_Cmd.bits.Length = 8; // 8 bytes

	//setup DMA
	//Map Device to Channel
	XllpDmacMapDeviceToChannel(SSP_TX_DMA_DEVICE, SSP_TX_CHANNEL);
	XllpDmacMapDeviceToChannel(SSP_RX_DMA_DEVICE, SSP_RX_CHANNEL);

	//turn ON user alignment - in case buffer address is 64bit aligned
	alignChannel(SSP_TX_CHANNEL, 1);

	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL | 0x300cc3);

	//setup in 32bit mode
	reg_bit_set(SSP_CR0, SSP_CR0_DSS_32);
    
	//fire SSP up
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);

	memcpy(&temp[0], Buffer, Size );
	
	DMA_PP[0] = (SPI_CMD_PROGRAM << 24) | (Address & 0x00FFFFFF);
	ptr = (UINT_T *)temp;
	for( i = 1; i < ((Size >> 2) + 1); i++ )
	{
		DMA_PP[i] = Endian_Convert( ptr[i-1] );
	}
		
	//configDescriptor(&RX_Desc[0], &RX_Desc[1],(UINT_T)SSP_DR,			(UINT_T)&rx1_fromcommand, &RX_Cmd, 4, 0);
	//configDescriptor(&TX_Desc[0], &TX_Desc[1],(UINT_T)&tx1_command,	(UINT_T)SSP_DR, 			&TX_Cmd, 4, 0);
	configDescriptor(&RX_Desc[0], NULL,	(UINT_T)SSP_DR,	(UINT_T)&rx1_fromcommand, &RX_Cmd, Size + 4, 1);
	configDescriptor(&TX_Desc[0], NULL,	(UINT_T)DMA_PP,	(UINT_T)SSP_DR, 			&TX_Cmd, Size + 4, 1); 

	//Load descriptors
	loadDescriptor (&TX_Desc[0], SSP_TX_CHANNEL);
	loadDescriptor (&RX_Desc[0], SSP_RX_CHANNEL);

	//Kick off DMA's
	XllpDmacStartTransfer(SSP_TX_CHANNEL);
	XllpDmacStartTransfer(SSP_RX_CHANNEL);

	//timer loop waiting for dma to finish
	//setup a timer to fail gracefully in case of error
	start_time = GetOSCR0();
	start_time = GetOSCR0();
	Retval = NoError;

	//wait until the RX channel gets the stop UINT_Terrupt and the TX fifo is drained
	while( ((readDmaStatusRegister(SSP_TX_CHANNEL) & XLLP_DMAC_DCSR_STOP_INTR) != XLLP_DMAC_DCSR_STOP_INTR) &&
		   ((*SSP_SR & 0xF10) != 0x0) )
	{
		//if we've waited long enough, fail
		if( OSCR0IntervalInMilli(start_time, GetOSCR0()) > SSP_READ_TIME_OUT_MILLI )
		{
			Retval = TimeOutError;
			break;
		}
	}

	//if we errored out, kill the DMA transfers
	if(Retval != NoError)
	{
		XllpDmacStopTransfer( SSP_TX_CHANNEL );
		XllpDmacStopTransfer( SSP_RX_CHANNEL );
	}

	SPINOR_WaitSSPComplete();

	//make sure SSP is disabled
	SPINOR_DisableSSP();

	//clear out DMA settings
	XllpDmacUnMapDeviceToChannel(SSP_TX_DMA_DEVICE, SSP_TX_CHANNEL);
	XllpDmacUnMapDeviceToChannel(SSP_RX_DMA_DEVICE, SSP_RX_CHANNEL);

	//turn UINT_Terrupts back on
	EnableIrqInterrupts();

	return Retval;
}

UINT_T SPINOR_Read_DMA(UINT_T FlashOffset, UINT_T Buffer, UINT_T Size, UINT_T CopySize)
{
	UINT_T i;
	UINT_T	*buff, *temp_buff;
	UINT_T	read_size, total_size, start_time, Retval, un_read_size, read_buff;
	DMA_CMDx_T		RX_Cmd, TX_Cmd;

	if ((FlashOffset + Size - 1) > 0xFFFFFF)   //xyl: minus 1 for 16MB download bug
		return FlashAddrOutOfRange;

	//turn off UINT_Terrupts during the Read
	DisableIrqInterrupts();

	// * initialize variables * //
	//save off the buffer poUINT_Ter - for endian convert at end of routine
	buff = (UINT_T *)Buffer;
	//remember how much (in words) is Read - for endian convert at the end of routine
	total_size = Size >> 2;

	read_buff = DMA_Read_Buffer;

	//tx0 - load the SPI command and address (0xCmd_AddrHI_AddrMid_AddrLow)
	//tx1_command  = (pSPI_commands->SPI_READ_CMD << 24) | (FlashOffset & 0x00FFFFFF);

	//fill out commands
	RX_Cmd.value = 0;
	RX_Cmd.bits.IncSrcAddr = 0;
	RX_Cmd.bits.IncTrgAddr = 1;
	RX_Cmd.bits.FlowSrc = 1;
	RX_Cmd.bits.FlowTrg = 0;
	RX_Cmd.bits.Width = 3;
	RX_Cmd.bits.MaxBurstSize = 2;

	TX_Cmd.value = 0;
	TX_Cmd.bits.IncSrcAddr = 0;
	TX_Cmd.bits.IncTrgAddr = 0;
	TX_Cmd.bits.FlowSrc = 0;
	TX_Cmd.bits.FlowTrg = 1;
	TX_Cmd.bits.Width = 3;
	TX_Cmd.bits.MaxBurstSize = 2;

	//setup DMA
	//Map Device to Channels
	XllpDmacMapDeviceToChannel(SSP_RX_DMA_DEVICE, SSP_RX_CHANNEL);
	XllpDmacMapDeviceToChannel(SSP_TX_DMA_DEVICE, SSP_TX_CHANNEL);

	//turn ON user alignment - in case buffer address is 64bit aligned
	alignChannel(SSP_RX_CHANNEL, 1);

	//reset SSP CR's
	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL | 0x300cc3);

	//setup in 32bit mode
	reg_bit_set(SSP_CR0, SSP_CR0_DSS_32);

	//fire SSP up
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);

	Retval = NoError;

	//Size = Size + 4;
	un_read_size=Size +12;

	//for each loop iteration, one Read Command will be issued with a link descriptor chain
	//	the chain will read a total of (SSP_READ_DMA_DESC-1)*SSP_READ_DMA_SIZE bytes from SPI

	//while (Size > 0)
	{
		// * configure RX & TX descriptors * //
		//initial 1 word transfer:
		//		TX: send command+address
		//		RX: receive dummy word
		tx1_command  = (SPI_CMD_READ << 24) | (FlashOffset & 0x00FFFFFF);
		
		//configDescriptor(&RX_Desc[0], &RX_Desc[1],(UINT_T)SSP_DR,			(UINT_T)&rx1_fromcommand, &RX_Cmd, 4, 0);
		//configDescriptor(&TX_Desc[0], &TX_Desc[1],(UINT_T)&tx1_command,	(UINT_T)SSP_DR, 			&TX_Cmd, 4, 0);
		
		i = 0;
		//chaining of full transfers (descriptors that are not "stop"ped)
		//fill out descriptors until either:
		//	- there is only enough data for 1 more descriptor (which needs to be the "stopped" descriptor)
		//	- the pool of descriptors is depleted (minus 1 because the last needs to be "stopped")
		while( (un_read_size > SSP_READ_DMA_SIZE) && (i < (SSP_READ_DMA_DESC-1)) )
		{
			configDescriptor(&RX_Desc[i], &RX_Desc[i+1],	(UINT_T)SSP_DR,			(UINT_T)read_buff, &RX_Cmd, SSP_READ_DMA_SIZE, 0);
			configDescriptor(&TX_Desc[i], &TX_Desc[i+1],	(UINT_T)&tx1_command,	(UINT_T)SSP_DR, &TX_Cmd, SSP_READ_DMA_SIZE, 0);

			//Update counters
			read_buff 		+=	SSP_READ_DMA_SIZE;
			//FlashOffset +=	SSP_READ_DMA_SIZE;
			un_read_size 		-=	SSP_READ_DMA_SIZE;			
			i++;
			
		}

		//last link: descriptor must be "stopped"
		read_size 	=  un_read_size > SSP_READ_DMA_SIZE ? SSP_READ_DMA_SIZE : un_read_size;
		configDescriptor(&RX_Desc[i], NULL,	(UINT_T)SSP_DR,			(UINT_T)read_buff, &RX_Cmd, read_size, 1);
		configDescriptor(&TX_Desc[i], NULL,	(UINT_T)&tx1_command,	(UINT_T)SSP_DR, &TX_Cmd, read_size, 1);

		//update counters after "stop" descriptor
		read_buff 		+= read_size;
		//FlashOffset += read_size; 
		un_read_size 		-= read_size;

		//Load descriptors
		loadDescriptor (&RX_Desc[0], SSP_RX_CHANNEL);
		loadDescriptor (&TX_Desc[0], SSP_TX_CHANNEL);

		//Kick off DMA's
		XllpDmacStartTransfer(SSP_RX_CHANNEL);
		XllpDmacStartTransfer(SSP_TX_CHANNEL);

		//setup a timer to fail gracefully in case of error
		start_time = GetOSCR0();
		start_time = GetOSCR0();

		//wait until the RX channel gets the stop UINT_Terrupt and the TX fifo is drained
		while( ((readDmaStatusRegister(SSP_RX_CHANNEL) & XLLP_DMAC_DCSR_STOP_INTR) != XLLP_DMAC_DCSR_STOP_INTR) &&
			   ((*SSP_SR & 0xF10) != 0x0) )
		{
			//if we've waited long enough, fail
			if( OSCR0IntervalInMilli(start_time, GetOSCR0()) > SSP_READ_TIME_OUT_MILLI )
			{
				Retval = TimeOutError;
				break;
			}
		}
		//if(Retval != NoError)
		//	break;

		SPINOR_WaitSSPComplete();
	}

	//if we errored out, kill the DMA transfers
	if(Retval != NoError)
	{
		XllpDmacStopTransfer( SSP_RX_CHANNEL );
		XllpDmacStopTransfer( SSP_TX_CHANNEL );
	}

	//make sure SSP is disabled
	reg_bit_clr(SSP_CR0, SSP_CR0_SSE);
	//reset SSP CR's
	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);

	//clear out DMA settings
	XllpDmacUnMapDeviceToChannel(SSP_RX_DMA_DEVICE, SSP_RX_CHANNEL);
	XllpDmacUnMapDeviceToChannel(SSP_TX_DMA_DEVICE, SSP_TX_CHANNEL);

	//turn UINT_Terrupts back on
	EnableIrqInterrupts();

	temp_buff = (UINT_T *)DMA_Read_Buffer;
	for( i = 0; i < total_size + 2; i ++ )	
	{		
		temp_buff[i] = Endian_Convert( temp_buff[i] );
	}
	//temp_buff[i] = Endian_Convert( temp_buff[i] ); // last byte

	memcpy( (UINT8_T *)Buffer, (UINT8_T *)(DMA_Read_Buffer + 4), CopySize );

	//postprocessing... endian convert
	//for(i = 0; i < total_size; i++)
	//	buff[i] = Endian_Convert(buff[i]);

	return Retval;
}

UINT_T SPINOR_Read(UINT_T FlashOffset, UINT_T Buffer, UINT_T Size)
{
	UINT_T Retval, i, total_size, read_size, status;

	do {

		read_size = Size > SIZE_64KB ? SIZE_64KB : Size;
	
		Retval = SPINOR_Read_DMA(FlashOffset, Buffer, SIZE_64KB, read_size);
	#if DECOMPRESS_SUPPORT
		if (MPUFlag == 1) // only for boot
			CacheInvalidateMemory(DMA_Read_Buffer, SIZE_64KB + 4);
	#endif
		//update counters
		FlashOffset+=read_size;
		Buffer+=read_size;
		Size-=read_size;

	} while( (Size > 0) && (Retval == NoError) );

	return Retval;
}

#if ZIMI_PB05   //xyl re-write eraseSector
UINT_T SPINOR_EraseSector(UINT_T secAddr, UINT_T cmdSelect)
{
	UINT_T temp, command, CMDSelect;
	
	SPINOR_ReadStatus(TRUE);
	SPINOR_WriteEnable();
	SPINOR_ReadStatus(TRUE);

	switch (cmdSelect)
	{
		case 0:
		//	serial_outstr("4KB erase command 0x20\n");
			CMDSelect = 0x20;
			break;

		case 1:
		//	serial_outstr("32KB erase command 0xd8\n");
			CMDSelect = 0xd8;
			break;

		default:
		//	serial_outstr("64KB erase command 0xd8\n");
			CMDSelect = 0xd8;
			break;
	}
	
	command  = CMDSelect << 24;
	command |= secAddr & 0xFFFFFF;

	reg_write(SSP_CR0, SSP_CR0_DSS_32);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);
	
	reg_write(SSP_DR, command);

	//wait for TX fifo to empty AND busy signal to go away
	SPINOR_WaitSSPComplete();
	
	temp = *SSP_DR;
	
	//make sure SSP is disabled
	SPINOR_DisableSSP();

	SPINOR_ReadStatus(TRUE);

	return NoError;
}
#endif

UINT_T SPINOR_Wipe(void)
{
	UINT_T temp;

	//make sure the device is ready for the command
	SPINOR_ReadStatus(TRUE);
	SPINOR_WriteEnable();
	SPINOR_ReadStatus(TRUE);

	// sequence for issuing the wipe command (aka chip erase, aka bulk erase)
	//make sure SSP is disabled
	reg_write(SSP_CR0, SSP_CR0_DSS_8);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);

	// write the command to the fifo. this starts the spi clock running and the command appears on the bus.
	reg_write(SSP_DR, SPI_CMD_CHIP_ERASE);

	//wait for TX fifo to empty AND busy signal to go away
	SPINOR_WaitSSPComplete();

	temp = *SSP_DR;

	//make sure SSP is disabled
	SPINOR_DisableSSP();

	SPINOR_ReadStatus(TRUE);
	
	return NoError;
}

UINT_T SPINOR_Write(UINT_T Address, UINT_T Buffer, UINT_T Size)
{
	UINT_T Retval, i, total_size, write_size, status;
	UINT_T *buff = (UINT_T *) Buffer;

	if(Size & 0x3)    //make size 4bytes-align
    	Size = (Size+4)&(~3);

	total_size = Size >> 2;

	//postprocessing... endian convert
	if ((Address + Size -1) > 0xFFFFFF)  //xyl: minus 1 for 16MB download bug
		return FlashAddrOutOfRange;
	
	//for(i = 0; i < total_size; i++)
	//	buff[i] = Endian_Convert(buff[i]);

	do {
		//make sure the device is ready to be written to
		SPINOR_ReadStatus(TRUE);
		//get device ready to Program
		SPINOR_WriteEnable();
        SPINOR_ReadStatus(TRUE);

		write_size = Size > WRITE_SIZE ? WRITE_SIZE : Size;
        
		//write a byte
		if (write_size == WRITE_SIZE)
		{
			Retval = SPINOR_Page_Program_DMA(Address, Buffer, WRITE_SIZE);
			
			//update counters
			Address+=WRITE_SIZE;
			Buffer+=WRITE_SIZE;
			Size-=WRITE_SIZE;
		}
		else
		{
			Retval = SPINOR_Page_Program_DMA(Address, Buffer, WRITE_SIZE);
			Size=0;
		}

		SPINOR_ReadStatus(TRUE);

	} while( (Size > 0) && (Retval == NoError) );

	return Retval;
}

#if ZIMI_PB05   //xyl re-write erase func
UINT_T SPINOR_Erase(UINT_T Address, UINT_T Size)
{
	UINT_T numSectors, i, sector_size, Retval = NoError, zero_address;
	
	P_FlashProperties_T pFlashP = GetFlashProperties(BOOT_FLASH);

	if ((Address + Size -1) > 0xFFFFFF)  //xyl: minus 1 for 16MB download bug
		return FlashAddrOutOfRange;

	extern UINT_T gChipID, isUniformSector;
	if(gChipID == SPANSION_16M_ID && isUniformSector == 0)
	{
		zero_address = Address;

		// specific case for spansion spi-nor
		if (zero_address == 0)
		{
			for (i = 0; i < 8; i++)
			{
				SPINOR_EraseSector(Address, 0);
				Address += 0x1000;
			}

			SPINOR_EraseSector(Address, 1);
			Address += 0x8000;
		}
	}

	sector_size = pFlashP->BlockSize;

	if (Size % pFlashP->BlockSize == 0)
		numSectors = Size / pFlashP->BlockSize;
	else
		numSectors = Size / pFlashP->BlockSize + 1;

	if(gChipID == SPANSION_16M_ID && isUniformSector == 0)
	{
		if (zero_address == 0)
			numSectors -= 1;
	}
	
	for (i = 0; i < numSectors; i++)
	{
		//erase this sector
		Retval = SPINOR_EraseSector(Address, 2);

		Address += sector_size;

		if (Retval != NoError)
			break;
	}

	return Retval;
}
#endif
