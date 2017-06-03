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
	if (argc < 2)
		die("usage: %s can-interface", argv[0]);

	const char *can_if = argv[1];

	int fd = can_sock(can_if);
	if (fd < 0)
		die("creating can socket failed");

	while (1) {
		uint8_t buf[8];
		canid_t id;
		int len;

		memset(buf, 0, sizeof(buf));

		if ((len = can_recv(fd, buf, &id, -1)) < 0) {
			perror("can_recv failed");
			continue;
		}

		if (memcmp(buf, "\xed\xe9\xb0\x07", 4))
			continue;

		id &= CAN_EFF_MASK;
		logm("new client with ID %08x", id);

		buf[0] = 0x00;
		if (can_send(fd, id, buf, 1) < 0)
			perror("can_send failed");
	}
}
