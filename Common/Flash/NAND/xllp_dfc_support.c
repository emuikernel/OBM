/******************************************************************************
 *
 *  (C)Copyright 2005 - 2008 Marvell. All Rights Reserved.
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
 *
 *  FILENAME: xllp_dfc_support.c
 *
 *  PURPOSE:  XLLP DFC support routines.
 *
 ******************************************************************************/

#include "xllp_dfc.h"
#include "xllp_dfc_defs.h"
#include "PlatformConfig.h"
#include "misc.h"
#include "nand.h"
#include "Errors.h"
#include "Typedef.h"
#include "dma.h"
#include "xllp_dmac.h"


//++
//************************************************************
// Unit Test Support
#ifdef _DFC_EM
#include "em_devlib.h"
#endif
//************************************************************
//--

// allocated aligned space for the READ commands
static __attribute__ ((aligned(16))) NAND_CMD_T		dispatch;
static __attribute__ ((aligned(16))) NAND_CMD_T		nand_read;	//used for either PAGE READ or NAKED READ

// Allocate 4 data descriptors: 1 pre bytes, 1 data, 1 post bytes, 1 spare area
static __attribute__ ((aligned(16))) XLLP_DMAC_DESCRIPTOR_T		nfu_data[4];
// Allocate 2 cmd descriptors: 1 dispatch, 1 naked read
static __attribute__ ((aligned(16))) XLLP_DMAC_DESCRIPTOR_T		nfu_cmd[2];

// aligned bit bucket
static __attribute__ ((aligned(16))) unsigned long	bit_bucket[16];

UINT_T temp = 0;

//#ifdef _DFC_EM
#define MIN(x, y)  (x < y) ? x : y
/*#else
#define MIN(x, y)           \
   ({ typeof (x) x_ = (x);  \
      typeof (y) y_ = (y);  \
      (x_ < y_) ? x_ : y_;  \
    })
#endif*/

extern UINT_T All_FF_flag;
extern UINT_T upload_nand_spare;

//Prototypes
UINT_T xdfc_getInt(UINT8_T * BaseAddress, unsigned int ByteOffset, unsigned int NumBytes);

////////
/////////
//////       S U P P O R T   R O U T I N E S
////////
///////


// xdfc_stop: this routine clears the run bit in the dfc
//            this is usually only called in error handling,
//            specifically, when the WRCMDRUN bit does not assert.
//            the assertion failure is usually due to the dfc
//            already being in a run state when the NDRUN bit
//            is written.

// this routine does not handle draining the fifo, which may
// be necessary if an error occurred in the middle of an operation.
//
// it may be better to reset the controller than to try to
// drain the fifo since there are no underflow indicators.
unsigned long xdfc_stop()
{
    NDCR_REG        control;
    unsigned long   temp;

    DFC_REG_READ(DFC_CONTROL, control.value);

    if( control.value & DFC_CTRL_ND_RUN )   // only do this if the dfc is running.
    {
        control.value &= ~DFC_CTRL_ND_RUN;          // force the run bit off.
        DFC_REG_WRITE(DFC_CONTROL, control.value);

        DFC_REG_READ(DFC_CONTROL, control.value);   // read back to ensure write completion
        temp = control.value + 1;                   // second part of ensuring write completion: use the read result in any way.
    }

    return control.value;
}




// xdfc_write_LP: xdfc_write for large pages (page size > 2k)
//
// MLC devices typically have have large pages.
//
// This routine will use a multi-stage write sequence to write large pages to flash.
// For each page, the multi-stage program sequence is:
//  1. Issue program command, with extended type set to command_dispatch, to start the write process
//  2. Then issue a program command again, with extended type set to "naked_write", each 2k chunk of data to be written.
//  use the NC bit to chain those commands together until the whole page has been programmed.
//
// notes:
//  1. All flash offsets must be page aligned. that will eliminate the need to do any "pre-byte" handling.
//  2. If the last chunk does not fill a page completely, the remainder of the page will be padded. that
//     efectively removes "post-byte" support, also.
//  The reason for removing pre-byte and post-byte support is so that the stack does not have to keep
//  buffers for scratch pages on the stack.
//
//  If pre-byte and post-byte do need to be supported, then only a single scratch page area of memory
//  is required (not two separate ones for pre-byte and post-byte). Use a single buffer. Only read into it
//  when necessary: load it at the beginning for pre-byte handling. Load it during the last page write for
//  post byte handling. Also handle the case where the pre-byte, real data, and post-byte data are all in
//  the same page. (Would only need one scratch page area of memory in that case, too.)

// note:
//  since pages for MLC can be so large, its not possible to keep a pre-byte
//  and post-byte page on the stack. result:
//  - all programs must start at a page boundary. eliminates "pre-byte" support.
//  - all pages will be filled to end with the "erase pattern" (0xff). eliminates "post-byte" support.


#if NAND_LP
UINT_T xdfc_write_LP(P_DFC_BUFFER buffer, unsigned int address, unsigned int bytes,UINT_T bUseSA,P_NAND_Properties_T pNAND_Prop)
{
    // variables for programming the dfc controller
    CMD_BLOCK       *default_cmds;
    NDCR_REG        control;
    NDCB0_REG       command;
    NDCB1_REG       addr1234;
    NDCB2_REG       addr5;

    // variables for keeping track of buffers & lengths.
    unsigned int    dfc_internal_buffer_size;       // this value determines how much data to transfer during
                                                    // the data transfer phase of a naked command transaction.
    unsigned int    page, pages, page_size;
    unsigned int    stage, last_stage, stages_per_page;         // number of data transfer stages per page.
    unsigned int    bytes_to_write, words_to_write;
    unsigned int    status;

    unsigned int    buf[2];     // used for read status commands.
    UINT_T          Retval;
    unsigned int    i;

    // these masks will be initialized according to which chip select the target device is on.
    unsigned int    ready_mask;
    unsigned int    page_done_mask;
    unsigned int    command_done_mask;
    unsigned int    bad_block_detect_mask;
	unsigned int    page_SASize, check_length, all_FF;
	P_DFC_BUFFER	spare_addr;



    /*
    *   Preliminary stuff.
    */

    // stop any command in progress.
    // FIXME: this is temporary, until more robust and complete error handling is
    // implemented. each routine should ensure that the dfc is returned to a known
    // state before returning - even (or especially!) in the case of error.
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value &= ~DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);


    dfc_internal_buffer_size = 2048;        // this could be platform dependent. so far, all of our processors use 2k internal buffer.

    default_cmds = pNAND_Prop->device_cmds; // used to find the specific command code for the nand media being accessed.
    // bpc: fix: turn off auto_rs for now. FIXME: investigate why auto_rs doesn't work.
    default_cmds->pg_program.bits.AUTO_RS = 0;

    // each chip select has its own set of status bits.
    // determine which status bits to observer.
    if( default_cmds->pg_program.bits.CSEL == 0 )   // FIXME: look for a more flexible way to determine chip select.
    {
        ready_mask              = DFC_SR_RDY0;
        page_done_mask          = DFC_SR_CS0_PAGED;
        command_done_mask       = DFC_SR_CS0_CMDD;
        bad_block_detect_mask   = DFC_SR_CS0_BBD;
    }
    else
    {
        ready_mask              = DFC_SR_RDY1;
        page_done_mask          = DFC_SR_CS1_PAGED;
        command_done_mask       = DFC_SR_CS1_CMDD;
        bad_block_detect_mask   = DFC_SR_CS1_BBD;
    }

    page_size = pNAND_Prop->PageSize;               // eg, 2k for SLC NAND, 4k & > for MLC.
	page_SASize = bUseSA == 1 ? pNAND_Prop->SpareAreaSize : 0;
	if(bUseSA)
		pages = bytes/(page_size+page_SASize);
	else
    	pages = (bytes+(page_size-1)) / page_size;
    stages_per_page = page_size / dfc_internal_buffer_size;
    last_stage = stages_per_page-1;                 // used to determine when to clear the next command NC flag in the NDCB0 register.


    // start the programming process:
    for(page=0;page<pages;page++)
    {
		if (All_FF_flag)
		{
			check_length = (page_size+page_SASize)/4;
			//check if next page are all 0XFF, if yes, then skip the page
			do
			{ 
				for(i=0;i<check_length;i++)
				{
				  all_FF = buffer[i];
				  if(all_FF != 0xFFFFFFFF)
					  break;
				}
				if(all_FF == 0xFFFFFFFF)
				{
				  buffer += ((page_size + page_SASize)>>2);
				  bytes -= page_size + page_SASize;
				  address += page_size;
				  page ++;
				}
			}while((all_FF==0xFFFFFFFF)&&(page<pages));

			if(page==pages)
				return NoError;
		}


		spare_addr = buffer + page_size/4;
        // (start the multi-stage naked write sequence with a dispatch command)

        // Clear status bits and set DFC to expect a command.
        DFC_REG_READ(DFC_STATUS, status);
        DFC_REG_WRITE(DFC_STATUS, status);

        // set the run bit so the dfc enters the wait for command state.
        DFC_REG_READ(DFC_CONTROL, control.value);
        control.value |= DFC_CTRL_ND_RUN;
        DFC_REG_WRITE(DFC_CONTROL, control.value);

        // Wait till DFC is ready for next command to be issued.
        Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);     // wait for write command request bit to assert
        if (Retval != NoError)
        {
            xdfc_stop();
            return DFC_WRCMD_TO;
        }
        DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

        // dfc is ready for a command now...


        /*
         *  convert the flash address into values appropriate for the address fields of the dfc NDCBx registers.
         */
        getAddr(address, &addr1234, &addr5, pNAND_Prop);

        // build a multistage command: operation (program), extended_type = dispatch, and address
        command.value = default_cmds->pg_program.value;
        command.bits.CMD_XTYPE = DFC_CMDXT_COMMAND_DISPATCH;
        command.bits.NC = 1; // valid command following this one

        // first we send the dispatch command
        *DFC_COMMAND0 = command.value;    // send program command with command dispatch
        *DFC_COMMAND0 = addr1234.value;   // page address 1-4
        *DFC_COMMAND0 = addr5.value;      // page address 5
        Retval = _WaitForDFCOperationComplete(command_done_mask, 10);       // wait for write command request bit to assert
        if (Retval != NoError)
        {
            xdfc_stop();
            return DFCCS0CommandDoneError;  // FIXME: should select between cs0 & cs1 as appropriate.
        }
        DFC_REG_WRITE(DFC_STATUS, command_done_mask);


        // now a multi-stage write has been initiated.
        // send a xtype write command for each stage.

        for(stage=0;stage<stages_per_page;stage++)
        {
            // Wait till DFC is ready for next command to be issued.
            Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);     // wait for write command request bit to assert
            if (Retval != NoError)
            {
                xdfc_stop();
                return DFC_WRCMD_TO;
            }
            DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

            // set the dfc to begin filling the internal buffer from software.
            // now we send the naked write command
            command.value = default_cmds->pg_program.value;
            if( stage != last_stage )
            {
                command.bits.CMD_XTYPE = DFC_CMDXT_NAKED_WRITE; // Naked write, not last
                command.bits.NC = 1;                // another stage/command following this one
            }
            else
            {
                command.bits.CMD_XTYPE = DFC_CMDXT_NAKED_WRITE_WITH_FINAL_COMMAND;
                command.bits.NC = 0;                // another stage/command is not following this one
            }
            *DFC_COMMAND0 = command.value;          // send naked write command
            *DFC_COMMAND0 = addr1234.value;         // dummy write
            *DFC_COMMAND0 = addr5.value;            // dummy write

            // Wait for dfc to indicate it is ready
            Retval = _WaitForDFCOperationComplete(DFC_SR_WRDREQ, 10);   // wait for write data request
            if (Retval != NoError)
            {
                xdfc_stop();
                return DFC_WRREQ_TO;
            }
            DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRDREQ);


            // fill the dfc internal fifo

            // determine how many real bytes need to be sent.
            bytes_to_write = bytes;
            if(bytes_to_write>dfc_internal_buffer_size)
                bytes_to_write=dfc_internal_buffer_size;            // limit amount to write to dfc_internal_buffer_size.
            bytes -= bytes_to_write;                                // adjust remaining bytes to write counter.

            words_to_write=bytes_to_write/sizeof(unsigned int);     // convert byte length counter to word length counter.

            for(i=0; i<words_to_write;i++)                          // fill the dfc internal fifo
            {
                *DFC_DATA = *buffer++;
            }
            // end of requested page data write for this stage...



            // see if any padding (to end of internal buffer) needs to be done.
            while(bytes_to_write < dfc_internal_buffer_size)
            {
                *DFC_DATA = 0xffffffff; // fixme: commented out during debug. uncomment this in release code.
                bytes_to_write+=sizeof(unsigned int);
            }
            // end of all page data (requested and pad) for this stage.




            // write the spare areas, too.

            /*
             *  Write the spare area with the "erased pattern"
             */
            if(bUseSA)
            {
	            for (i = xdfc_getSpareArea_LP(pNAND_Prop); i > 0; i--)
	            {
	                *DFC_DATA = *spare_addr++; // fixme: commented out during debug. uncomment this in release code.
	            }            	
            }
			else
			{
	            for (i = xdfc_getSpareArea_LP(pNAND_Prop); i > 0; i--)
	            {
	                *DFC_DATA = 0xffffffff; // fixme: commented out during debug. uncomment this in release code.
	            }
			}



            // the fifo is full, and now the dfc is writing the contents to flash.
            // after all page and spare area written, pgdn & cmdd should assert
            Retval = _WaitForDFCOperationComplete(page_done_mask, 10);
            if (Retval != NoError)
            {
                xdfc_stop();
                return DFC_PGDN_TO;
            }
            DFC_REG_WRITE(DFC_STATUS, page_done_mask);


            /*
            *  Wait for write to be done.
            *  FIXME: which asserts first? cmdd or pgdn?
            */
            Retval = _WaitForDFCOperationComplete(command_done_mask, 10);
            if (Retval != NoError)
            {
                xdfc_stop();
                return DFCCS0CommandDoneError;
            }
            DFC_REG_WRITE(DFC_STATUS, command_done_mask);

            // Check for double bit error.
            if (*DFC_STATUS & DFC_SR_DBERR)
            {
                xdfc_stop();
                return DFCDoubleBitError;
            }

            // fixme: need to wait for RDY here? in the debugger, see if it is asserting.
        }

        // end of page: do a readstatus and check for successful programming.

        // need to wait for RDY before sending the readstatus command...
        // fixme: make this wait for rdy code check which cs should be used.
        Retval = _WaitForDFCOperationComplete(ready_mask, 10);
        if (Retval != NoError)
        {
            xdfc_stop();
            return DFC_RDY_TO;
        }
        DFC_REG_WRITE(DFC_STATUS, ready_mask);

        xdfc_readstatus( &buf[0], pNAND_Prop);
        // check for an error here...
    //   if( buf[0] != 0xe0 )
    if( buf[0]!= 0xc0 && buf[0]!= 0xe0 && buf[0]!= 0xe2)
        {
            xdfc_stop();
            return ProgramError;
        }

        // a little more checking... FIXME: should this be done before calling read status?
        if (*DFC_STATUS & bad_block_detect_mask)
        {
            xdfc_stop();
            return DFCCS0BadBlockDetected;
        }

        // the page was written successfully...go to the next one.
        address += page_size;
		if(bUseSA)
		{
			buffer += page_SASize/4;
			bytes -= page_SASize;
		}
    }


   return NoError;
}
#endif

