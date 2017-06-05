#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <errno.h>
#include <poll.h>

#include "common.h"

static int get_ifindex(int fd, const char *ifname)
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(fd, SIOGIFINDEX, &ifr) < 0)
		return -1;

	return ifr.ifr_ifindex;
}

// Jenkins One-at-a-Time hash with additive seed
void jenkins_mix(uint32_t *state, uint8_t *data, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++) {
		*state += data[i];
		*state += (*state << 10);
		*state ^= (*state >> 6);
	}
}

uint32_t jenkins_finish(uint32_t state)
{
	state += (state << 3);
	state ^= (state >> 11);
	state += (state << 15);

	return state;
}

uint32_t jenkins(uint8_t *data, uint32_t seed, uint32_t len)
{
	uint32_t hash, i;

	hash = seed;

	jenkins_mix(&hash, data, len);

	return jenkins_finish(hash);
}

int can_sock(const char *ifname)
{
	int fd;

	fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (fd < 0) {
		perror("opening can socket failed");
		return -1;
	}

	struct sockaddr_can addr;
	addr.can_family = AF_CAN;
	addr.can_ifindex = get_ifindex(fd, ifname);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("binding can socket failed");
		return -1;
	}

	return fd;
}

int can_send(int fd, uint32_t id, uint8_t *data, uint8_t len)
{
	struct can_frame msg;
	ssize_t wlen;

	msg.can_id = CAN_EFF_FLAG | id;
	msg.can_dlc = len;
	memcpy(msg.data, data, len);

	wlen = write(fd, &msg, sizeof(msg));
	while (wlen < 0 && errno == ENOBUFS) {
		usleep(10000);
		wlen = write(fd, &msg, sizeof(struct can_frame));
	}

	if(wlen < 0)
		return -1;

	if(wlen != sizeof(struct can_frame))
		return -1;

	return 0;
}

int can_recv(int fd, uint8_t data[static 8], canid_t *id, long int timeout)
{
	struct pollfd pfd;

	pfd.fd = fd;
	pfd.events = POLLIN;

	if(timeout != -1) {
		int ret = poll(&pfd, 1, timeout);
		if(ret == 0) {
			errno = ETIMEDOUT;
			return -1;
		}

		if(ret != 1)
			return -1;
	}

	struct can_frame msg;
	ssize_t len = read(fd, &msg, sizeof(struct can_frame));
	if(len < 0)
		return -1;

	if(len != sizeof(struct can_frame))
		return -1;

	memcpy(data, msg.data, msg.can_dlc);
	if (id)
		*id = msg.can_id;

	return msg.can_dlc;
}

int can_filter(int fd, uint32_t id)
{
	struct can_filter filter;

	filter.can_id = id | CAN_EFF_FLAG;
	filter.can_mask = (CAN_EFF_FLAG | CAN_EFF_MASK);

	return setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));
}

static void put_le32(uint8_t *buf, uint32_t val)
{
	for (int i = 0; i < 4; i++) {
		*buf++ = val & 0xFF;
		val >>= 8;
	}
}

int bl_get_chip_type(int fd, uint32_t id)
{
	uint8_t buf[8] = {0};

	buf[0] = 0;
	if (can_send(fd, id, buf, 1) < 0)
		return -1;

	if (can_recv(fd, buf, NULL, 1000) != 1)
		return -1;

	return buf[0];
}

int bl_set_pointer(int fd, uint32_t id, uint32_t addr)
{
	uint8_t buf[8] = {0};

	buf[0] = 0x10;
	put_le32(&buf[4], addr);

	if (can_send(fd, id, buf, 8) < 0)
		return -1;

	if (can_recv(fd, buf, NULL, 1000) != 1)
		return -1;

	return buf[0] == 0;
}

int bl_exec(int fd, uint32_t id)
{
	uint8_t buf[8] = {0x31};

	if (can_send(fd, id, buf, 1) < 0)
		return -1;
}

int bl_write_mem(int fd, uint32_t id, uint8_t *data, size_t len)
{
	uint8_t buf[8] = {0};

	while (len) {
		buf[0] = 0x13;

		memcpy(&buf[4], data, 4);
		data += 4;
		len -= 4;

		if (can_send(fd, id, buf, 8) < 0)
			return -1;

		if (can_recv(fd, buf, NULL, 1000) != 1)
			return -1;

		if (buf[0] != 0)
			return -1;
	}

	return 0;
}
