MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 1K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 6K
}

EXTERN(nvic_table)
ENTRY(main)

SECTIONS {
	_estack = ORIGIN(ram) + LENGTH(ram);

	.text : {
		KEEP(*(.nvic_table))

		*(.text*)

		*(.rodata*)

		_etext = .;
	} > rom

	.bss : {
		_bstart = .;
		*(.bss)
		*(COMMON)
		_bend = .;

	} > ram
}