/*UINT_T debug_info[40]; */

//uses flags to control spare area writes for SLC NAND and FBF downloading
int
xdfc_write(P_DFC_BUFFER buffer, UINT_T address, UINT_T bytes, UINT_T bUseSA, UINT_T bUseHwEcc, P_NAND_Properties_T pNAND_Prop)
{
#if COPYIMAGESTOFLASH
#if !NAND_LP

   CMD_BLOCK    *default_cmds;
   NDCR_REG   control;
   NDCB1_REG    addr1234;
   NDCB2_REG    addr5;
   unsigned int  i,  all_FF, check_length;
   unsigned int  pages, page_mask, page_first, page_last, page_size, page_extra;
   unsigned int  byte_count = 0, longwords;
   unsigned int  pre_bytes, post_bytes;
   unsigned int  status;
   unsigned int  page1[2048/4]={0xFFFFFFFF}, pageN[2048/4]={0xFFFFFFFF};
   unsigned int  buf[2];
   UINT_T Retval;
   unsigned int page_SASize;
   unsigned int spa_words, eccbytes;
   unsigned int lw;

   //make sure bytes is word aligned
   bytes += (bytes & 3) == 0 ? 0 : 4 - (bytes & 3);

   /*
   *   Preliminary stuff.
    */
   // stop any command in progress.
   // FIXME: this is temporary, until more robust and complete error handling is
   // implemented. each routine should ensure that the dfc is returned to a known
   // state before returning - even (or especially!) in the case of error.
   DFC_REG_READ(DFC_CONTROL, control.value);
   control.value &= ~DFC_CTRL_ND_RUN;
   DFC_REG_WRITE(DFC_CONTROL, control.value);


   page_size  = pNAND_Prop->PageSize;
   page_SASize = bUseSA == 1 ? pNAND_Prop->SpareAreaSize : 0;

   spa_words = xdfc_getStatusPadding(pNAND_Prop);
   eccbytes = page_SASize - spa_words*4;

   page_mask  = ~(page_size - 1);
   page_first =  address              & page_mask;
   page_last  = (address + bytes - 1) & page_mask;
   page_extra = ((address + bytes) > page_last) ? 1 : 0;

   pages = (bytes / page_size) + page_extra;
   getAddr(address, &addr1234, &addr5, pNAND_Prop);


   //////////////////////////////////////////////////////////////////////////////////
   ///////////                                                           ////////////
   ///////////            P R E S E R V A T I O N   S T E P S            ////////////
   ///////////                                                           ////////////
   //////////////////////////////////////////////////////////////////////////////////
   /*
   *  Calculate the pre and post padding.
    */
   pre_bytes  = address - page_first;
   post_bytes = (page_last + page_size) - (address + bytes);

   //if (pre_bytes)  xdfc_read(&page1[0], page_first, page_size, NULL, pNAND_Prop);
   //if (post_bytes) xdfc_read(&pageN[0], page_last,  page_size, NULL, pNAND_Prop);

   byte_count = 0;
   lw  = 0;
   longwords = 0;

   if ((All_FF_flag) && (bytes && pre_bytes==0))
   {
	   check_length = (page_size+page_SASize)/4;
	   //check if next page are all 0XFF, if yes, then skip the page
	   do
	   { 
		   for(i=0;i<check_length;i++)
		   {
			 all_FF = buffer[i+longwords];
			 if(all_FF != 0xFFFFFFFF)
				 break;
		   }
		   if(all_FF == 0xFFFFFFFF)
		   {
			 longwords += ((page_size + page_SASize)>>2);
			 bytes -= (page_size + page_SASize);
			 byte_count += page_size;
	   
			 page_first += page_size;
		   }
	   }while((all_FF==0xFFFFFFFF)&&(bytes>=page_size));

	   if(bytes==0)
		return NoError;

   }


   //////////////////////////////////////////////////////////////////////////////////
   ///////////                                                           ////////////
   ///////////               S T A R T   W R I T I N G                   ////////////
   ///////////                                                           ////////////
   //////////////////////////////////////////////////////////////////////////////////

   /*
   *  SET UP THE COMMAND
    */

    default_cmds = pNAND_Prop->device_cmds;
    getAddr(page_first, &addr1234, &addr5, pNAND_Prop);
#if 0
    // bc: seems like auto_rs is not working, so force it off for now.
    // If read status command is not 0x70, do not use auto read status method.
    if( default_cmds->read_status.bits.CMD1 == 0x70)
        default_cmds->pg_program.bits.AUTO_RS = 1;
#else
    // bc: seems like auto_rs is not working, so force it off for now.
    default_cmds->pg_program.bits.AUTO_RS = 0;
#endif

    // Clear write status bit and set DFC to expect a command.
    DFC_REG_READ(DFC_STATUS, status);
    DFC_REG_WRITE(DFC_STATUS, status);

    //*DFC_CONTROL |= DFC_CTRL_ND_RUN;
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value |= DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value)

    /*
    *  Wait for write command request bit.
    */
    Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFC_WRCMD_TO;
    }

    *DFC_COMMAND0 = default_cmds->pg_program.value;
    *DFC_COMMAND0 = addr1234.value;
    *DFC_COMMAND0 = addr5.value;

    /*
    *  Wait for write command request bit.
    */
    Retval = _WaitForDFCOperationComplete(DFC_SR_WRDREQ, 10);
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFC_WRREQ_TO;
    }

    /*
    *  Write the pre padding. (No need to check for a page boundary.)
    */
    while (pre_bytes)
    {
       *DFC_DATA = page1[lw++];
       byte_count += 4;
       pre_bytes  -= 4;
    }

    /*
    *  Write the buffer contents. (Check for page boundaries.)
    */
    while (bytes)
    {
       *DFC_DATA = buffer[longwords++];
       byte_count += 4;
       bytes      -= 4;

       /*
       *  Check for page boundary. Reissue the write command all over again.
       */
       if (bytes && ((byte_count & ~page_mask) == 0))
       {
          /*
          *  Write the status area padding.
          */
          if ( bUseSA )
          {
             // output the spare area
             for (i = spa_words; i > 0; i--)
             {
                // pull the spare area data from the buffer
                *DFC_DATA = buffer[longwords++];
                bytes -= 4;
             }

             // output the ECC bytes if HW ecc is off
             if ( !bUseHwEcc )
             {
                 for (i = eccbytes/4; i > 0; i--)
                 {
                    *DFC_DATA = 0xFFFFFFFF;
                 }
             }
			 else
			 {
				longwords += (eccbytes>>2);
				bytes -= eccbytes;
			 }
          }
          else
          {
              i = spa_words + ((bUseHwEcc == 1) ? 0 : eccbytes/4);
              for (; i > 0; i--)
                 *DFC_DATA = 0xFFFFFFFF;
          }

          /*
          *  Wait for write to be done.
          */
          if (default_cmds->pg_program.bits.CSEL == 0)
             Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);

          if (Retval != NoError)
          {
                xdfc_stop();
                return DFCCS0CommandDoneError;
          }

          if (default_cmds->pg_program.bits.CSEL == 1)
            Retval = _WaitForDFCOperationComplete(DFC_SR_CS1_CMDD, 10);

          if (Retval != NoError)
          {
                xdfc_stop();
                return DFCCS1CommandDoneError;
          }

          // Check for double bit error.
          if (*DFC_STATUS & DFC_SR_DBERR)
          {
             xdfc_stop();
             return DFCDoubleBitError;
          }

          /*
          *  Wait for rdy to assert.
          */
          if (default_cmds->pg_program.bits.CSEL == 0)
          {
             Retval = _WaitForDFCOperationComplete(DFC_SR_RDY0, 10);
             if (Retval != NoError)
             {
                xdfc_stop();
                return DFC_RDY_TO;
             }
          }

          if (default_cmds->pg_program.bits.CSEL == 1)
          {
             Retval = _WaitForDFCOperationComplete(DFC_SR_RDY1, 10);
             if (Retval != NoError)
             {
                xdfc_stop();
                return DFC_RDY_TO;
             }
          }

          // If using auto status read, read the status returned.
          if (default_cmds->pg_program.bits.AUTO_RS)
          {
              // NOTE: Use the data returned by the NAND in some way...
              buf[0] = *DFC_DATA;
              status = *DFC_DATA;
          }
          else
          {
            xdfc_readstatus( &buf[0], pNAND_Prop);
          }

          // note: in the next few checks, there is no need to stop the controller
          // it's already stopped because the program page command has finished.

          // check the status of the program operation
         /// if( buf[0] != 0xe0 )
	   if( buf[0]!= 0xc0 && buf[0]!= 0xe0 && buf[0]!= 0xe2)
          {
             return ProgramError;
          }

          if ((default_cmds->pg_program.bits.CSEL == 0) && (*DFC_STATUS & DFC_SR_CS0_BBD))
              return DFCCS0BadBlockDetected;
          if ((default_cmds->pg_program.bits.CSEL == 1) && (*DFC_STATUS & DFC_SR_CS1_BBD))
              return DFCCS1BadBlockDetected;


          // resuming state where controller needs to be stopped on any failure...

          // Increment page address.
          page_first += page_size;

		  //check if next page are all 0XFF, if yes, then skip the page
		 if((All_FF_flag)&&(bytes>=page_size))
		 {
			 check_length = (page_size+page_SASize)/4;
			  do
			  {	
				  for(i=0;i<check_length;i++)
				  {
				  	all_FF = buffer[i+longwords];
					if(all_FF != 0xFFFFFFFF)
						break;
				  }
				  if(all_FF == 0xFFFFFFFF)
				  {
				  	longwords += ((page_size + page_SASize)>>2);
					bytes -= (page_size + page_SASize);
					byte_count += page_size;

					page_first += page_size;
				  }
			  }while((all_FF==0xFFFFFFFF)&&(bytes>= page_size));

			if(bytes==0)
			  	return NoError;
		 }	
          getAddr(page_first, &addr1234, &addr5, pNAND_Prop);

          /*
          *  SET UP THE COMMAND
          */
          // Clear write status bit and set DFC to expect a command.
          *DFC_STATUS  = DFC_SR_WRCMDREQ;
          DFC_REG_READ(DFC_CONTROL, control.value);
          control.value |= DFC_CTRL_ND_RUN;
          DFC_REG_WRITE(DFC_CONTROL, control.value);

          //
          // Wait till DFC is ready for the next command to be issued.
          Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
          if (Retval != NoError)
          {
             xdfc_stop();
             return DFC_WRCMD_TO;
          }

          *DFC_COMMAND0 = default_cmds->pg_program.value;
          *DFC_COMMAND0 = addr1234.value;
          *DFC_COMMAND0 = addr5.value;

          //
          // Wait till the DFC is ready for us to write.
          Retval = _WaitForDFCOperationComplete(DFC_SR_WRDREQ, 10);
          if (Retval != NoError)
          {
             xdfc_stop();
             return DFC_WRREQ_TO;
          }
       }
    }

     if ( bUseSA)
        return Retval;        

    /*
    *  Write the post padding. (No need to check for a page boundary.)
    */
    // finish writing out the last page
    lw = (page_size - post_bytes) / 4;
    while (post_bytes)
    {
       *DFC_DATA = pageN[lw++];
       byte_count += 4;
       post_bytes -= 4;
    }

    /*
    *  Write the status area padding.
    */
