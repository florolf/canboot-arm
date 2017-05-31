#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/flash.h>

#define N (1024 / 2)
#define FLASH_START 0x8000000

static void wait_busy(void)
{
	__asm__ volatile ("nop");

	while ((FLASH_SR & FLASH_SR_BSY))
		;

	FLASH_SR |= FLASH_SR_EOP;
}

char data_start;

void __attribute__ ((naked, noreturn)) main(void)
{
	uint16_t *dst = (uint16_t*) FLASH_START;
	uint16_t *src = (uint16_t*) &data_start;

	// erase first sector
	FLASH_CR |= FLASH_CR_PER;
	FLASH_AR = FLASH_START;
	FLASH_CR |= FLASH_CR_STRT;

	wait_busy();

	FLASH_CR &= ~FLASH_CR_PER;

	for (uint32_t i = 0; i < N; i++) {
		FLASH_CR |= FLASH_CR_PG;

		*dst++ = *src++;

		wait_busy();
	}

	FLASH_CR &= ~FLASH_CR_PG;

	// reset
	SCB_AIRCR |= SCB_AIRCR_SYSRESETREQ;
	while (1)
		;
}
