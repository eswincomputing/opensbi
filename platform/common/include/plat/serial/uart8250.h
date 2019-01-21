/*
 * Copyright (c) 2018 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __SERIAL_UART8250_H__
#define __SERIAL_UART8250_H__

#include <sbi/sbi_types.h>

void uart8250_putc(char ch);

char uart8250_getc(void);

int uart8250_init(unsigned long base,
		  u32 in_freq, u32 baudrate,
		  u32 reg_shift, u32 reg_width);

#endif