/*    if ( bUseSA )
    {
         // output the spare area
         for (i = xdfc_getStatusPadding(pNAND_Prop); i > 0; i--)
         {
            // pull the spare area data from the buffer
            *DFC_DATA = buffer[longwords++];
         }

         // output the ECC bytes if HW ecc is off
         if ( !bUseHwEcc )
         {
             for (i = eccbytes/4; i > 0; i--)
                *DFC_DATA = 0xFFFFFFFF;
        }
    }
    else
    {
*/    
         i = spa_words + ((bUseHwEcc == 1) ? 0 : eccbytes/4);
         for (; i > 0; i--)
            *DFC_DATA = 0xFFFFFFFF;
/*    }
*/
    /*
    *  Wait for write to be done.
    */
    if (default_cmds->pg_program.bits.CSEL == 0)
    {
       Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
       if (Retval != NoError)
       {
          xdfc_stop();
          return DFCCS0CommandDoneError;
       }
    }

    if (default_cmds->pg_program.bits.CSEL == 1)
    {
       Retval = _WaitForDFCOperationComplete(DFC_SR_CS1_CMDD, 10);
       if (Retval != NoError)
       {
          xdfc_stop();
          return DFCCS1CommandDoneError;
       }
    }

    // wait for the controller to stop running.
    // you would think that the run bit would clear at the same time as CMDD asserts,
    // but that is not the case. if you don't wait for NDCR:RUN to deassert, then
    // the dfc will not be able to issue the readstatus commands that follow.
    //
    // examining a debug log shows that rdy does assert around the same time as run deasserts.
    // so put in a wait for rdy first, before trying to do a read status
    //
    // Question: if must wait for run bit to clear before sending the next command,
    // is overlapping commands support really useful? almost all flash devices allow a read
    // status command to be sent at any time. but it looks like our controller must wait for
    // the command to complete (eg. CMDD asserts, then RDY asserts [and NDRUN clear at the
    // same time] before it will accept the readstatus command. Is software supposed to force
    // the run bit clear if it wants to send overlapping commands like readstatus?

    /*
    *  Wait for rdy to assert.
    */
    if (default_cmds->pg_program.bits.CSEL == 0)
    {
        Retval = _WaitForDFCOperationComplete(DFC_SR_RDY0, 10);
        if (Retval != NoError)
        {
            xdfc_stop();
            return DFC_RDY_TO;
        }
    }

    if (default_cmds->pg_program.bits.CSEL == 1)
    {
        Retval = _WaitForDFCOperationComplete(DFC_SR_RDY1, 10);
        if (Retval != NoError)
        {
            xdfc_stop();
            return DFC_RDY_TO;
        }
    }


    /*
    *  If we used auto status read, the bad block status is in the status register.
    *  Use read status as auto status does not work as expected
    */
    if (default_cmds->pg_program.bits.AUTO_RS)
    {
       // NOTE: Use the data returned by the NAND in some way...
       buf[0] = *DFC_DATA;
       status = *DFC_DATA;
    }
    else
    {
       xdfc_readstatus( &buf[0], pNAND_Prop);
    }

    // check the status of the program operation
    //if( buf[0] != 0xe0 )
   if( buf[0]!= 0xc0 && buf[0]!= 0xe0 && buf[0]!= 0xe2)	
    {
        return ProgramError;
    }

    if ((default_cmds->pg_program.bits.CSEL == 0) && (*DFC_STATUS & DFC_SR_CS0_BBD))
         return DFCCS0BadBlockDetected;
    if ((default_cmds->pg_program.bits.CSEL == 1) && (*DFC_STATUS & DFC_SR_CS1_BBD))
         return DFCCS1BadBlockDetected;

#endif
#endif

    return NoError;

}

UINT_T xdfc_bch_pace( P_NAND_Properties_T pNAND_Prop )
{
   UINT_T Retval;

   //
   // Wait for page to be ready to read.
   Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
   if (Retval != NoError)
   {
       // catastropic error.
       // stop the dfc operation and clear the status bits before returning.
       // FIXME: do that after debug is complete. during debug it may be
       // interesting to examine these bits before they're cleared.
          xdfc_stop();
          return DFC_RDDREQ_TO;
   }
   DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDDREQ);
   return NoError;
}



// SpareArea for the last page is returned only. However it could be 1st page and or last page of an erased block for SLC.
// MLC - Bad Block table no longer maintained in spare area. Must be created on first scan of device and saved.
UINT_T xdfc_read_nonDMA(P_DFC_BUFFER buffer, unsigned int address, unsigned int bytes, unsigned int* SpareArea, P_NAND_Properties_T pNAND_Prop)
{
#if !NAND_LP

    CMD_BLOCK   *default_cmds;
    NDCR_REG    control;
    NDCB0_REG   command;
    NDCB1_REG   addr1234;
    NDCB2_REG   addr5;
    unsigned int  page_first, page_last, page_mask;
    unsigned int  byte_count = 0, longwords;
    unsigned int  page_size, pre_bytes, post_bytes;
    unsigned int  i, status;
    UINT_T Retval, ReadStatus = NoError;

    // mlc/bch support:
   unsigned int  bch_en;
   unsigned int  bch_boundary = 0x0000001f;         // at bch boundary when none of these bits are set in the address.
   unsigned int  bch_rc;
   NDECCCTRL_REG dfc_ecc_ctrl_reg;

   // get the bch_en setting...
   DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );
   bch_en = dfc_ecc_ctrl_reg.bits.BCH_EN;


    /*
    *  Some preliminary information.
    *    Determine if we need to use the default READ command or Naked READ command
    *    If page size is larger than 2KB (DFC FIFO size) then multiple 2KB reads must be issued
    *   using the naked read sematics.
    */
    default_cmds = pNAND_Prop->device_cmds;
    command.value = default_cmds->read.value;
    page_size  = pNAND_Prop->PageSize;
    page_mask  = ~(page_size - 1);
    page_first =  address              & page_mask;
    page_last  = (address + bytes - 1) & page_mask;
    pre_bytes  = address - page_first;
    post_bytes = (page_last + page_size) - (address + bytes);

    /*
    *  Set up the address for the command
    */
    getAddr(page_first, &addr1234, &addr5, pNAND_Prop);

   // stop any command in progress.
   // FIXME: this is temporary, until more robust and complete error handling is
   // implemented. each routine should ensure that the dfc is returned to a known
   // state before returning - even (or especially!) in the case of error.
   DFC_REG_READ(DFC_CONTROL, control.value);
   control.value &= ~DFC_CTRL_ND_RUN;
   if (upload_nand_spare == TRUE)
   	control.value &= ~DFC_CTRL_ECC_EN; // disable ecc
   DFC_REG_WRITE(DFC_CONTROL, control.value);


    // Clear read status bit and set DFC to expect a command.
	DFC_REG_READ(DFC_STATUS, status);
	DFC_REG_WRITE(DFC_STATUS, status);

    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value |= DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    //
    // Wait till DFC is ready for next command to be issued.
    Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFC_WRCMD_TO;
    }

    *DFC_COMMAND0 = command.value;
    *DFC_COMMAND0 = addr1234.value;
    *DFC_COMMAND0 = addr5.value;

    //
    // Wait for page to be ready to read.
    Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFC_RDDREQ_TO;
    }


    /*
    *  Read and discard the pre padding. (No need to check for a page boundary.)
    */
    byte_count = 0;
    while (pre_bytes)
    {
        i = *DFC_DATA;
        byte_count += 4;
        pre_bytes  -= 4;            // Possible infinite loop

        if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;
    }

    /*
    *  Read into the buffer. (Check for page boundaries.)
    */
    longwords = 0;
    while (bytes)
    {
        buffer[longwords++] = *DFC_DATA;
        byte_count += 4;
        bytes      -= 4;

        /*
        *  Check for page boundary. Reissue the read command all over again.
        */
        if (bytes && ((byte_count & ~page_mask) == 0))
        {
            // check for bch pacing (every 32 bytes, wait for rddreq)
            if( bch_en && ((byte_count & bch_boundary)==0) ) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;

            /*
            *  Read the status area padding. // TBD MLC uses BCH ECC so padding is different - MLC returns 8 (LB only)
            */
            //for (i = (unsigned int)xdfc_getStatusPadding(pNAND_Prop); i > 0; i--)
            //    status = *DFC_DATA;

			if ((SpareArea != NULL) && (upload_nand_spare == TRUE))
		    {
		        for (i = (unsigned int)xdfc_getStatusPadding(pNAND_Prop); i > 0; i--)
		        {
		            //SpareArea[i] = *DFC_DATA;
		            buffer[longwords] = *DFC_DATA;
					longwords++;
		        }
		    }
		    else
		    {
		        for (i = xdfc_getStatusPadding(pNAND_Prop); i > 0; i--)
		            status = *DFC_DATA;
		    }
#if 0
            // Check for uncorrectable bit error.
            if (*DFC_STATUS & DFC_SR_DBERR)
            {
                xdfc_stop();
                return DFCDoubleBitError;
            }

            // Check for ECC threshold (if we hit 14 or more bits)
            if ((*DFC_STATUS & DFC_SR_ECC_CNT) >= (14 << 16))
                ReadStatus = ReadDisturbError;
#endif
            // Increment page address.
            page_first += page_size;
            getAddr(page_first, &addr1234, &addr5, pNAND_Prop);

            /*
            *   SET UP THE COMMAND
            */
            // Clear read status bit and set DFC to expect a command.
           	DFC_REG_READ(DFC_STATUS, status);
			DFC_REG_WRITE(DFC_STATUS, status);
            //*DFC_CONTROL |= DFC_CTRL_ND_RUN;
            DFC_REG_READ(DFC_CONTROL, control.value);
            control.value |= DFC_CTRL_ND_RUN;
            DFC_REG_WRITE(DFC_CONTROL, control.value);

            //
            // Wait till DFC is ready for next command to be issued.
            Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
            if (Retval != NoError)
            {
                xdfc_stop();
                return DFC_WRCMD_TO;
            }

            *DFC_COMMAND0 = command.value;
            *DFC_COMMAND0 = addr1234.value;
            *DFC_COMMAND0 = addr5.value;

            //
            // Wait for page to be ready to read.
            Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
            if (Retval != NoError)
            {
                xdfc_stop();
                return DFC_RDDREQ_TO;
            }
        }
        else if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;


    }

    /*
    *  Read and discard the post padding.
    */
    while (post_bytes)
    {
        i = *DFC_DATA;
        post_bytes -= 4;
        byte_count += 4;
        if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;
    }


    // Read the spare area.
	if ((SpareArea != NULL) && (upload_nand_spare == TRUE))
    {
        for (i = (unsigned int)xdfc_getStatusPadding(pNAND_Prop); i > 0; i--)
        {
            //SpareArea[i] = *DFC_DATA;
            buffer[longwords] = *DFC_DATA;
			longwords++;
        }
    }
    else
    {
        for (i = xdfc_getStatusPadding(pNAND_Prop); i > 0; i--)
            status = *DFC_DATA;
    }

	if (upload_nand_spare == TRUE)
	{
		DFC_REG_READ(DFC_CONTROL, control.value);
    	control.value |= DFC_CTRL_ECC_EN; // enable ecc
    	DFC_REG_WRITE(DFC_CONTROL, control.value);
	}
