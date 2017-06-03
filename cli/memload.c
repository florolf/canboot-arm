#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "common.h"

int parse_hex(const char *s)
{
	if (!strncmp(s, "0x", 2))
		s += 2;

	return strtol(s, NULL, 16);
}

int main(int argc, char **argv)
{
	if (argc < 4)
		die("usage: %s can-interface can-address load-address file", argv[0]);

	const char *can_if = argv[1];
	uint32_t can_address = parse_hex(argv[2]);
	uint32_t load_address = parse_hex(argv[3]);

	FILE *f = fopen(argv[4], "r");
	if (!f)
		pdie("opening file failed");

	int fd = can_sock(can_if);
	if (fd < 0)
		die("creating can socket failed");

	if (can_filter(fd, can_address))
		pdie("setting can filter failed");

	uint8_t membuf[4096];
	memset(membuf, 0, sizeof(membuf));

	int cnt;
	cnt = fread(membuf, 1, sizeof(membuf), f);
	if (cnt < 0)
		pdie("fread failed");

	cnt = (cnt + 3) & (~3);

	bl_set_pointer(fd, can_address, load_address);

	printf("writing %d bytes\n", cnt);
	printf("%d\n", bl_write_mem(fd, can_address, membuf, cnt));

	bl_set_pointer(fd, can_address, load_address | 1);
}
