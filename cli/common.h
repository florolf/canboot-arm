#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/can/raw.h>

#define logm(format, ...) do { fprintf(stderr, format "\n", ##__VA_ARGS__); } while(0)
#define die(format, ...) do { logm(format, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)
#define pdie(string, ...) do { perror(string); exit(EXIT_FAILURE); } while(0)

void jenkins_mix(uint32_t *state, uint8_t *data, uint32_t len);
uint32_t jenkins_finish(uint32_t state);
uint32_t jenkins(uint8_t *data, uint32_t seed, uint32_t len);

int can_sock(const char *ifname);
int can_send(int fd, uint32_t id, uint8_t *data, uint8_t len);
int can_recv(int fd, uint8_t data[static 8], canid_t *id, long int timeout);
int can_filter(int fd, uint32_t id);

int bl_get_chip_type(int fd, uint32_t id);
int bl_set_pointer(int fd, uint32_t id, uint32_t addr);
int bl_exec(int fd, uint32_t id);
int bl_write_mem(int fd, uint32_t id, uint8_t *data, size_t len);