#if 0
    // Check for ECC threshold (if we hit 14 or more bits)
    if ((*DFC_STATUS & DFC_SR_ECC_CNT) >= (14 << 16))
            ReadStatus = ReadDisturbError;

    // the uncorrectable error status is more important than the
    // readdisturberror, so the return value should be uncorrectable error
    // in the case that they're both set.
    if (*DFC_STATUS & DFC_SR_DBERR)
        ReadStatus = DFCDoubleBitError;
#endif

    return ReadStatus;
#else
	return NoError;
#endif
}

#if NAND_LP
UINT_T xdfc_read_LP(P_DFC_BUFFER buffer, unsigned int address, unsigned int bytes, unsigned int* SpareArea, P_NAND_Properties_T pNAND_Prop)
{

   CMD_BLOCK     *default_cmds;
   NDCR_REG      control;
   NDCB0_REG     command;
   NDCB1_REG     addr1234;
   NDCB2_REG     addr5;
   unsigned int  page_first, page_last, page_mask;
   unsigned int  byte_count = 0, longwords;
   unsigned int  page_size, pre_bytes, post_bytes;
   unsigned int  i, status;
   UINT_T        Retval, ReadStatus = NoError;
   unsigned int  bch_en;
   unsigned int  bch_boundary = 0x0000001f;         // at bch boundary when none of these bits are set in the address.
   unsigned int  bch_rc;
   NDECCCTRL_REG dfc_ecc_ctrl_reg;

   // get the bch_en setting...
   DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );
   bch_en = dfc_ecc_ctrl_reg.bits.BCH_EN;

   page_size = pNAND_Prop->PageSize;
   page_mask  = ~(page_size - 1);
   page_first =  address              & page_mask;
   page_last  = (address + bytes - 1) & page_mask;
   pre_bytes  = address - page_first;
   post_bytes = (page_last + page_size) - (address + bytes);

   /*
   *  Set up the address for the command
   */
   getAddr(page_first, &addr1234, &addr5, pNAND_Prop);

   // stop any command in progress.
   // FIXME: this is temporary, until more robust and complete error handling is
   // implemented. each routine should ensure that the dfc is returned to a known
   // state before returning - even (or especially!) in the case of error.
   DFC_REG_READ(DFC_CONTROL, control.value);
   control.value &= ~DFC_CTRL_ND_RUN;
   DFC_REG_WRITE(DFC_CONTROL, control.value);


   // Clear read status bit and set DFC to expect a command.
   DFC_REG_READ(DFC_STATUS, status);
   DFC_REG_WRITE(DFC_STATUS, status);
   DFC_REG_READ(DFC_CONTROL, control.value);
   control.value |= DFC_CTRL_ND_RUN;
   DFC_REG_WRITE(DFC_CONTROL, control.value);

   //
   // Wait till DFC is ready for next command to be issued.
   Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
   if (Retval != NoError)
   {
       xdfc_stop();
       return DFC_WRCMD_TO;
   }
   DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

   default_cmds = pNAND_Prop->device_cmds;
   command.value = default_cmds->read.value;
   command.bits.CMD_XTYPE = DFC_CMDXT_COMMAND_DISPATCH;
   command.bits.NC = 1; // valid command following this one

   // first we send the dispatch command
   *DFC_COMMAND0 = command.value;    // send read command with command dispatch
   *DFC_COMMAND0 = addr1234.value;   // page address 1-4
   *DFC_COMMAND0 = addr5.value;      // page address 5

   // Wait til the command is done.
   Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
   if (Retval != NoError)
   {
       xdfc_stop();
       return DFCCS0CommandDoneError;
   }
   DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);

   // Wait til the data is available.
    Retval = _WaitForDFCOperationComplete(DFC_SR_RDY0, 10); // FIXME: should check which CS this is.
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFCCS0CommandDoneError;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDY0);     // clear CS0RDY, also. FIXME: should check which CS this is.

   // Wait till DFC is ready for next command to be issued.
   Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
   if (Retval != NoError)
   {
       xdfc_stop();
       return DFC_WRCMD_TO;
   }
   DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

   // now we send the naked read command
   command.value = default_cmds->read.value;
   command.bits.CMD_XTYPE = DFC_CMDXT_NAKED_READ;
   command.bits.NC = 1; // valid command following this one
   command.bits.CMD1 = 0xff;
   command.bits.CMD2 = 0xff;
   *DFC_COMMAND0 = command.value;    // send naked read command
   *DFC_COMMAND0 = addr1234.value;          // dummy write
   *DFC_COMMAND0 = addr5.value;             // dummy write

   //
   // Wait for page to be ready to read.
   Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
   if (Retval != NoError)
   {
       xdfc_stop();
       return DFC_RDDREQ_TO;
   }
   DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDDREQ);

    // FIXME: this is the place to check for an error, not til after the data's been read in...

   /*
   *  Read and discard the pre padding. (No need to check for a page boundary.)
   */
   byte_count = 0;
   while (pre_bytes)
   {
          i = *DFC_DATA;
          byte_count += 4;
          pre_bytes  -= 4;

          if(byte_count > 0 && (byte_count & (TWO_KB_MASK)) == 0)
          {

                // check for bch pacing (every 32 bytes, wait for rddreq)
                if( bch_en && ((byte_count & bch_boundary)==0) ) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;

               //  Read the status area padding.
                for (i = (unsigned int)xdfc_getSpareArea_LP(pNAND_Prop); i>0; i--)
                    status = *DFC_DATA;


               // PGDN & CMDD should assert here. clear of those ndsr bits.
               // FIXME: make these aware of which chip select is being used.
               Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
               if (Retval != NoError)
               {
                   xdfc_stop();
                   return DFCCS0CommandDoneError;
               }
               DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);

               Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_PAGED, 10);
               if (Retval != NoError)
               {
                   xdfc_stop();
                   return DFCCS0CommandDoneError;
               }
               DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_PAGED);


                // Check for double bit error.
                if (*DFC_STATUS & DFC_SR_DBERR)
                {
                    xdfc_stop();
                    return DFCDoubleBitError;
                }

               // Check for ECC threshold (if we hit 14 or more bits)
               if ((*DFC_STATUS & DFC_SR_ECC_CNT) >= (14 << 16))
                    ReadStatus = ReadDisturbError;

               // Wait till DFC is ready for next command to be issued.
               Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
               if (Retval != NoError)
               {
                   xdfc_stop();
                    return DFC_WRCMD_TO;
               }
               DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

               // reissue the naked read command for the next 2KB
               command.value = default_cmds->read.value;
               command.bits.CMD_XTYPE = DFC_CMDXT_NAKED_READ;
               command.bits.NC = 1; // valid command following this one
               command.bits.CMD1 = 0xff;
               command.bits.CMD2 = 0xff;
               *DFC_COMMAND0 = command.value;    // send naked read command
               *DFC_COMMAND0 = addr1234.value;          // dummy
               *DFC_COMMAND0 = addr5.value;             // dummy

               // Wait till DFC is ready for next command to be issued.
               Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
               if (Retval != NoError)
               {
                   xdfc_stop();
                   return DFC_RDDREQ_TO;
               }
               DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDDREQ);

               // FIXME: this is the place to check for an error, not til after the data's been read in...

          }
          // check for bch pacing (every 32 bytes, wait for rddreq)
          else if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;


   }

   /*
   *  Read into the buffer. (Check for page boundaries.)
   */
   longwords = 0;
   while (bytes)
   {
          buffer[longwords++] = *DFC_DATA;
          byte_count += 4;
          bytes      -= 4;

          /*
          *  Check for page boundary. Reissue the read command all over again.
          */
          if (bytes && ((byte_count & ~page_mask) == 0))
          {

                // check for bch pacing (every 32 bytes, wait for rddreq)
                if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;

                //  Read the status area padding.
                //  return the spare area contents even if it is between 2k buffers...
                for (i = (unsigned int)xdfc_getSpareArea_LP(pNAND_Prop); i>0; i--)
                {
                  if (SpareArea != NULL)
                         *SpareArea++ = *DFC_DATA;
                  else
                         status = *DFC_DATA;
                }

                // PGDN & CMDD will be asserted here.
                // FIXME: make these aware of which chip select is being used.
                Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
                if (Retval != NoError)
                {
                   xdfc_stop();
                   return DFCCS0CommandDoneError;
                }
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);

                Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_PAGED, 10);
                if (Retval != NoError)
                {
                   xdfc_stop();
                   return DFCCS0CommandDoneError;
                }
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_PAGED);

                // Check for ECC threshold (if we hit 14 or more bits)
                if ((*DFC_STATUS & DFC_SR_ECC_CNT) >= (14 << 16))
                    ReadStatus = ReadDisturbError;

                // Check for double bit error.
                if (*DFC_STATUS & DFC_SR_DBERR)
                {
                    xdfc_stop();
                    return DFCDoubleBitError;
                }

                // Increment page address.
                page_first += page_size;
                getAddr(page_first, &addr1234, &addr5, pNAND_Prop);

                 //
                 // Wait till DFC is ready for next command to be issued.
                 Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
                 if (Retval != NoError)
                 {
                     xdfc_stop();
                     return DFC_WRCMD_TO;
                 }
                 DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

                 default_cmds = pNAND_Prop->device_cmds;
                 command.value = default_cmds->read.value;
                 command.bits.CMD_XTYPE = DFC_CMDXT_COMMAND_DISPATCH;
                 command.bits.NC = 1; // valid command following this one

                 // first we send the dispatch command
                 *DFC_COMMAND0 = command.value;    // send read command with command dispatch
                 *DFC_COMMAND0 = addr1234.value;   // page address 1-4
                 *DFC_COMMAND0 = addr5.value;      // page address 5


                 // Wait til the command is done.
                 Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
                 if (Retval != NoError)
                 {
                    xdfc_stop();
                    return DFCCS0CommandDoneError;
                 }
                 DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);

                 // Wait til the data is available.
                 Retval = _WaitForDFCOperationComplete(DFC_SR_RDY0, 10); // FIXME: should check which CS this is.
                 if (Retval != NoError)
                 {
                    xdfc_stop();
                    return DFCCS0CommandDoneError;
                 }
                 DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDY0);        // clear CS0RDY, also. FIXME: should check which CS this is.


                 // Wait till DFC is ready for next command to be issued.
                 Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
                 if (Retval != NoError)
                 {
                     xdfc_stop();
                     return DFC_WRCMD_TO;
                 }
                 DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

                 // now we send the naked read command
                 command.value = default_cmds->read.value;
                 command.bits.CMD_XTYPE = DFC_CMDXT_NAKED_READ;
                 command.bits.NC = 1; // valid command following this one
                 command.bits.CMD1 = 0xff;
                 command.bits.CMD2 = 0xff;
                 *DFC_COMMAND0 = command.value;    // send naked read command
                 *DFC_COMMAND0 = addr1234.value;          // dummy write
                 *DFC_COMMAND0 = addr5.value;             // dummy write


                 Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
                 if (Retval != NoError)
                 {
                     xdfc_stop();
                     return DFC_RDDREQ_TO;
                 }
                 DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDDREQ);

          }
          else if( (bytes||post_bytes) && ((byte_count & (TWO_KB_MASK)) == 0))  // need another naked_read to re-fill the internal fifo?
          {                                                                     // answer is 'yes' if there are more bytes (data or post) to be read
                // check for bch pacing (every 32 bytes, wait for rddreq)
                if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;

                //  Read the status area padding.
                //  return the spare area contents even if it is between 2k buffers...
                for (i = (unsigned int)xdfc_getSpareArea_LP(pNAND_Prop); i>0; i--)
                {
                  if (SpareArea != NULL)
                         *SpareArea++ = *DFC_DATA;
                  else
                         status = *DFC_DATA;
                }
                // pgdn & cmdd assert here.
                // FIXME: make these aware of which chip select is being used.
                Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
                if (Retval != NoError)
                {
                   xdfc_stop();
                   return DFCCS0CommandDoneError;
                }
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);

                Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_PAGED, 10);
                if (Retval != NoError)
                {
                   xdfc_stop();
                   return DFCCS0CommandDoneError;
                }
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_PAGED);


                // Check for double bit error.
                if (*DFC_STATUS & DFC_SR_DBERR)
                {
                    xdfc_stop();
                    return DFCDoubleBitError;
                }

                // Check for ECC threshold (if we hit 14 or more bits)
                if ((*DFC_STATUS & DFC_SR_ECC_CNT) >= (14 << 16))
                    ReadStatus = ReadDisturbError;

                 // Wait till DFC is ready for next command to be issued.
                 Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
                 if (Retval != NoError)
                 {
                     xdfc_stop();
                     return DFC_WRCMD_TO;
                 }
                 DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

                // reissue the naked read command for the next 2KB
                // now we send the naked read command
                command.value = default_cmds->read.value;
                command.bits.CMD_XTYPE = DFC_CMDXT_NAKED_READ;
                command.bits.NC = (page_first >= page_last) ? 0 : 1; // valid command following this one
                command.bits.CMD1 = 0xff;
                command.bits.CMD2 = 0xff;
                *DFC_COMMAND0 = command.value;    // send naked read command
                *DFC_COMMAND0 = addr1234.value;   // dummy
                *DFC_COMMAND0 = addr5.value;      // dummy

                //
                // Wait till DFC is ready for next command to be issued.
                Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
                if (Retval != NoError)
                {
                    xdfc_stop();
                    return DFC_RDDREQ_TO;
                }
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDDREQ);
          }
          else  if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;


   }

   /*
   *  Read and discard the post padding.
   */
   while (post_bytes)
   {
          i = *DFC_DATA;
          post_bytes -= 4;
          byte_count += 4;

          if(((byte_count & (TWO_KB_MASK)) == 0) && (byte_count & (~page_mask)) != 0)
          {
                // check for bch pacing (every 32 bytes, wait for rddreq)
                if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;

                //  Read the status area padding.
                for (i = (unsigned int)xdfc_getSpareArea_LP(pNAND_Prop); i>0; i--)
                              status = *DFC_DATA;
                // pgdn & cmdd assert here.
                // FIXME: make these aware of which chip select is being used.
                Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
                if (Retval != NoError)
                {
                   xdfc_stop();
                   return DFCCS0CommandDoneError;
                }
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);

                Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_PAGED, 10);
                if (Retval != NoError)
                {
                   xdfc_stop();
                   return DFCCS0CommandDoneError;
                }
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_PAGED);


                // Check for ECC threshold (if we hit 14 or more bits)
                if ((*DFC_STATUS & DFC_SR_ECC_CNT) >= (14 << 16))
                ReadStatus = ReadDisturbError;

                // Wait till DFC is ready for next command to be issued.
                Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
                if (Retval != NoError)
                {
                    xdfc_stop();
                    return DFC_WRCMD_TO;
                }
                // pgdn & cmdd assert here.
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ); // PGDN. FIXME: check which CS. Also, use the constant instead of 400

                // reissue the naked read command for the next 2KB
                // now we send the naked read command
                command.value = default_cmds->read.value;
                command.bits.CMD_XTYPE = DFC_CMDXT_NAKED_READ;
                command.bits.NC = 0; // no valid command following this one
                command.bits.CMD1 = 0xff;
                command.bits.CMD2 = 0xff;
                *DFC_COMMAND0 = command.value;    // send naked read command
                *DFC_COMMAND0 = addr1234.value;          // dummy
                *DFC_COMMAND0 = addr5.value;             // dummy

                //
                // Wait till DFC is ready for next command to be issued.
                Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
                if (Retval != NoError)
                {
                    xdfc_stop();
                    return DFC_RDDREQ_TO;
                }
                DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDDREQ);
          }
          // check for bch pacing (every 32 bytes, wait for rddreq)
          else if( bch_en && ((byte_count & bch_boundary)==0)) if( (bch_rc=xdfc_bch_pace(pNAND_Prop)) ) return bch_rc;
    }


    //  Read the status area padding.
    for (i = (unsigned int)xdfc_getSpareArea_LP(pNAND_Prop); i>0; i--)
    {
          if (SpareArea != NULL)
                 *SpareArea++ = *DFC_DATA;
          else
                 status = *DFC_DATA;
    }

    // pgdn & cmdd assert here.
    // FIXME: make these aware of which chip select is being used.
    Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
    if (Retval != NoError)
    {
       xdfc_stop();
       return DFCCS0CommandDoneError;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);

    Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_PAGED, 10);
    if (Retval != NoError)
    {
       xdfc_stop();
       return DFCCS0CommandDoneError;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_PAGED);



    // Check for ECC threshold (if we hit 14 or more bits)
    if ((*DFC_STATUS & DFC_SR_ECC_CNT) >= (14 << 16))
        ReadStatus = ReadDisturbError;

    // the uncorrectable error status is more important than the
    // readdisturberror, so the return value should be uncorrectable error
    // in the case that they're both set.
    if (*DFC_STATUS & DFC_SR_DBERR)
        ReadStatus = DFCDoubleBitError;

 return ReadStatus;
}
#endif
//int xdfc_read(P_DFC_BUFFER buffer, unsigned int address, unsigned int bytes, unsigned int* SpareArea, P_NAND_Properties_T pNAND_Prop)

