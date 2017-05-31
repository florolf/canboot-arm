TOPDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJ = main.o

CROSS_PREFIX ?= arm-none-eabi-
CC=$(CROSS_PREFIX)gcc
OBJCOPY=$(CROSS_PREFIX)objcopy
SIZE=$(CROSS_PREFIX)size

GIT_REV=$(shell git rev-parse --short=8 HEAD)

CFLAGS = -Wall -Wextra -std=gnu99 -I../libopencm3/include -DSTM32F0 -mcpu=cortex-m0 -mthumb -fno-common -ffreestanding -O2 -fpie -g
LDFLAGS = -L../libopencm3/lib -mthumb -mcpu=cortex-m0 -nostartfiles -Wl,--gc-sections -T linker.ld -fpie -g

all: ramstub.bin

ramstub.elf: $(OBJ)
	$(CC) -o ramstub.elf $^ $(LDFLAGS)
	$(SIZE) -B ramstub.elf

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o ramstub.elf ramstub.bin

.PHONY: all clean
