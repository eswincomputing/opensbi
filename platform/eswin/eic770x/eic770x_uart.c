// SPDX-License-Identifier: BSD-2-Clause
/*
 * eic770x_uart.c is the UART2 APIs, which is used to communicate with
 * the BMC on HF106 board.
 *
 * Copyright 2024 Beijing ESWIN Computing Technology Co., Ltd.
 *
 * Authors:
 *   HuangYifeng <huangyifeng@eswincomputing.com>
 *
 */

#include <sbi/riscv_locks.h>
#include <sbi/riscv_io.h>
#include "eic770x_uart.h"

static volatile void *eic770x_uart8250_base;
static u32 eic770x_uart8250_in_freq;
static u32 eic770x_uart8250_baudrate;
static u32 eic770x_uart8250_reg_width;
static u32 eic770x_uart8250_reg_shift;

static spinlock_t eic770x_out_lock = SPIN_LOCK_INITIALIZER;

static u32 eic770x_get_reg(u32 num)
{
	u32 offset = num << eic770x_uart8250_reg_shift;

	if (eic770x_uart8250_reg_width == 1)
		return readb(eic770x_uart8250_base + offset);
	else if (eic770x_uart8250_reg_width == 2)
		return readw(eic770x_uart8250_base + offset);
	else
		return readl(eic770x_uart8250_base + offset);
}

static void eic770x_set_reg(u32 num, u32 val)
{
	u32 offset = num << eic770x_uart8250_reg_shift;

	if (eic770x_uart8250_reg_width == 1)
		writeb(val, eic770x_uart8250_base + offset);
	else if (eic770x_uart8250_reg_width == 2)
		writew(val, eic770x_uart8250_base + offset);
	else
		writel(val, eic770x_uart8250_base + offset);
}

static void eic770x_uart8250_putc(char ch)
{
	while ((eic770x_get_reg(EIC770X_UART_LSR_OFFSET) & EIC770X_UART_LSR_THRE) == 0)
		;

	eic770x_set_reg(EIC770X_UART_THR_OFFSET, ch);
}
#if 0
static int eic770x_uart8250_getc(void)
{
	if (eic770x_get_reg(EIC770X_UART_LSR_OFFSET) & EIC770X_UART_LSR_DR)
		return eic770x_get_reg(EIC770X_UART_RBR_OFFSET);
	return -1;
}
#endif
extern int sbi_printf(const char *format, ...);
static void eic770x_uart_snd(char *str, u32 len)
{
	spin_lock(&eic770x_out_lock);
	while (len--) {
		eic770x_uart8250_putc(*str);
		str++;
	}
	spin_unlock(&eic770x_out_lock);
}

int eic770x_uart8250_init(unsigned long base, u32 in_freq, u32 baudrate, u32 reg_shift,
		  u32 reg_width)
{
	u16 bdiv;
	u32 bdiv_f, base_baud;

	eic770x_uart8250_base      = (volatile void *)base;
	eic770x_uart8250_reg_shift = reg_shift;
	eic770x_uart8250_reg_width = reg_width;
	eic770x_uart8250_in_freq   = in_freq;
	eic770x_uart8250_baudrate  = baudrate;

	base_baud = eic770x_uart8250_baudrate * 16;
	bdiv = eic770x_uart8250_in_freq / base_baud;
	bdiv_f = eic770x_uart8250_in_freq % base_baud;

	bdiv_f = EIC770X_DIV_ROUND_CLOSEST(bdiv_f << 0x4, base_baud);

	/* Disable all interrupts */
	eic770x_set_reg(EIC770X_UART_IER_OFFSET, 0x00);
	/* Enable DLAB */
	eic770x_set_reg(EIC770X_UART_LCR_OFFSET, 0x80);

	if (bdiv) {
		/* Set divisor low byte */
		eic770x_set_reg(EIC770X_UART_DLL_OFFSET, bdiv & 0xff);
		/* Set divisor high byte */
		eic770x_set_reg(EIC770X_UART_DLM_OFFSET, (bdiv >> 8) & 0xff);

		eic770x_set_reg(EIC770X_UART_DLF_OFFSET, bdiv_f);
	}

	/* 8 bits, no parity, one stop bit */
	eic770x_set_reg(EIC770X_UART_LCR_OFFSET, 0x03);
	/* Enable FIFO */
	eic770x_set_reg(EIC770X_UART_FCR_OFFSET, 0x01);
	/* No modem control DTR RTS */
	eic770x_set_reg(EIC770X_UART_MCR_OFFSET, 0x00);
	/* Clear line status */
	eic770x_get_reg(EIC770X_UART_LSR_OFFSET);
	/* Read receive buffer */
	eic770x_get_reg(EIC770X_UART_RBR_OFFSET);
	/* Set scratchpad */
	eic770x_set_reg(EIC770X_UART_SCR_OFFSET, 0x00);

	return 0;
}

// Function to check message checksum
#if 0
static int check_checksum(Message *msg)
{
	unsigned char checksum = 0;
	checksum ^= msg->msg_type;
	checksum ^= msg->cmd_type;
	checksum ^= msg->data_len;
	for (int i = 0; i < msg->data_len; ++i) {
		checksum ^= msg->data[i];
	}
	return checksum == msg->checksum;
}
#endif

static void generate_checksum(Message *msg)
{
	unsigned char checksum = 0;
	checksum ^= msg->msg_type;
	checksum ^= msg->cmd_type;
	checksum ^= msg->data_len;
	for (int i = 0; i < msg->data_len; ++i) {
		checksum ^= msg->data[i];
	}
	msg->checksum = checksum;
}

int eic770x_uart2_init();
int transmit_message(Message *msg)
{
	eic770x_uart2_init();
	generate_checksum(msg);

	eic770x_uart_snd((char *)msg, sizeof(Message));

	return 0;
}