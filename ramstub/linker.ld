ENTRY(main)

SECTIONS {
	.text : {
		*(.text*)
	}

	. = ALIGN(2);

	data_start = .;
}
