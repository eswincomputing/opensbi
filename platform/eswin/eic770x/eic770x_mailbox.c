// SPDX-License-Identifier: BSD-2-Clause
/*
 *
 * Copyright 2026 Beijing ESWIN Computing Technology Co., Ltd.
 *
 * Authors:
 *   Hu Jiamiao <hujiamiao@eswincomputing.com>
 *
 */

#include <sbi/riscv_io.h>
#include "eic770x_mailbox.h"

extern int sbi_printf(const char *format, ...);

static int eswin_mailbox_lock_channel(eswin_mailbox_reg_t *tx_reg, u32 lock_bit)
{
	writel(readl(&tx_reg->wr_lock) & ~(0xfff), &tx_reg->wr_lock);
	writel(readl(&tx_reg->wr_lock) | lock_bit, &tx_reg->wr_lock);
	if (readl(&tx_reg->wr_lock) & lock_bit)
		return 0;
	return -1;
}

int eswin_mailbox_add_data(eswin_mbox_msg_t *msg, u8 *data, int data_len)
{
	u8 *pdata;
	int i;

	if (data_len > 8 || data_len < 0)
		return -1;

	pdata = (u8 *)&msg->data;

	if (0 == data_len) {
		pdata[0] = 0x5a;
	} else {
		for (i = 0; i < data_len; i++)
			pdata[i] = data[i];
	}
	return 0;
}

int eswin_mailbox_send_irq(eswin_mailbox_reg_t *tx_reg, eswin_mbox_msg_t *msg,
			u32 irq_bit)
{
	int     i,j = 0;
	u8      *pdata;
	u8      *data;
	u8      retry = 0;
	u32     tmp_data;
	u32     status;

	if (0 == msg)
		return -1;

	status = mailbox_readl(&tx_reg->fifo_status);
	if (status & BIT0)
		return -1;

	writel(readl(&tx_reg->int_ctrl) & ~ESWIN_MAIBOX_LPCPU_IRQ_BIT, &tx_reg->int_ctrl);

	while (retry <= 5) {
		if ((eswin_mailbox_lock_channel(tx_reg, ESWIN_MAILBOX_WR_LOCK_BIT_LPCPU) == 0) &&
				(mailbox_readl(&tx_reg->fifo_status) & BIT1)) {
			data = (u8 *)&msg->data;
			pdata = (u8 *)&tmp_data;

			for (i = 0; i < 8; i++) {
				pdata[j++] = data[i];
				if (i == 3) {
					mailbox_writel(tmp_data, &tx_reg->wr_data0);
					j = 0;
				} else if (i == 7) {
					writel(tmp_data & ~BIT31, &tx_reg->wr_data1);
					writel(readl(&tx_reg->wr_data1) | BIT31, &tx_reg->wr_data1);
				}
			}
			writel(readl(&tx_reg->int_ctrl) | irq_bit, &tx_reg->int_ctrl);
			break;
		} else {
			retry++;
			if (retry > 5)
				sbi_printf("eswin_mailbox_send_irq: failed\n");
		}
	}

	return 0;
}
