/* Script for linking kernel.elf  */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(kernel_start)

SECTIONS
{
	/* text section: start at 0xc1000000 */
	. = 0xc1000000;
	.text : {
		_stext = .;	/* define the '_stext' symbol to this value */
		*(.entry.text)
		*(.text)
		_etext = .;
	} = 0x9090		/* Internal space is filled with 'nop'. */

	/* data section */
	. = ALIGN(4096);
	.data : {
		_sdata = .;
		*(.entry.data)
		*(.data)
	}
	.rodata : {
		*(.rodata)
		*(.rodata1)
		_edata = .;
	}

	/* bss section */
	. = ALIGN(4096);
	.bss : {
		__bss_start = .;
		*(.bss)
		__bss_end = .;
	}
	_end = .;

	/* DWARF debug starts at 0, otherwise gdb cannot find debug info */
	.debug_aranges	0: { *(.debug_aranges) }	/* Does this exist? */
	.debug_ranges	0: { *(.debug_ranges) }
	.debug_pubnames	0: { *(.debug_pubnames) }
	.debug_info	0: { *(.debug_info) }
	.debug_abbrev	0: { *(.debug_abbrev) }
	.debug_line	0: { *(.debug_line) }
	.debug_frame	0: { *(.debug_frame) }
	.debug_str	0: { *(.debug_str) }
	.debug_pubtypes	0: { *(.debug_pubtypes) }

	/DISCARD/ : { *(.*) }

	. = ASSERT(_end <= 0xc2000000, "kernel image is larger than 16MB");
}