UINT_T xdfc_stripPad(unsigned int upper, unsigned int lower)
{   //example:
    //upper = 0x00120034    lower = 0x00560078
    //value = 0x12345678

    unsigned int value = 0;
    value = (upper << 8) & 0xFF000000;
    value |= (upper << 16) & 0x00FF0000;
    value |= (lower >> 8) & 0x0000FF00;
    value |= lower & 0x000000FF;

    return value;
}

UINT_T xdfc_readID(P_DFC_BUFFER buffer, DEVICE_TYPE dev_type)
{
    NDCB0_REG       cmd0;
    NDCB1_REG       cmd1;
    NDCB2_REG       cmd2;
    NDCR_REG        control;
    NDSR_REG        status;

    //unsigned int    longwords = 0;
    UINT_T          Retval;

    NDECCCTRL_REG   dfc_ecc_ctrl_reg;
    NDECCCTRL_REG   dfc_ecc_ctrl_reg_save;

    // stop any command in progress.
    // FIXME: this is temporary, until more robust and complete error handling is
    // implemented. each routine should ensure that the dfc is returned to a known
    // state before returning - even (or especially!) in the case of error.
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value &= ~DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    // make sure bch is off during readid
    DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );
    dfc_ecc_ctrl_reg_save = dfc_ecc_ctrl_reg;
    dfc_ecc_ctrl_reg.bits.BCH_EN = 0;
    DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );


    /*
    *  Clear any previous Read Status
    *
    */
    //*DFC_STATUS = *DFC_STATUS;  // clear only *non* reserved bits
    DFC_REG_READ(DFC_STATUS, status.value);
    DFC_REG_WRITE(DFC_STATUS, status.value);


    /*
     *  Kick off the command
     *      Note that you should be able to fill the command buffer
     *           and then start the command but this does not seem
     *           to work with the DFC controller as documented in
     *           the IAS.  So we kick off the command and wait for
     *           the DFC to request the next command like in DMA mode.
     */

    //control.value = *DFC_CONTROL;
    DFC_REG_READ(DFC_CONTROL, control.value);

    if(dev_type == LARGE)
        control.bits.RD_ID_CNT = 4;
    else
        control.bits.RD_ID_CNT = 2;

    control.bits.ND_RUN = 1;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    cmd0.value = 0; //take care of reserved bits, etc.
    cmd0.bits.CMD1 = 0x90;
    cmd0.bits.ADDR_CYC = 1;
    cmd0.bits.CMD_TYPE = DFC_CMDTYPE_READID;

    cmd1.value = 0x0;
    cmd2.value = 0x0;


   /*
    *  Wait for write command request bit.
    */
    Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
    if (Retval != NoError)
    {
        DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg_save.value );
        xdfc_stop();
        return DFC_WRCMD_TO;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);


   /*
    *  Write commands to command buffer 0.
    *
    */
   DFC_REG_WRITE(DFC_COMMAND0, cmd0.value);
   DFC_REG_WRITE(DFC_COMMAND0, cmd1.value);
   DFC_REG_WRITE(DFC_COMMAND0, cmd2.value);

   /*
    *  Wait for data in the buffer and check command done in the control register.
    */
    Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
    if (Retval != NoError)
    {
        DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg_save.value );
        xdfc_stop();
        return DFC_RDDREQ_TO;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDDREQ);

   /*
    *  Read the read ID bytes. + read 4 bogus bytes
    */
    DFC_REG_READ(DFC_DATA, buffer[0]);      // this read returns the data of interest
    DFC_REG_READ(DFC_DATA, buffer[1]);      // this read required by spec (sections x.4.1.2, x.3.4.3, x.4.6)

   /*
    *  Wait for command to complete (it's probably already finished). FIXME: make this aware of which chip select is being used.
    */
    Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
    if (Retval != NoError)
    {
        DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg_save.value );
        xdfc_stop();
        return DFCCS0CommandDoneError;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);

    // check for a run bit stuck on. this is an error - but try to handle it gracefully.
    DFC_REG_READ(DFC_CONTROL, control.value);
    if (control.bits.ND_RUN == 1)
    {
        //DFC_REG_READ(DFC_CONTROL, control.value);
        //control.value &= ~DFC_CTRL_ND_RUN;
        //DFC_REG_WRITE(DFC_CONTROL, control.value);
        xdfc_stop();
    }

   DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg_save.value );
   return NoError;
}

/*
* Naked command is for Read Unique ID
*/
UINT_T xdfc_Naked_CMD(P_NAND_Properties_T pNAND_Prop, UINT8_T cmd)
{
    NDCR_REG control;
    //NDTR1CS0_REG  dfc_timing1_reg;
    NDCB0_REG cmd0;
    NDSR_REG status;
    UINT_T Retval;

    // stop any command in progress.
    // FIXME: this is temporary, until more robust and complete error handling is
    // implemented. each routine should ensure that the dfc is returned to a known
    // state before returning - even (or especially!) in the case of error.
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value &= ~DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    /*
    *  Clear any previous Read Status
    */
    //*DFC_STATUS = *DFC_STATUS;  // clear only *non* reserved bits/*
    DFC_REG_READ(DFC_STATUS, status.value);
    DFC_REG_WRITE(DFC_STATUS, status.value);

    /*
    *  Setting ND_RUN bit will get the transfer going.
    */
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value |= DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    // Wait for WRCMDREQ
    Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFC_WRREQ_TO;
    }

   /*
   *  Setup Commands.
    *
   */
    cmd0.value = 0; //take care of reserved bits, etc.
    cmd0.bits.CMD1 = cmd;
    cmd0.bits.CMD_TYPE = DFC_CMDTYPE_NAKED;

    /*
    *  Write commands to command buffer 0.
    *
    */
    /* Send Command */
    //*DFC_COMMAND0 = cmd0.value;
    DFC_REG_WRITE(DFC_COMMAND0, cmd0.value);
    //*DFC_COMMAND0 = 0;
    DFC_REG_WRITE(DFC_COMMAND0, 0);
    //*DFC_COMMAND0 = 0;
    DFC_REG_WRITE(DFC_COMMAND0, 0);

    // Wait for CMDD
    Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFCCS0CommandDoneError;
    }

    /*
    *  Make sure RDY is set at this point
    */
    Retval = _WaitForDFCOperationComplete(DFC_SR_RDY0, 10);
     /* Clear RDY */
    //*DFC_STATUS  = DFC_SR_RDY0 | DFC_SR_RDY1 | DFC_SR_CS0_CMDD;
    DFC_REG_WRITE(DFC_STATUS,DFC_SR_RDY0 | DFC_SR_RDY1 | DFC_SR_CS0_CMDD );
   if(Retval != NoError){
        //control.value = *DFC_CONTROL;
        DFC_REG_READ(DFC_CONTROL, control.value);

        // Clear ND_RUN bit
        if (control.bits.ND_RUN == 1)
        {
            ////*DFC_CONTROL &= ~DFC_CTRL_ND_RUN;
            //DFC_REG_READ(DFC_CONTROL, control.value);
            //control.value &= ~DFC_CTRL_ND_RUN;
            //DFC_REG_WRITE(DFC_CONTROL, control.value);
            xdfc_stop();
        }
        return DFC_RDY_TO;
    }
    return Retval;
}

UINT_T xdfc_Read_UID(UINT_T * buffer)
{
	UINT_T i;
	
	ResetNAND(BOOT_FLASH);
	
	xdfc_Naked_CMD(GetNANDProperties(), 0x5a);
	xdfc_Naked_CMD(GetNANDProperties(), 0xb5);
	
	xdfc_read_nonDMA(buffer, 0x0, 0x100, NULL, GetNANDProperties());
	
	ResetNAND(BOOT_FLASH);

	for (i = 0; i< 0x40;)
	{
		serial_outnum(buffer[i]);
		serial_outnum(buffer[i+1]);
		serial_outnum(buffer[i+2]);
		serial_outnum(buffer[i+3]);
		serial_outstr("\n");
		i+=4;
	}
}


