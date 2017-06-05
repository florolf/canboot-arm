#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "common.h"

#define PAGE_SIZE 2048
#define APP_START 0x8000800

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

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

	int offset, total_length = 0;
	uint8_t buf[8], page[PAGE_SIZE];
	uint32_t hash = 0;

	if (bl_set_pointer(fd, can_address, APP_START))
		die("resetting pointer failed");

	while (1) {
		int cnt;
		cnt = fread(page, 1, sizeof(page), f);
		if (cnt < 0)
			pdie("fread failed");

		if (cnt == 0)
			break;

		total_length += cnt;
		jenkins_mix(&hash, page, cnt);

		logm("hitting page boundary, triggering erase");

		buf[0] = 0x20;

		if (can_send(fd, can_address, buf, 1) < 0)
			pdie("can_send erase failed");

		if (can_recv(fd, buf, NULL, 1000) != 1)
			pdie("can_recv erase failed");

		if (buf[0] != 0)
			pdie("erase unsuccessful");

		offset = 0;
		while (offset < PAGE_SIZE) {
			int writelen = MIN(PAGE_SIZE - offset, 6);

			buf[0] = 0x21;
			buf[1] = 0x0;

			memcpy(&buf[2], &page[offset], writelen);

			if (can_send(fd, can_address, buf, 2 + writelen) < 0)
				pdie("can_send flash failed");

			if (can_recv(fd, buf, NULL, 1000) != 1)
				pdie("can_recv flash failed");

			if (buf[0] != 0)
				pdie("flash unsuccessful");

			offset += writelen;
		}
	}

	hash = jenkins_finish(hash);

	if (bl_set_pointer(fd, can_address, APP_START))
		die("resetting pointer failed");

	uint32_t hash_out;
	if (bl_hash(fd, can_address, total_length, &hash_out) < 0)
		die("hashing failed");

	if (hash_out != hash)
		die("hash verification failed: Expected 0x%08x, got 0x%08x", hash, hash_out);
	else
		logm("flash verification successful");

	buf[0] = 0x30;
	if (can_send(fd, can_address, buf, 1) < 0)
		pdie("can_send exec failed");

	return EXIT_SUCCESS;
}
