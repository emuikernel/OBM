/******************************************************************************
 *
 *  (C)Copyright 2014 Marvell Hefei Branch. All Rights Reserved.
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

#include "macronix.h"

extern XLLP_DMAC_DESCRIPTOR_T	TX_Desc[SSP_READ_DMA_DESC];	
		// [0]    used to send the command 
		// [1:XX] used for getting the data - just keeps the clock running.
extern XLLP_DMAC_DESCRIPTOR_T	RX_Desc[SSP_READ_DMA_DESC];
	 	// [0]	  place holder - garbage word obtains from transmit command
		// [1:XX] used for getting the data

extern UINT_T	 tx1_command, tx1_command2, tx_4bytes_read[2], rx_4bytes_read[2];		// contains command opcode and address
extern UINT_T	 rx1_fromcommand;	// receive garbage clocked in while command is transmitted

extern UINT_T	 tx2_runclocks;	// word to transmit to keep clocks running while pulling in data

extern UINT8_T	 tx_1bytes[5], rx_1bytes[5];
extern UINT_T DMA_Read_Buffer;
extern UINT_T MPUFlag;

UINT8_T MX_SPINOR_ReadReg(MX_Register reg)
{
	UINT_T dummy;
	UINT8_T status, command;

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
	switch (reg)
	{
		case MX_Status_Register:
			command = MX_SPI_CMD_READ_STATUS_REG;
			break;

		case MX_Config_Register:
			command = MX_SPI_CMD_READ_CONFIG_REG;
			break;

		case MX_Security_Register:
			command = MX_SPI_CMD_READ_SECURITY_REG;
			break;

		default:
			return NoError;
	}
	
	*SSP_DR = command << 8;

	//wait till the TX fifo is empty, then read out the status
	while((*SSP_SR & 0xF10) != 0x0);

	dummy = *SSP_DR;
	status = dummy & 0xFF;	//the status will be in the second byte

	//make sure SSP is disabled
	reg_bit_clr(SSP_CR0, SSP_CR0_SSE);
	//reset SSP CR's
	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);

	//return last known status
	return status;
}

UINT_T MX_SPINOR_CheckStatus(void)
{
	UINT8_T status;

	status = MX_SPINOR_ReadReg(MX_Security_Register);
	//serial_outstr("security register value\n");
	//serial_outnum(status);
	//serial_outstr("\n");
	if ((status & MX_CONFIG_PFAIL) == MX_CONFIG_PFAIL)
	{
		serial_outstr("Program fail\n");
		return SPINORPROGRAMFAIL;
	}

	if ((status & MX_CONFIG_EFAIL) == MX_CONFIG_EFAIL)
	{
		serial_outstr("Erase fail\n");
		return SPINORERASEFAIL;
	}

	return NoError;
}

void MX_SPINOR_WriteStatus(unsigned char status, unsigned char config)
{
	unsigned int temp;

	//reset SSP CR's
	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);

	//need to use 24bit data
	reg_bit_set(SSP_CR0, SSP_CR0_DSS_24);
	//fire it up
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);

	temp = MX_SPI_CMD_WRITE_STATUS_REG << 16;
	temp |= status << 8;
	temp |= config;
	
	reg_write(SSP_DR, temp);
	
	SPINOR_WaitSSPComplete();
	
	temp = *SSP_DR;
	
	//make sure SSP is disabled
	reg_bit_clr(SSP_CR0, SSP_CR0_SSE);
	//reset SSP CR's
	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL);

	return;
}

void MX_SPINOR_Protect_Blocks(void)
{
	UINT_T status_value, config_value;

	// make sure it's ready
	SPINOR_ReadStatus(TRUE);

	// read out status register and configuration register
	status_value = MX_SPINOR_ReadReg(MX_Status_Register);
	config_value = MX_SPINOR_ReadReg(MX_Config_Register);

	if (((config_value & MX_CONFIG_TB) == MX_CONFIG_TB)
		&& ((status_value & (MX_STATUS_BP0 | MX_STATUS_BP1 | MX_STATUS_BP2| MX_STATUS_BP3)) == MX_STATUS_BP1))
	{
		serial_outstr("SPI Nor is already protected\n");
		return;
	}
	else
	{
		serial_outstr("SPI Nor is not protected\n");
	}
	
	SPINOR_WriteEnable();

	// clear BP0/BP1/BP2/BP3
	status_value &= ~(MX_STATUS_BP0 | MX_STATUS_BP1 | MX_STATUS_BP2| MX_STATUS_BP3);

	// set BP1
	status_value |= MX_STATUS_BP1;

	// select bottom area protect
	config_value |= MX_CONFIG_TB;

	MX_SPINOR_WriteStatus(status_value, config_value);
	SPINOR_ReadStatus(TRUE);
	
	status_value = MX_SPINOR_ReadReg(MX_Status_Register);
	config_value = MX_SPINOR_ReadReg(MX_Config_Register);

	serial_outstr("SPI Nor protect done\n");
}

void MX_SPINOR_UnProtectBlocks(void)
{
	UINT_T status_value, config_value;
	
	SPINOR_ReadStatus(TRUE);
	status_value = MX_SPINOR_ReadReg(MX_Status_Register);
	config_value = MX_SPINOR_ReadReg(MX_Config_Register);

	if ((status_value & (MX_STATUS_BP0 | MX_STATUS_BP1 | MX_STATUS_BP2| MX_STATUS_BP3)) == 0)
	{
		serial_outstr("SPI Nor is already unprotected\n");
		return;
	}
	else
	{
		serial_outstr("SPI Nor is not unprotected\n");
	}
	
	SPINOR_WriteEnable();

	// clear BP0/BP1/BP2/BP3
	status_value &= ~(MX_STATUS_BP0 | MX_STATUS_BP1 | MX_STATUS_BP2| MX_STATUS_BP3);
	
	MX_SPINOR_WriteStatus(status_value, config_value);	
	
	SPINOR_ReadStatus(TRUE);
	
	status_value = MX_SPINOR_ReadReg(MX_Status_Register);
	config_value = MX_SPINOR_ReadReg(MX_Config_Register);

	serial_outstr("SPI Nor unprotect done\n");
}

UINT_T MX_Read(UINT_T FlashOffset, UINT_T Buffer, UINT_T Size, UINT_T CopySize)
{
	UINT_T i;
	UINT_T	*buff, *temp_buff;
	UINT_T	read_size, total_size, start_time, Retval, un_read_size, read_buff;
	DMA_CMDx_T		RX_Cmd, TX_Cmd;

	//turn off UINT_Terrupts during the Read
	DisableIrqInterrupts();

	// * initialize variables * //
	//save off the buffer poUINT_Ter - for endian convert at end of routine
	buff = (UINT_T *)Buffer;
	//Read_buff = DMA_Read_Buffer;
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

	//for each loop iteration, one Read Command will be issued with a link descriptor chain
	//	the chain will read a total of (SSP_READ_DMA_DESC-1)*SSP_READ_DMA_SIZE bytes from SPI

	un_read_size=Size +12;
	//un_read_size = Size;

	//while (Size > 0)
	{
		// * configure RX & TX descriptors * //
		//initial 1 word transfer:
		//		TX: send command+address
		//		RX: receive dummy word
		tx_4bytes_read[0] = (MX_SPI_CMD_READ << 24) | (FlashOffset >> 8);
		tx_4bytes_read[1] = (FlashOffset & 0xFF) << 24;
		
		//configDescriptor(&RX_Desc[0], &RX_Desc[1],(UINT_T)SSP_DR,			(UINT_T)&rx1_fromcommand, &RX_Cmd, 4, 0);
		//configDescriptor(&TX_Desc[0], &TX_Desc[1],(UINT_T)&tx1_command,	(UINT_T)SSP_DR, 			&TX_Cmd, 4, 0);
		//Buffer -=1;
		
		i = 0;
		//chaining of full transfers (descriptors that are not "stop"ped)
		//fill out descriptors until either:
		//	- there is only enough data for 1 more descriptor (which needs to be the "stopped" descriptor)
		//	- the pool of descriptors is depleted (minus 1 because the last needs to be "stopped")
		while( (un_read_size > SSP_READ_DMA_SIZE) && (i < (SSP_READ_DMA_DESC-1)) )
		{
			configDescriptor(&RX_Desc[i], &RX_Desc[i+1],	(UINT_T)SSP_DR,			(UINT_T)read_buff, &RX_Cmd, SSP_READ_DMA_SIZE, 0);
			configDescriptor(&TX_Desc[i], &TX_Desc[i+1],	(UINT_T)&tx_4bytes_read,	(UINT_T)SSP_DR, &TX_Cmd, SSP_READ_DMA_SIZE, 0);

			//Update counters
			read_buff 		+=	SSP_READ_DMA_SIZE;
			//FlashOffset +=	SSP_READ_DMA_SIZE;
			un_read_size 		-=	SSP_READ_DMA_SIZE;			
			i++;
			
		}

		//last link: descriptor must be "stopped"
		read_size 	=  un_read_size > SSP_READ_DMA_SIZE ? SSP_READ_DMA_SIZE : un_read_size;
		configDescriptor(&RX_Desc[i], NULL,	(UINT_T)SSP_DR,			(UINT_T)read_buff, &RX_Cmd, read_size, 1);
		configDescriptor(&TX_Desc[i], NULL,	(UINT_T)&tx_4bytes_read,	(UINT_T)SSP_DR, &TX_Cmd, read_size, 1);

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

	memcpy( (UINT8_T *)Buffer, (UINT8_T *)(DMA_Read_Buffer + 5), CopySize );

	//postprocessing... endian convert
	//for(i = 0; i < total_size; i++)
	//	buff[i] = Endian_Convert(buff[i]);

	return Retval;
}

UINT_T MX_SPINOR_Read(UINT_T FlashOffset, UINT_T Buffer, UINT_T Size)
{
	UINT_T Retval, i, total_size, read_size, status;

	do {

		read_size = Size > SIZE_64KB ? SIZE_64KB : Size;
	
		Retval = MX_Read(FlashOffset, Buffer, SIZE_64KB, read_size);
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

UINT_T MX_Write(UINT_T Address, UINT_T Buffer, UINT_T Size)
{
	DMA_CMDx_T TX_Cmd, RX_Cmd;
	UINT_T start_time, Retval, i;
	unsigned char temp[260] = {0};
	UINT_T *ptr;

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
	TX_Cmd.bits.MaxBurstSize = 1;
	//TX_Cmd.bits.Length = 8; // 8 bytes

	RX_Cmd.value = 0;
	RX_Cmd.bits.IncSrcAddr = 0;
	RX_Cmd.bits.IncTrgAddr = 0;
	RX_Cmd.bits.FlowSrc = 0;
	RX_Cmd.bits.FlowTrg = 0;
	RX_Cmd.bits.Width = 3;
	RX_Cmd.bits.MaxBurstSize = 1;
	//RX_Cmd.bits.Length = 8; // 8 bytes

	//setup DMA
	//Map Device to Channel
	XllpDmacMapDeviceToChannel(SSP_TX_DMA_DEVICE, SSP_TX_CHANNEL);
	XllpDmacMapDeviceToChannel(SSP_RX_DMA_DEVICE, SSP_RX_CHANNEL);

	//turn ON user alignment - in case buffer address is 64bit aligned
	alignChannel(SSP_TX_CHANNEL, 1);

	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL | 0x300083);

	//setup in 32bit mode
	reg_bit_set(SSP_CR0, SSP_CR0_DSS_32);
    
	//fire SSP up
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);

	temp[0] = Address & 0xFF;
	memcpy(&temp[1], Buffer, Size );
	memcpy(&temp[Size+1], Buffer, 3 );

	DMA_PP[0] = (MX_SPI_CMD_PAGE_PROGRAM << 24) | (Address >> 8);
	ptr = (UINT_T *)temp;
	for( i = 1; i < ((Size >> 2) + 2); i++ )
	{
		DMA_PP[i] = Endian_Convert( ptr[i-1] );
	}
	
	//configDescriptor(&RX_Desc[0], &RX_Desc[1],(UINT_T)SSP_DR,		(UINT_T)&rx1_fromcommand, &RX_Cmd, Size + 4, 0);
	//configDescriptor(&TX_Desc[0], &TX_Desc[1],(UINT_T)DMA_PP,	(UINT_T)SSP_DR, 	  &TX_Cmd, Size + 4, 0);
	configDescriptor(&RX_Desc[0], NULL,	(UINT_T)SSP_DR,	(UINT_T)&rx1_fromcommand, &RX_Cmd, Size + 8, 1);
	configDescriptor(&TX_Desc[0], NULL,	(UINT_T)DMA_PP,	(UINT_T)SSP_DR, 			&TX_Cmd, Size + 8, 1); 

	
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

UINT_T MX_SPINOR_Write(UINT_T Address, UINT_T Buffer, UINT_T Size)
{
	UINT_T Retval, i, total_size, write_size, status;
	UINT_T *buff = (UINT_T *) Buffer;

	if(Size & 0x3)    //make size 4bytes-align
    	Size = (Size+4)&(~3);

	total_size = Size >> 2;

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
			Retval = MX_Write(Address, Buffer, WRITE_SIZE);
			
			//update counters
			Address+=WRITE_SIZE;
			Buffer+=WRITE_SIZE;
			Size-=WRITE_SIZE;
		}
		else
		{
			Retval = MX_Write(Address, Buffer, WRITE_SIZE);
			Size=0;
		}

		SPINOR_ReadStatus(TRUE);
		Retval = MX_SPINOR_CheckStatus();

	} while( (Size > 0) && (Retval == NoError) );

	return Retval;
}

UINT_T MX_EraseSector(UINT_T secAddr)
{
	DMA_CMDx_T		RX_Cmd, TX_Cmd;
	UINT_T start_time, Retval = NoError;

	unsigned char temp[5] = {0}, rx_temp[5];
	
	SPINOR_ReadStatus(TRUE);
	SPINOR_WriteEnable();
	SPINOR_ReadStatus(TRUE);

	TX_Cmd.value = 0;
	TX_Cmd.bits.IncSrcAddr = 1;
	TX_Cmd.bits.IncTrgAddr = 0;
	TX_Cmd.bits.FlowSrc = 0;
	TX_Cmd.bits.FlowTrg = 1;
	TX_Cmd.bits.Width = 1;
	TX_Cmd.bits.MaxBurstSize = 1;
	//TX_Cmd.bits.Length = 5;

	RX_Cmd.value = 0;
	RX_Cmd.bits.IncSrcAddr = 0;
	RX_Cmd.bits.IncTrgAddr = 0;
	RX_Cmd.bits.FlowSrc = 0;
	RX_Cmd.bits.FlowTrg = 1;
	RX_Cmd.bits.Width = 1;
	RX_Cmd.bits.MaxBurstSize = 1;

	TX_Cmd.value = 0x90014000;
	RX_Cmd.value = 0x10014000;

	temp[0] = MX_SPI_CMD_SECTOR_ERASE;
	temp[1] = (secAddr >> 24) & 0xFF;
	temp[2] = (secAddr >> 16) & 0xFF;
	temp[3] = (secAddr >> 8) & 0xFF;
	temp[4] = (secAddr >> 0) & 0xFF;

	//setup DMA
	//Map Device to Channel
	XllpDmacMapDeviceToChannel(SSP_TX_DMA_DEVICE, SSP_TX_CHANNEL);
	XllpDmacMapDeviceToChannel(SSP_RX_DMA_DEVICE, SSP_RX_CHANNEL);

	reg_write(SSP_CR0, SSP_CR0_INITIAL);
	reg_write(SSP_CR1, SSP_CR1_INITIAL | 0x700083);

	reg_write(SSP_CR0, SSP_CR0_DSS_8);
	reg_bit_set(SSP_CR0, SSP_CR0_SSE);

	configDescriptor(&RX_Desc[0], NULL,(UINT_T)SSP_DR,(UINT_T)rx_temp, &RX_Cmd, 5, 1);
	configDescriptor(&TX_Desc[0], NULL,(UINT_T)temp,	(UINT_T)SSP_DR,  &TX_Cmd, 5, 1);
		
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

	return Retval;
}

UINT_T MX_SPINOR_Erase(UINT_T Address, UINT_T Size)
{
	UINT_T numSectors, i, sector_size, Retval = NoError;
	
	P_FlashProperties_T pFlashP = GetFlashProperties(BOOT_FLASH);

	sector_size = pFlashP->BlockSize;

	if (Size % pFlashP->BlockSize == 0)
		numSectors = Size / pFlashP->BlockSize;
	else
		numSectors = Size / pFlashP->BlockSize + 1;

	for (i = 0; i < numSectors; i++)
	{
		//erase this sector
		Retval = MX_EraseSector(Address);
		Retval |= MX_SPINOR_CheckStatus();

		Address += sector_size;

		if (Retval != NoError)
			break;
	}

	return Retval;
}