UINT_T xdfc_reset(P_NAND_Properties_T pNAND_Prop)
{
    NDCR_REG control;
    //NDTR1CS0_REG  dfc_timing1_reg;
    NDCB0_REG cmd0;
    NDSR_REG status;
    UINT_T Retval;

    // stop any command in progress.
    // FIXME: this is temporary, until more robust and complete error handling is
    // implemented. each routine should ensure that the dfc is returned to a known
    // state before returning - even (or especially!) in the case of error.
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value &= ~DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    /*
    *  Clear any previous Read Status
    */
    //*DFC_STATUS = *DFC_STATUS;  // clear only *non* reserved bits/*
    DFC_REG_READ(DFC_STATUS, status.value);
    DFC_REG_WRITE(DFC_STATUS, status.value);

    /*
    *  Setting ND_RUN bit will get the transfer going.
    */
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value |= DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    // Wait for WRCMDREQ
    Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFC_WRREQ_TO;
    }

   /*
   *  Setup Commands.
    *
   */
    cmd0.value = 0; //take care of reserved bits, etc.
    cmd0.bits.CMD1 = 0xFF;
    cmd0.bits.CMD_TYPE = DFC_CMDTYPE_RESET;

    /*
    *  Write commands to command buffer 0.
    *
    */
    /* Send Command */
    //*DFC_COMMAND0 = cmd0.value;
    DFC_REG_WRITE(DFC_COMMAND0, cmd0.value);
    //*DFC_COMMAND0 = 0;
    DFC_REG_WRITE(DFC_COMMAND0, 0);
    //*DFC_COMMAND0 = 0;
    DFC_REG_WRITE(DFC_COMMAND0, 0);

    // Wait for CMDD
    Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
    if (Retval != NoError)
    {
        xdfc_stop();
        return DFCCS0CommandDoneError;
    }

    /*
    *  Make sure RDY is set at this point
    */
    Retval = _WaitForDFCOperationComplete(DFC_SR_RDY0, 10);
     /* Clear RDY */
    //*DFC_STATUS  = DFC_SR_RDY0 | DFC_SR_RDY1 | DFC_SR_CS0_CMDD;
    DFC_REG_WRITE(DFC_STATUS,DFC_SR_RDY0 | DFC_SR_RDY1 | DFC_SR_CS0_CMDD );
   if(Retval != NoError){
        //control.value = *DFC_CONTROL;
        DFC_REG_READ(DFC_CONTROL, control.value);

        // Clear ND_RUN bit
        if (control.bits.ND_RUN == 1)
        {
            ////*DFC_CONTROL &= ~DFC_CTRL_ND_RUN;
            //DFC_REG_READ(DFC_CONTROL, control.value);
            //control.value &= ~DFC_CTRL_ND_RUN;
            //DFC_REG_WRITE(DFC_CONTROL, control.value);
            xdfc_stop();
        }
        return DFC_RDY_TO;
    }
    return Retval;
}

UINT_T xdfc_readstatus(P_DFC_BUFFER buffer, P_NAND_Properties_T pNAND_Prop)
{
    CMD_BLOCK       *default_cmds;
    NDCB1_REG       cmd1;
    NDCB2_REG       cmd2;
    NDCR_REG        control;
    NDCR_REG        status;
    UINT_T          Retval;

    NDECCCTRL_REG   dfc_ecc_ctrl_reg;
    NDECCCTRL_REG   dfc_ecc_ctrl_reg_save;

    // stop any command in progress.
    // FIXME: this is temporary, until more robust and complete error handling is
    // implemented. each routine should ensure that the dfc is returned to a known
    // state before returning - even (or especially!) in the case of error.
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value &= ~DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    // make sure bch is off during readstatus
    //DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );
    //dfc_ecc_ctrl_reg_save = dfc_ecc_ctrl_reg;
    //dfc_ecc_ctrl_reg.bits.BCH_EN = 0;
    //DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );

	xdfc_enable_ecc(0);

    /*
    *  Clear any previous Read Status
    */
    DFC_REG_READ(DFC_STATUS, status.value);
    DFC_REG_WRITE(DFC_STATUS, status.value);

    /*
     *  Setting ND_RUN bit will get the transfer going.
    */
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value |= DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);


   /*
   *  Setup Commands.
    *
   */
    default_cmds = pNAND_Prop->device_cmds;
    cmd1.value = 0;
    cmd2.value = 0;
   /*
   *  Wait for write command request bit.
    */
    Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
    if (Retval != NoError)
    {
		xdfc_enable_ecc(1);
        //DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg_save.value );
        xdfc_stop();
        return DFC_WRCMD_TO;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRCMDREQ);

   /*
   *  Write commands to command buffer 0.
   */
   DFC_REG_WRITE(DFC_COMMAND0, default_cmds->read_status.value);
   DFC_REG_WRITE(DFC_COMMAND0, cmd1.value);
   DFC_REG_WRITE(DFC_COMMAND0, cmd2.value);

   /*
   *  Wait for read data to appear
   */
    Retval = _WaitForDFCOperationComplete(DFC_SR_RDDREQ, 10);
    if (Retval != NoError)
    {
		xdfc_enable_ecc(1);
        //DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg_save.value );
        xdfc_stop();
        return DFC_RDDREQ_TO;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_RDDREQ);


   DFC_REG_READ(DFC_DATA, buffer[0]);       // this read returns the status data
   DFC_REG_READ(DFC_DATA, buffer[1]);       // this read required by spec (sections x.4.1.2, x.3.4.3, x.4.6)

   /*
   *  Wait for CMDD. FIXME: make this aware of which chip select is really being used.
   */
    Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
    if (Retval != NoError)
    {
		xdfc_enable_ecc(1);
        //DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg_save.value );
        xdfc_stop();
        return DFCCS0CommandDoneError;
    }
    DFC_REG_WRITE(DFC_STATUS, DFC_SR_CS0_CMDD);


   // restore the bch_en state.
   xdfc_enable_ecc(1);
   //DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg_save.value );
   return NoError;
}

/*
*  NOTE: Introduce a generic erase. One that lets you erase any random slice of memory.
 *       Do this by reading from the segments of the block(s) that are to be preserved
*        before issuing the erase command. (Of couse, with about 32 pages per block,
 *       this would be an expensive operation...)
*/

//
// NOTE: To delete a page, is the address the start of the page, or can it be
//       any address within the page? [02aug2004agapito]
//

UINT_T xdfc_erase(unsigned int address, P_NAND_Properties_T pNAND_Prop)
{
#if COPYIMAGESTOFLASH
   CMD_BLOCK    *default_cmds;
   NDCR_REG     control;
   NDSR_REG     status;
   NDCB1_REG    addr1234;
   NDCB2_REG    addr5;
   unsigned int discarded_data;
   unsigned int buf[2];
   //unsigned int tmp;
   UINT_T Retval;

   // stop any command in progress.
   // FIXME: this is temporary, until more robust and complete error handling is
   // implemented. each routine should ensure that the dfc is returned to a known
   // state before returning - even (or especially!) in the case of error.
   DFC_REG_READ(DFC_CONTROL, control.value);
   control.value &= ~DFC_CTRL_ND_RUN;
   DFC_REG_WRITE(DFC_CONTROL, control.value);


   /*
    *  Setting ND_RUN bit will get the transfer going.
   */
    //
    //This is the original: *DFC_STATUS  |= 0xFFF;      //clear only *non* reserved bits
    //This is the usual:    *DFC_STATUS = *DFC_STATUS;  // clear only *non* reserved bits/*
    DFC_REG_READ(DFC_STATUS, status.value);
    DFC_REG_WRITE(DFC_STATUS, status.value);

    //*DFC_CONTROL |= DFC_CTRL_ND_RUN;
    DFC_REG_READ(DFC_CONTROL, control.value);
    control.value |= DFC_CTRL_ND_RUN;
    DFC_REG_WRITE(DFC_CONTROL, control.value);

    /*
     *  Generate command
    *
    */
    default_cmds = pNAND_Prop->device_cmds;
    getEraseAddr(address, &addr1234, &addr5, pNAND_Prop);

#if 0
    // bc: seems like auto_rs is not working, so force it off for now.
    // If read status command is not 0x70, do not use auto read status method.
    if( default_cmds->read_status.bits.CMD1 == 0x70)
        default_cmds->pg_program.bits.AUTO_RS = 1;
#else
    // bc: seems like auto_rs is not working, so force it off for now.
    default_cmds->pg_program.bits.AUTO_RS = 0;
#endif

   /*
    *  Wait for write command request bit.
   */

   Retval = _WaitForDFCOperationComplete(DFC_SR_WRCMDREQ, 10);
   if (Retval != NoError)
   {
       xdfc_stop();
       return DFC_WRCMD_TO;
   }

   /*
    *  Write commands to command buffer 0.
    *
   */
   DFC_REG_WRITE(DFC_COMMAND0, default_cmds->blk_erase.value);

   // Write out the address cycles
   DFC_REG_WRITE(DFC_COMMAND0, addr1234.value);
   DFC_REG_WRITE(DFC_COMMAND0, addr5.value);


   /*
   *  Erasing the block... Wait for ready to check status.
   *
   */
   // Check Device ready.
   Retval = _WaitForDFCOperationComplete(DFC_SR_RDY0, 10);
   if (Retval != NoError)
   {
        xdfc_stop();
        return DFC_RDY_TO;
   }
   if (default_cmds->pg_program.bits.CSEL == 0)
    Retval = _WaitForDFCOperationComplete(DFC_SR_CS0_CMDD, 10);
   if (Retval != NoError)
   {
       xdfc_stop();
       return DFCCS0CommandDoneError;
   }
   if (default_cmds->pg_program.bits.CSEL == 1)
    Retval = _WaitForDFCOperationComplete(DFC_SR_CS1_CMDD, 10);
   if (Retval != NoError)
   {
       xdfc_stop();
        return DFCCS1CommandDoneError;
   }

  /*
   *  If we used auto status read get the status else issue read status
   *    Then check the bad block status is in the status register.
    *
   */
    if (default_cmds->pg_program.bits.AUTO_RS)
    {
      DFC_REG_READ(DFC_DATA, buf[0]);
      DFC_REG_READ(DFC_DATA, discarded_data);
    }
    else
    {
        xdfc_readstatus( &buf[0], pNAND_Prop);
    }

    // examine the result of the erase operation
   /// if( buf[0] != 0xe0 )
   	if( buf[0]!= 0xc0 && buf[0]!= 0xe0 && buf[0]!= 0xe2)
    {
        // the flash device reported that the erase failed.
        return EraseError;
    }

    if ((default_cmds->pg_program.bits.CSEL == 0) && (*DFC_STATUS & DFC_SR_CS0_BBD))
         return DFCCS0BadBlockDetected;
    if ((default_cmds->pg_program.bits.CSEL == 1) && (*DFC_STATUS & DFC_SR_CS1_BBD))
         return DFCCS1BadBlockDetected;


   /*
   *  Check for double bit error.
    *
   */
   DFC_REG_READ(DFC_STATUS, status.value);
   if (status.value & DFC_SR_DBERR)
   {
      return DFCDoubleBitError;
   }
#endif

   return NoError;
}


