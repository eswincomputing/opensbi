// SPDX-License-Identifier: BSD-2-Clause
/*
 *
 * Copyright 2026 Beijing ESWIN Computing Technology Co., Ltd.
 *
 * Authors:
 *   Hu Jiamiao <hujiamiao@eswincomputing.com>
 *
 */

#ifndef _EIC770X_MAILBOX_H_
#define _EIC770X_MAILBOX_H_

typedef struct {
	u32 wr_data0;
	u32 wr_data1;
	u32 rd_data0;
	u32 rd_data1;
	u32 fifo_status;
	u32 mb_err;
	u32 int_ctrl;
	u32 wr_lock;
} eswin_mailbox_reg_t;

typedef struct eswin_mbox_msg {
	u64 data;
} eswin_mbox_msg_t;

#define BIT0	((u32)1 << 0)
#define BIT1	((u32)1 << 1)
#define BIT2	((u32)1 << 2)
#define BIT31	((u32)1 << 31)

#define ESWIN_MAILBOX_WR_LOCK_BIT_LPCPU	BIT2
#define ESWIN_MAIBOX_LPCPU_IRQ_BIT		BIT2

#define mailbox_writel(val, addr)		writel(val, addr)
#define mailbox_readl(addr)			readl(addr)

int eswin_mailbox_add_data(eswin_mbox_msg_t *msg, u8 *data, int data_len);
int eswin_mailbox_send_irq(eswin_mailbox_reg_t *tx_reg, eswin_mbox_msg_t *msg, u32 irq_bit);

#endif /* _EIC770X_MAILBOX_H_ */
