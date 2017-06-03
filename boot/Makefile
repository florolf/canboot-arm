TOPDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJ = main.o

CROSS_PREFIX ?= arm-none-eabi-
CC=$(CROSS_PREFIX)gcc
OBJCOPY=$(CROSS_PREFIX)objcopy
SIZE=$(CROSS_PREFIX)size

GIT_REV=$(shell git rev-parse --short=8 HEAD | tr -d '\n' | tac -rs ..)

CFLAGS = -Wall -Wextra -std=gnu99 -I../libopencm3/include -DSTM32F3 -mcpu=cortex-m4 -mthumb -fno-common -ffreestanding -Os -g -DGIT_REV=0x$(GIT_REV)
LDFLAGS = -L../libopencm3/lib -mthumb -mcpu=cortex-m4 -nostartfiles -Wl,--gc-sections -T linker.ld -g

all: boot.bin

boot.elf: $(OBJ)
	$(CC) -o boot.elf $^ $(LDFLAGS)
	$(SIZE) -B boot.elf

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o boot.elf boot.bin

.PHONY: all clean