void xdfc_setTiming(FLASH_TIMING *flash)
{
   NDTR0CS0_REG timing0={0};
   NDTR1CS0_REG timing1={0};

   //
   // NOTE: Confirm these calculations make sense! [25jun2004agapito]
   // METHOD:
   //    - integer division will truncate the result, so add a 1 in all cases
   //    - subtract the extra 1 cycle added to all register timing values
   //                                          1000 / NAND_CONTROLLER_CLOCK
   timing0.bits.tCH = MIN(((int) (flash->tCH * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tCH);
   timing0.bits.tCS = MIN(((int) (flash->tCS * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tCS);
   timing0.bits.tWH = MIN(((int) (flash->tWH * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tWH);
   timing0.bits.tWP = MIN(((int) (flash->tWP * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tWP);
   timing0.bits.tRH = MIN(((int) (flash->tRH * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tRH);
   timing0.bits.tRP = MIN(((int) (flash->tRP * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tRP);

   timing1.bits.tR   = MIN(((int) (flash->tR   * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tR);
   timing1.bits.tWHR = MIN(((int) (flash->tWHR * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tWHR);
   timing1.bits.tAR  = MIN(((int) (flash->tAR  * NAND_CONTROLLER_CLOCK / 1000) + 1), TIMING_MAX_tAR);

   //
   // Write timing registers to corresponding memory mapped registers.
   //
   //*DFC_TIMING_0 = timing0.value;
   DFC_REG_WRITE(DFC_TIMING_0, timing0.value);
   //*DFC_TIMING_1 = timing1.value;
   DFC_REG_WRITE(DFC_TIMING_1, timing1.value);

   return;
}

void getAddr(unsigned int address, NDCB1_REG * addr1234, NDCB2_REG * addr5, P_NAND_Properties_T pNAND_Prop)
{
   unsigned int column_address;
   unsigned int row_address;

   addr1234->value = 0;
   addr5->value    = 0;



   switch (pNAND_Prop->NAND_type)
   {
      case SMALL:
        addr1234->bits.ADDR1  = address & 0x00000FF;  // A0-A7
        addr1234->bits.ADDR2  = (address & 0x001FE00) >> 9; // A9-A16
        addr1234->bits.ADDR3  = (address & 0x1FE0000) >> 17; // A17-A24
        addr1234->bits.ADDR4  = (address & 0x6000000) >> 25; // A25-A26
        addr5->bits.ADDR5     = 0;
        break;

      case LARGE:
      default:
        // NOTE: The DFC should handle incrementing the column address
              //            we just need to worry about the row address to get
              //            to the correct page.  Row address starts in addr cycle 3.
        column_address = address & (pNAND_Prop->PageSize - 1);
        // if this is a x16 device, then convert the input "byte" address into
        // a "word" address appropriate for indexing a word-oriented device.
        if( pNAND_Prop->FlashBusWidth == FlashBusWidth16 ) column_address /= 2; // word to byte conversion

        row_address    = address / pNAND_Prop->PageSize;

        addr1234->bits.ADDR1  = (column_address & 0x000000FF);      // A0-A7   (column address)
        addr1234->bits.ADDR2  = (column_address & 0x0000FF00) >> 8; // A8-A12  (column address)
        addr1234->bits.ADDR3  = (row_address & 0x000000FF);         // A13-A20 (row address)
        addr1234->bits.ADDR4  = (row_address & 0x0000FF00) >> 8;    // A21-A28 (row address)
        addr5->bits.ADDR5     = (row_address & 0x00FF0000) >> 16;   // A29-A31 (row address)

        break;
      }
   return;
}

// getEraseAddress:
//   Convert a byte address into the format used during a Block Erase command.
//
//   The Block Erase command gets the block and page info in three address cycles.
//
//   This routine basically removes the column bits from the address and
//   puts the result into the three bytes used during the address cycle.

void getEraseAddr(unsigned int address, NDCB1_REG * addr1234, NDCB2_REG * addr5, P_NAND_Properties_T pNAND_Prop)
{
   unsigned int row_address;

   addr1234->value = 0;
   addr5->value    = 0;

   switch (pNAND_Prop->NAND_type)
   {
      case SMALL:
         addr1234->bits.ADDR1  = (address & 0x001FE00) >> 9;    // A9-A16
         addr1234->bits.ADDR2  = (address & 0x1FE0000) >> 17;   // A17-A24
         addr1234->bits.ADDR3  = (address & 0x2000000) >> 25;   // A25-A26
         addr1234->bits.ADDR4  = 0;
         addr5->bits.ADDR5     = 0;
         break;

      default:
        row_address = address / pNAND_Prop->PageSize;
        addr1234->bits.ADDR1  = (row_address & 0x000000FF);         // A13-A20 (row address)
        addr1234->bits.ADDR2  = (row_address & 0x0000FF00) >> 8;    // A21-A28 (row address)
        addr1234->bits.ADDR3  = (row_address & 0x00FF0000) >> 16;   // A29-A31 (row address)
        addr1234->bits.ADDR4  = 0;
        addr5->bits.ADDR5     = 0;
       break;
   }
   return;
}

#if 0
// Spare area, returns number of words to read
// bc: use getSpareArea because getSpareArea
//     knows about page size and it checks ecc & bch settings.
UINT_T xdfc_getStatusPadding(P_NAND_Properties_T pNAND_Prop)
{
   if (pNAND_Prop->PageSize > 512)
        return 10;
   else
        return  2;
}
#else
UINT_T xdfc_getStatusPadding(P_NAND_Properties_T pNAND_Prop)
{
    UINT_T  spareareasize;

    spareareasize=xdfc_getSpareArea_LP(pNAND_Prop);
    return spareareasize;
}

#endif


// Spare area, returns number of words to read
UINT_T xdfc_getSpareArea_LP(P_NAND_Properties_T pNAND_Prop)
{


    NDCR_REG dfc_control_reg;
    NDCB0_REG dfc_ndcb0_reg;
    NDECCCTRL_REG dfc_ecc_reg;
    UINT_T count = 0;

    DFC_REG_READ(DFC_CONTROL, dfc_control_reg.value);
    DFC_REG_READ(DFC_COMMAND0, dfc_ndcb0_reg.value);
    DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_reg.value);


    if ((dfc_control_reg.bits.SPARE_EN) && !(dfc_ndcb0_reg.bits.LEN_OVRD))
    {
        if ((dfc_control_reg.bits.PAGE_SZ == 0) && (!dfc_control_reg.bits.ECC_EN))
            count = 4;
        else if ((dfc_control_reg.bits.PAGE_SZ == 0) && (dfc_control_reg.bits.ECC_EN))
            count = 2;
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (!dfc_control_reg.bits.ECC_EN))
            count = 16;
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (dfc_control_reg.bits.ECC_EN) && (!dfc_ecc_reg.bits.BCH_EN))
            count = 10;
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (dfc_control_reg.bits.ECC_EN) && (dfc_ecc_reg.bits.BCH_EN))
            count = 8;
    }

    return count;

}


// Spare area, returns number of words to read, for normal reads only.
// this routine will ignore the setting of length override.
// it can be safely used if it is known that the following reads
// will be for normal page sizes, and not for something like parameter pages.
#if NAND_LP
UINT_T xdfc_getSpareArea_LPnormal(P_NAND_Properties_T pNAND_Prop)
{


    NDCR_REG dfc_control_reg;
    NDECCCTRL_REG dfc_ecc_reg;
    UINT_T count = 0;

    DFC_REG_READ(DFC_CONTROL, dfc_control_reg.value);
    DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_reg.value);


    if(dfc_control_reg.bits.SPARE_EN)
    {
        if ((dfc_control_reg.bits.PAGE_SZ == 0) && (!dfc_control_reg.bits.ECC_EN))
            count = 4;                                      // 512 byte page, no ecc. 16 byte sp
        else if ((dfc_control_reg.bits.PAGE_SZ == 0) && (dfc_control_reg.bits.ECC_EN))
            count = 2;                                      // 512 byte page, hamming ecc. 8 byte sp, 6 byte hamming, 2 byte unav.
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (!dfc_control_reg.bits.ECC_EN))
            count = 16;                                     // 2048 byte page, no ecc. 64 byte sp.
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (dfc_control_reg.bits.ECC_EN) && (!dfc_ecc_reg.bits.BCH_EN))
            count = 10;                                     // 2048 byte page, hamming ecc. 40 byte sp, 4*6 byte hamming
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (dfc_control_reg.bits.ECC_EN) && (dfc_ecc_reg.bits.BCH_EN))
            count = 8;                                      // 2048 byte page, bch ecc. 32 byte sp, 30 byte bch, 2 byte unav.
    }

    return count;

}
#endif


// ECC area, returns number of words to read
#if NAND_LP
UINT_T xdfc_getECCArea_LP(P_NAND_Properties_T pNAND_Prop)
{


    NDCR_REG dfc_control_reg;
    NDECCCTRL_REG dfc_ecc_reg;
    int count = 0;

    DFC_REG_READ(DFC_CONTROL, dfc_control_reg.value);
    DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_reg.value);


    if (dfc_control_reg.bits.ECC_EN)
    {
        if (dfc_control_reg.bits.PAGE_SZ == 0)
            count = 2;
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (!dfc_ecc_reg.bits.BCH_EN))
            count = 6;
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (dfc_ecc_reg.bits.BCH_EN))
            count = 8;
    }

    return count;

}
#endif


UINT_T xdfc_getInt(unsigned char* BaseAddress, unsigned int ByteOffset, unsigned int NumBytes)
{
    unsigned int temp = 0;
    unsigned int i;

    for (i=0; i<NumBytes; i++)
        {
        temp =  temp | (BaseAddress[ByteOffset+i] << (i*8));
        }
    return temp;
}

//----------------------------------------------------------------------------
// _WaitForDFCOperationPulldown
// Polls the status register to check if a particular status bit got cleared  
//
//
//----------------------------------------------------------------------------
UINT_T _WaitForDFCOperationPulldown( unsigned int statusMask, UINT32 TimeOutMicrosec )
{
  UINT32 startTime, endTime;
  NDSR_REG     status;

  DFC_REG_READ(DFC_STATUS, status.value);
  if( (statusMask & status.value) == 0 )
        return NoError;
        
  startTime = GetOSCR0(); //Dummy read to flush potentially bad data
  startTime = GetOSCR0();

  do
  {
    DFC_REG_READ(DFC_STATUS, status.value);
    if( (statusMask & status.value) == 0 )
        return NoError;
    endTime = GetOSCR0();
    if (endTime < startTime)
        endTime += (0x0 - startTime);

  }
  while( OSCR0IntervalInMicro(startTime, endTime) < TimeOutMicrosec );
  
  return TimeOutError;
}

//----------------------------------------------------------------------------
// WaitForDFCOperationComplete
// Polls the status register to check if a particular status bit got
// set after writing a command.
//
//----------------------------------------------------------------------------

UINT_T _WaitForDFCOperationComplete( unsigned int statusMask, UINT32 TimeOutMillisec )
{
  UINT32 startTime, endTime;
  NDSR_REG     status;
  
  DFC_REG_READ(DFC_STATUS, status.value);
  if(statusMask & status.value)
         return NoError;

  startTime = GetOSCR0(); //Dummy read to flush potentially bad data
  startTime = GetOSCR0();
  do
  {
    DFC_REG_READ(DFC_STATUS, status.value);
    if(statusMask & status.value)
         return NoError;
    endTime = GetOSCR0();
    if (endTime < startTime)
        endTime += (0x0 - startTime);
  }
  while( OSCR0IntervalInMilli(startTime, endTime) < TimeOutMillisec );

  return TimeOutError;
}


void xdfc_enable_ecc( unsigned int bEnable )
{
    NDCR_REG dfc_control_reg;
    NDECCCTRL_REG   dfc_ecc_ctrl_reg;
    P_NAND_Properties_T pNandP = GetNANDProperties();

    DFC_REG_READ(DFC_CONTROL, dfc_control_reg.value );
    DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );

    if ( pNandP->ECCMode == ECC_HAMMING ) // slc
    {
        dfc_control_reg.bits.ECC_EN = ((bEnable == 0) ? 0 : 1);
        DFC_REG_WRITE(DFC_CONTROL, dfc_control_reg.value );
        dfc_ecc_ctrl_reg.bits.BCH_EN = 0;
        DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );
    }
    if( pNandP->ECCMode == ECC_BCH ) // slc
    {
        dfc_control_reg.bits.ECC_EN = ((bEnable == 0) ? 0 : 1);
        DFC_REG_WRITE(DFC_CONTROL, dfc_control_reg.value );
        dfc_ecc_ctrl_reg.bits.BCH_EN = ((bEnable == 0) ? 0 : 1);
        DFC_REG_WRITE(DFC_ECCCTRL, dfc_ecc_ctrl_reg.value );
    }
}

// Spare area, returns number of words to read, for normal reads only.
// this routine will ignore the setting of length override.
// it can be safely used if it is known that the following reads
// will be for normal page sizes, and not for something like parameter pages.
UINT_T xdfc_getSpareAreaSize(P_NAND_Properties_T pNAND_Prop)
{
    NDCR_REG dfc_control_reg;
    NDCB0_REG dfc_ndcb0_reg;
    NDECCCTRL_REG dfc_ecc_reg;
    UINT_T count = 0;

    DFC_REG_READ(DFC_CONTROL, dfc_control_reg.value);
    DFC_REG_READ(DFC_COMMAND0, dfc_ndcb0_reg.value);
    DFC_REG_READ(DFC_ECCCTRL, dfc_ecc_reg.value);

	//All return values are in WORDS
    if (dfc_control_reg.bits.SPARE_EN)
    {
        if ((dfc_control_reg.bits.PAGE_SZ == 0) && (!dfc_control_reg.bits.ECC_EN))
            count = 4;                                      // 512 byte page, no ecc. 16 byte sp
        else if ((dfc_control_reg.bits.PAGE_SZ == 0) && (dfc_control_reg.bits.ECC_EN))
            count = 0;                                      // 512 byte page, hamming ecc. 8 byte sp, 6 byte hamming, 2 byte unav.
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (!dfc_control_reg.bits.ECC_EN))
            count = 16;                                     // 2048 byte page, no ecc. 64 byte sp.
        else if ((dfc_control_reg.bits.PAGE_SZ == 1) && (dfc_control_reg.bits.ECC_EN) && (dfc_ecc_reg.bits.BCH_EN))
            count = 8;                                      // 2048 byte page, bch ecc. 32 byte sp, 30 byte bch, 2 byte unav.
    }

    return count;
}

//NFU read chunk
// This routines reads the smaller of a Page or NFU fifo size
//	DMA is used, and NAKED Read operation will be used for devices with
//	a Page Size greater than the NFU fifo (2K). A MONOLITHIC read will
//	be used on all other devices
unsigned int xdfc_read_chunk(unsigned int buffer, 
							 unsigned int address, 
							 unsigned int size, 
							 unsigned int pre_bytes,
							 unsigned int SpareArea)
{
    unsigned int pSpareArea, Retval, value, dispatch_size, read_cmd_len;
    unsigned int post_bytes, spare_area_size, chunk_size, offset_mask;

	P_DMA_CMDx_T p_nfu_spare;
	DMA_CMDx_T	dma_data, dma_dummy;	//DMA cmds: data (data read into buffer and spare area)
										//			dummy (data dumped into dummy location from pre and post bytes)
	DMA_CMDx_T	dma_command;
    NDCB0_REG	command;
	P_NAND_Properties_T pNandP = GetNANDProperties();
    CMD_BLOCK	*dev_cmds = pNandP->device_cmds;

	//calculations
	offset_mask = pNandP->PageSize - 1;
	//how much are we reading for this call?
	if(pNandP->ECCMode == ECC_BCH_1K)
		chunk_size = ONE_KB;
	else
		chunk_size = NFU_FIFO_SIZE > pNandP->PageSize ? pNandP->PageSize : NFU_FIFO_SIZE;

	post_bytes = chunk_size - pre_bytes - size;
	spare_area_size = xdfc_getSpareAreaSize(pNandP) * 4;	

	//determine if caller wants the spare area stored, or just dump it
    if(SpareArea == NULL)
	{	//dump it
		pSpareArea = (unsigned int) &bit_bucket[0];
		p_nfu_spare = &dma_dummy;
	}
	else
	{	//return it
		pSpareArea = SpareArea;
		p_nfu_spare = &dma_data;
	}
	
	/* Setup the Read Commands */
	//READ command: either MONOLITHIC or NAKED
	read_cmd_len = 12;
	nand_read.cb0.value = dev_cmds->read.value;
	if(pNandP->PageSize > NFU_FIFO_SIZE)
	{	//Naked Read
		nand_read.cb0.bits.CMD_XTYPE = DFC_CMDXT_NAKED_READ;
		//Next Command: will be a 1 unless this is the last 'chunk' of the current page
		nand_read.cb0.bits.NC        = ((address + chunk_size) & offset_mask) ? 1 : 0;
		nand_read.cb1.value          = 0;
		nand_read.cb2.value          = 0;
		dispatch_size = 12;		//NAKED operation requires dispatch
	}
	else 
	{	//Standard Page Read
		//special case: for BCH_1K on a 2K Page device, there will be two page read commands
		//	issued per page of the device (1K each time).  For the second read, the flash 
		//	address must be incremented 30 bytes, as that is the space after the first 1K chunk
		//	where the ECC data is stored.
		if( (pNandP->ECCMode == ECC_BCH_1K) && (address & offset_mask) )
			address += 30;

		getAddr(address, &nand_read.cb1, &nand_read.cb2, pNandP);
		dispatch_size = 0;		//MONO operation means no dispatch
	}
	//update values for BCH_1K
	if(pNandP->ECCMode == ECC_BCH_1K)
	{
		nand_read.cb0.bits.LEN_OVRD = 1;
		nand_read.cb3.value = 0;
		nand_read.cb3.bits.NDLENCNT = ONE_KB;
		read_cmd_len = 16;
	}

	//DISPATCH command for NAKED Reads
	dispatch.cb0.value          = pNandP->device_cmds->read.value;
	dispatch.cb0.bits.CMD_XTYPE = DFC_CMDXT_COMMAND_DISPATCH;
	dispatch.cb0.bits.NC        = 1;
	getAddr(address, &dispatch.cb1, &dispatch.cb2, pNandP);
	dispatch_size = (address & offset_mask) ? 0 : dispatch_size;	//dispatch only at start of page

	//set the up the basic DMA commands
	dma_dummy.value = 0;
	dma_dummy.bits.FlowSrc = 1;
	dma_dummy.bits.Width = 3;
	dma_dummy.bits.MaxBurstSize = 1;
	
	dma_data.value = 0;
	dma_data.bits.IncTrgAddr = 1;
	dma_data.bits.FlowSrc = 1;
	dma_data.bits.Width = 3;
	dma_data.bits.MaxBurstSize = 1;

	dma_command.value = 0;
	dma_command.bits.IncSrcAddr = 1;
	dma_command.bits.FlowTrg = 1;
	dma_command.bits.Width = 3;
	dma_command.bits.MaxBurstSize = 1;

	//setup CMD descriptor
	//configure DISPATCH descriptor : NOTE - size will be 0 for MONO read or NAKED reads after the initial one
	configDescriptor(&nfu_cmd[0], &nfu_cmd[1], (unsigned int) &dispatch, (unsigned int) DFC_COMMAND0, &dma_command, dispatch_size, 0);
	//configure the READ command descriptor
	configDescriptor(&nfu_cmd[1], NULL, (unsigned int) &nand_read, (unsigned int) DFC_COMMAND0, &dma_command, read_cmd_len, 1);
	//setup DATA descriptors
	//configure the PREBYTE descriptor
	configDescriptor(&nfu_data[0], &nfu_data[1], (unsigned int) DFC_DATA, (unsigned int) &bit_bucket[0], &dma_dummy, pre_bytes, 0);
	//configure the Data descriptor
	configDescriptor(&nfu_data[1], &nfu_data[2], (unsigned int) DFC_DATA, buffer, &dma_data, size, 0);
	//configure the Post bytes descriptor
	configDescriptor(&nfu_data[2], &nfu_data[3], (unsigned int) DFC_DATA, (unsigned int) &bit_bucket[0], &dma_dummy, post_bytes, 0);
	//configure the Spare Area descriptor
	configDescriptor(&nfu_data[3], NULL, (unsigned int) DFC_DATA, pSpareArea, p_nfu_spare, spare_area_size, 1);
	/** data setup done **/

	/** configure NFU for read **/
	//clear DFC status
	DFC_REG_READ(DFC_STATUS, value);
	DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRTOCLR_MASK);

	// enable DFC DMA and asserts DMA request
	DFC_REG_READ(DFC_CONTROL, value);
	value |= DFC_CTRL_DMA_EN | DFC_CTRL_ND_RUN;
	DFC_REG_WRITE(DFC_CONTROL, value);

	// start DMA
	//Load descriptors
	loadDescriptor (&nfu_data[0], NFU_DATA_CHANNEL);
	loadDescriptor (&nfu_cmd[0], NFU_CMD_CHANNEL);

	//Kick off DMA's
	XllpDmacStartTransfer(NFU_CMD_CHANNEL);
	XllpDmacStartTransfer(NFU_DATA_CHANNEL);

	// wait for command DMA to complete
	Retval = WaitForOperationComplete(	DMA_OPERATION_TIMEOUT_MS * 10, 
										1, 
										(VUINT_T*)&XLLP_Return_PDmacHandle()->DCSR[NFU_CMD_CHANNEL], 
										XLLP_DMAC_DCSR_STOP_INTR, 
										XLLP_DMAC_DCSR_STOP_INTR);

	//wait for data DMA to complete
	Retval |= WaitForOperationComplete(	DMA_OPERATION_TIMEOUT_MS * 10, 
										1, 
										(VUINT_T*)&XLLP_Return_PDmacHandle()->DCSR[NFU_DATA_CHANNEL], 
										XLLP_DMAC_DCSR_STOP_INTR, 
										XLLP_DMAC_DCSR_STOP_INTR);

	//Grab DFC Status and clear the register
	DFC_REG_READ(DFC_STATUS, value);
	DFC_REG_WRITE(DFC_STATUS, DFC_SR_WRTOCLR_MASK);

	//read failure.  terminate DMA's and return failure
	if(Retval != NoError)
	{
		XllpDmacStopTransfer( NFU_CMD_CHANNEL );
		XllpDmacStopTransfer( NFU_DATA_CHANNEL );
		xdfc_stop();
		return Retval;
	}

	// check error conditions
    // Check for ECC threshold (if we hit 14 or more bits)
    if ((value & DFC_SR_ECC_CNT) >= (14 << 16))
		Retval = ReadDisturbError;

#if 0
    // the uncorrectable error status is more important than the
    // readdisturberror, so the return value should be uncorrectable error
    // in the case that they're both set.
    if (value & DFC_SR_DBERR)
		Retval = DFCDoubleBitError;
#endif
	xdfc_stop();
	//done.  return any errors
	return Retval;
}

//xdfc_read operation
//
// This routine will read data from the NFU to the user specified address
//	The data will be transfered in 'chunk's: 
//		Since the NFU FIFO is fixed at 2KB, that is the maximum amount of data 
//		that can be transfered in one command.  However, NAND pages can be anywhere
//		from 512B to 8KB (and still larger yet).  So this routine will define a 'chunk'
//		as the smaller of the device page size and the NFU fifo.
//	This routine will iterate over 'chunk's, and call the read chunk routine to handle
//		the actual data transfer
UINT_T xdfc_read(unsigned int  buffer, 
                 unsigned int  address, 
                 unsigned int  bytes, 
                 unsigned int* SpareArea, 
                 P_NAND_Properties_T pNAND_Prop)
{
	unsigned int chunk_size, offset_mask, Retval;
	unsigned int buff, spare, size, pre_size;
	unsigned int restore_dword = 0, *restore_addr = 0;

	offset_mask = (pNAND_Prop->PageSize - 1);
	pre_size = address & offset_mask;	//find pre-bytes on the first page
	address -= pre_size;				//page align the address
	
	if(pNAND_Prop->ECCMode == ECC_BCH_1K)
		chunk_size = ONE_KB;
	else
		chunk_size = NFU_FIFO_SIZE > pNAND_Prop->PageSize ? pNAND_Prop->PageSize : NFU_FIFO_SIZE;

	spare = (unsigned int) SpareArea;

	//pre-DMA setup:
	// modify the bytes value to be modulo 8 byte, as DMA cannot handle smaller burst sizes
	// this may cause an issue where 4 bytes after the image gets corrupted.  in this case
	// we must save and restore those 4 bytes
	if(bytes & 0x7) 	// check for modulo 8 bytes in length
	{					// - non 8 bytes.  adjustments required
		restore_addr = (unsigned int *)(buffer + bytes);//address *after* the loaded image
		restore_dword = *restore_addr;					//save the word after the loaded image
		bytes = (bytes & 0xFFFFFFF8) + 0x8;				//modulo 8 byte align the size
	}

	//setup DMA-NAND link
	//Map Device to Channels
	XllpDmacMapDeviceToChannel(DMAC_NAND_DATA, NFU_DATA_CHANNEL);
	XllpDmacMapDeviceToChannel(DMAC_NAND_CMD, NFU_CMD_CHANNEL);

	//turn ON user alignment - in case buffer address is 64bit aligned
	alignChannel(NFU_CMD_CHANNEL, 1);
	alignChannel(NFU_DATA_CHANNEL, 1);

	//need to loop around the chunk_size (smaller of page_size and NFU fifo size)
	do
	{
		//calculate read amount
		size = (pre_size > chunk_size) ? 0 : chunk_size - pre_size;
		size = size > bytes ? bytes : size;

		//call read chunk routine
		Retval = xdfc_read_chunk(buffer, address, size, pre_size, spare);

		//update counters
		pre_size = 0;			//after first read, no more pre bytes
		bytes 	-= size;		//decrement counter by data bytes read in
		buffer	+= size;		//bump buffer  by data bytes read in (ignoring pre bytes)
		address	+= chunk_size;	//bump address by entire chunk read
		if(spare != NULL)
			spare += size;

		//Success: when the page containing the end of the data has been fully read out
		if( (bytes == 0) && ((address & offset_mask) == 0) )
			break;

		//exit loop if there is an error
	} while( Retval == NoError );

	//disable DMA-NAND link
	XllpDmacUnMapDeviceToChannel(DMAC_NAND_CMD, NFU_CMD_CHANNEL);
	XllpDmacUnMapDeviceToChannel(DMAC_NAND_DATA, NFU_DATA_CHANNEL);

	//restore the dword if the image size was originally non-modulo 8 in length
	if( (Retval == NoError) && (restore_addr != NULL) )
		*restore_addr = restore_dword;

	return Retval;
}

