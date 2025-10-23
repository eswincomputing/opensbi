// SPDX-License-Identifier: BSD-2-Clause
/*
 * eic770x_dram.c is the dram API for getting the dram size of EIC770X SoC.
 *
 * Copyright 2024 Beijing ESWIN Computing Technology Co., Ltd.
 *
 * Authors:
 *   Min LIn <linmin@eswincomputing.com>
 *
 */
#include <sbi/sbi_console.h>
#include <sbi/riscv_locks.h>
#include <sbi/riscv_io.h>
#include <eswin/eic770x_dram.h>

/*
mr_type:0-write;1-read
rank: Ox1 - select rank 0 onlyOx2- select rank 1 onlyOx5 - select ranks 0 and 2OxA - select ranks 1 and 3OxF - select ranks 0, 1, 2 and 3
addr:mrX*/
int mr_operation(unsigned long base_addr, uint8_t mr_type, uint8_t rank, uint8_t addr)
{
	uint32_t value = 0;
	uint32_t data = 0;

	volatile void *ctrl_base_addr = (volatile void *)base_addr;

	/* 1. If performing MRR and using MRRDATAO/MRRDATA1 registers, write the MRCTRLO.mrr_done_clr to '1'.
	This bit is self-clearing, and clears the MRSTAT.mrr_done register. */
	value = readl(ctrl_base_addr + DDR_CTRL_MRCTRL0);
	value = value | (1 << 24);
	writel(value, (ctrl_base_addr + DDR_CTRL_MRCTRL0));

	/* 2. Poll MRSTAT.mr_wr_busy until it is 'O'. This checks that there is no outstanding MR transaction.
	No writes must be performed to MRCTRLO and MRCTRL1 if MRSTAT .mr_wr_busy=1. */
	while (1) {
		value = readl(ctrl_base_addr + DDR_CTRL_MRSTAT);
		if (0x0 == (value & 0x1)) {
			break;
		}
	}

	/*   3. Write the MRCTRLO.mr_type,MRCTRLO.mr_addr,MRCTRLO.mr_rank and (for MRWs) MRCTRL1 .mr_data to define the MR transaction. */
	value = readl(ctrl_base_addr + DDR_CTRL_MRCTRL0);
	value = value & 0xFFFF0FFE; // if can't read,modify 0xFFFF0FCE to 0xFFFF0FFE
	value = value | (mr_type & 0x1) | ((rank & 0x3) << 4);
	writel(value, (ctrl_base_addr + DDR_CTRL_MRCTRL0));
	value = ((addr & 0xff) << 8) | (data & 0xff);
	writel(value, (ctrl_base_addr + DDR_CTRL_MRCTRL1));

	/* 4. In a separate APB transaction, write the MRCTRLO.mr wr to '1'. This bit is self-clearing, and triggers the MR transaction.
	The DDRCTL then asserts the MRSTAT.mr wr busy while it performs the MR transaction to SDRAM, and no further accesses can be initiated until it is de-asserted. */
	value = readl(ctrl_base_addr + DDR_CTRL_MRCTRL0);
	value = value | (1u << 31);
	writel(value, (ctrl_base_addr + DDR_CTRL_MRCTRL0));
	if (MR_TYPE_READ == mr_type) {
		/* 5. If performing MRR, the MRR data is made available on the hif_mrr_data signals and theMRRDATAO/MRRDATA1 registers. */
		while (1) {
			value = readl(ctrl_base_addr + DDR_CTRL_MRSTAT);
			if (0x1 == ((value >> 16) & 0x1)) {
				break;
			}
		}
		value = readl(ctrl_base_addr + DDR_CTRL_MRRDATA0);
	}
	return value;
}

uint32_t ddr_sw_mr_size_mb(uint8_t mr_value)
{
	uint32_t chip_num = 0;
	uint32_t density = 0;
	uint8_t cache = ((mr_value >> 6) & 0x3);
	uint32_t chn_ddr_size_mb = 0;

	/*0:X16     1:X8  others:reserved*/
	if (0 == cache) {
		chip_num = 2; /* 32/16 */
	} else if (1 == cache) {
		chip_num = 4; /* 32/8 */
	} else {
		// error, !!!not support IO Width
		while (1)
			;
	}
	cache = ((mr_value >> 2) & 0xf);
	sbi_printf("mr_value 0x%x\n", mr_value);

	switch (cache) {
	case 0:
		density = 2;
		break;
	case 1:
		density = 3;
		break;
	case 2:
		density = 4;
		break;
	case 3:
		density = 6;
		break;
	case 4:
		density = 8;
		break;
	case 5:
		density = 12;
		break;
	case 6:
		density = 16;
		break;
	case 7:
		density = 24;
		break;
	case 8:
		density = 32;
		break;
	default:
		// error, !!!not support density
		while (1)
			;
		break;
	}
	chn_ddr_size_mb = (density * 1024 /* gb to mb */ * chip_num * 2 /* rank */) >> 3; /* bit to Byte */
	return chn_ddr_size_mb;
}

void get_dram_info(struct bd_info *bd)
{
	uint64_t ctrl_base_addr;
	uint64_t dram_size_mb, dram_size_bytes;

	for (uint32_t i = 0; i < MAX_NR_DRAM_BANKS; i++) {
		ctrl_base_addr = DDR_CTRL_ADDR + i*0x20000000;
		dram_size_mb = (ddr_sw_mr_size_mb(mr_operation(ctrl_base_addr, MR_TYPE_READ, 0x1, 0x8))) * 2;
		dram_size_bytes = dram_size_mb << 20;
		bd->bi_dram[i].size = dram_size_bytes;
	}
}
