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
	if (argc < 3)
		die("usage: %s can-interface can-address file", argv[0]);

	const char *can_if = argv[1];
	uint32_t can_address = parse_hex(argv[2]);

	FILE *f = fopen(argv[3], "r");
	if (!f)
		pdie("opening file failed");

	int fd = can_sock(can_if);
	if (fd < 0)
		die("creating can socket failed");

	if (can_filter(fd, can_address))
		pdie("setting can filter failed");

	int offset = 0;

	while (1) {
		uint8_t buf[8];

		int cnt;
		cnt = fread(&buf[2], 1, 6, f);
		if (cnt < 0)
			pdie("fread failed");

		if (cnt == 0)
			break;

		if (offset % 2048 == 0) {
			logm("hitting page boundary, triggering erase");

			buf[0] = 0x20;

			if (can_send(fd, can_address, buf, 1) < 0)
				pdie("can_send erase failed");

			if (can_recv(fd, buf, NULL, 1000) != 1)
				pdie("can_recv erase failed");

			if (buf[0] != 0)
				pdie("erase unsuccessful");
		}

		buf[0] = 0x21;
		buf[1] = 0x0;

		if (can_send(fd, can_address, buf, 2 + cnt) < 0)
			pdie("can_send flash failed");

		if (can_recv(fd, buf, NULL, 1000) != 1)
			pdie("can_recv flash failed");

		if (buf[0] != 0)
			pdie("flash unsuccessful");

		offset += cnt;
	}

	return EXIT_SUCCESS;
}