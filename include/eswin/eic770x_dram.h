// SPDX-License-Identifier: BSD-2-Clause
/*
 * The header file of eic770x_dram.c.
 *
 * Copyright 2024 Beijing ESWIN Computing Technology Co., Ltd.
 *
 * Authors:
 *   Min Lin <linmin@eswincomputing.com>
 *
 */
#ifndef _EIC770X_DRAM_H_
#define _EIC770X_DRAM_H_

#ifdef BR2_CHIPLET_2
#define MAX_NR_DRAM_BANKS 2
#else
#define MAX_NR_DRAM_BANKS 1
#endif

#define MR_TYPE_READ 1
#define MR_TYPE_WRITE 0

#define DDR_CTRL_ADDR 0x52300000

#define DDR_CTRL_MSTR0	0x10000
#define DDR_CTRL_MRCTRL0 0x10080
#define DDR_CTRL_MRCTRL1 0x10084

#define DDR_CTRL_MRSTAT 0x10090

#define DDR_CTRL_MRRDATA0 0x10094
#define DDR_CTRL_MRRDATA1 0x10098

struct bd_info {
	struct bdin_dram {			/* RAM configuration */
		unsigned long start;
		unsigned long size;
	} bi_dram[MAX_NR_DRAM_BANKS];
};

void get_dram_info(struct bd_info *bd);

#endif
