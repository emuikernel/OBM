/*-------------------------------------------------------------------------
 * Filename:      startup.lds
 *-----------------------------------------------------------------------*/
/*OUTPUT_FORMAT("elf32-arm", "elf32-arm", "elf32-arm")*/

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(__main)

/* when running from iSRAM, the following layout is used:  */
/* mem layout of sections and globalsdefined by this file: */
/*=========================================================*/
/*   (beginning of isram)           0xd1020000             */
/*   (bootrom common structs)          ...                 */
/*=========================================================*/
/*    __main         global         0xd102ba00             */
/*    text           section                               */
/*    rodata         setction                              */
/*    got            section                               */
/*    data           section                               */
/*    __bss_start    global                                */
/*    bss            section                               */
/*    __bss_end      global                                */
/*    __topofcode__  global                                */
/*    (end of isram)                0xd103ffff             */
/*=========================================================*/


SECTIONS
{

/**/
/**/
/* all code sections start at 0xd102ba00 */
	. = 0x01C00000;

	.__main = .;

	. = ALIGN(8);

	.text : {
		*(.text)
		. = ALIGN(8);
	}

    .rodata : { *(.rodata) }

    . = ALIGN(8);

	.got : { *(.got) }

    . = ALIGN(8);

	__egot = .;


/**/
/**/
/* data & bss start at 0xd1023000 */
	. = 0x01C16000;

/* correction: let all other sections */
/* just follow contiguously           */

	. = ALIGN(8);

    __data_start = .;
    .data : AT (__egot) {*(.data) }
	__data_end = .;

	. = ALIGN(8);

	__bss_start = .;
    .bss : { *(.bss) }
	__bss_end = .;


	. = ALIGN(8);

	.__topofcode__ = .;

	__data_size = __data_end - __data_start;

}


