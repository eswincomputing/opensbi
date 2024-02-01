/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 ****************************************************************
 *
 *  Copyright (C) 2022  Beiing Eswin Co. Ltd
 *
 *    This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  You should have received a copy of the GNU General Public License Version 2
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 ****************************************************************
 */

#include <libfdt.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_system.h>
#include <sbi/riscv_io.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/serial/uart8250.h>
#include <sbi_utils/timer/aclint_mtimer.h>
#include <sbi_utils/ipi/aclint_mswi.h>
#include <sbi_utils/fdt/fdt_pmu.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi/riscv_asm.h>
#include <sbi_utils/fdt/fdt_fixup.h>

/* clang-format off */
#ifdef BR2_CHIPLET_1
#ifdef BR2_CHIPLET_1_DIE0_AVAILABLE
#define EIC770X_HART_COUNT				4
#define	DIE_REG_OFFSET				0
#else
#define EIC770X_HART_COUNT				8
#define	DIE_REG_OFFSET				0x20000000
#endif
#else //BR2_CHIPLET_2
#define EIC770X_HART_COUNT				8
#define	DIE_REG_OFFSET				0
#endif

#define EIC770X_ACLINT_MSWI_ADDR			0x2000000
#define EIC770X_ACLINT_MTIMER_ADDR			0x2000000
#define EIC770X_ACLINT_MTIMER_FREQ			1000000
//#define EIC770X_ACLINT_MTIMER_FREQ			2000000

#define EIC770X_PLIC_ADDR				0xc000000
#define EIC770X_PLIC_NUM_SOURCES			520
#define EIC770X_PLIC_NUM_PRIORITIES		7

#define EIC770X_UART0_ADDR				(0x50900000UL + DIE_REG_OFFSET)
#define EIC770X_UART_BAUDRATE			115200

#define EIC770X_UART_CLK       (200000000UL)

/* Full tlb flush always */
#define EIC770X_TLB_RANGE_FLUSH_LIMIT		0

/* system reset register */
#define EIC770X_SYS_RESET_ADDR	0x51828300UL
#define EIC770X_SYS_RESET_VALUE	0x1ac0ffe6

/* clang-format on */

static struct plic_data plic = {
	.addr = EIC770X_PLIC_ADDR,
	.num_src = EIC770X_PLIC_NUM_SOURCES,
};

static struct aclint_mswi_data mswi = {
        .addr = EIC770X_ACLINT_MSWI_ADDR,
        .size = ACLINT_MSWI_SIZE,
        .first_hartid = 0,
        .hart_count = EIC770X_HART_COUNT,
};

static struct aclint_mtimer_data mtimer = {
	.mtime_freq = EIC770X_ACLINT_MTIMER_FREQ,
	.mtime_addr = EIC770X_ACLINT_MTIMER_ADDR +
		      0xbff8,
	.mtime_size = ACLINT_DEFAULT_MTIME_SIZE,
	.mtimecmp_addr = EIC770X_ACLINT_MTIMER_ADDR +
			 0x4000,
	.mtimecmp_size = ACLINT_DEFAULT_MTIMECMP_SIZE,
        .first_hartid = 0,
	.hart_count = EIC770X_HART_COUNT,
	.has_64bit_mmio = false,
};

#ifdef BR2_CHIPLET_1
#ifdef BR2_CHIPLET_1_DIE0_AVAILABLE
static u32 eic770x_hart_index2id[EIC770X_HART_COUNT] = {
	[0] = 0,
	[1] = 1,
	[2] = 2,
	[3] = 3,
};
#else
#ifdef BR2_CLUSTER_4_CORE
static u32 eic770x_hart_index2id[EIC770X_HART_COUNT] = {
	[0] = -1,
	[1] = -1,
	[2] = -1,
	[3] = -1,
	[4] = 4,
	[5] = 5,
	[6] = 6,
	[7] = 7,
};
#else
static u32 eic770x_hart_index2id[EIC770X_HART_COUNT] = {
	[0] = -1,
	[1] =  1,
	[2] = -1,
	[3] = -1,
	[4] = -1,
	[5] = -1,
	[6] = -1,
	[7] = -1,
};
#endif
#endif
#endif

#ifdef BR2_CHIPLET_2
static u32 eic770x_hart_index2id[EIC770X_HART_COUNT] = {
	[0] = 0,
	[1] = 1,
	[2] = 2,
	[3] = 3,
	[4] = 4,
	[5] = 5,
	[6] = 6,
	[7] = 7,
};
#endif

static void eic770x_modify_dt(void *fdt)
{
	fdt_cpu_fixup(fdt);

	fdt_fixups(fdt);

	/*
	 * SiFive Freedom U540 has an erratum that prevents S-mode software
	 * to access a PMP protected region using 1GB page table mapping, so
	 * always add the no-map attribute on this platform.
	 */
	/*当需要在resever memory中使能CMA内存时，需要关闭该函数的调用*/
	/*fdt_reserved_memory_nomap_fixup(fdt);*/
}
static int eic770x_system_reset_check(u32 type, u32 reason)
{
	return 1;
}

static void eic770x_system_reset(u32 type, u32 reason)
{
	writel(EIC770X_SYS_RESET_VALUE, (volatile void *)EIC770X_SYS_RESET_ADDR);

	while (1);
}

static struct sbi_system_reset_device eic770x_reset = {
	.name = "eswin_eic770x_reset",
	.system_reset_check = eic770x_system_reset_check,
	.system_reset = eic770x_system_reset
};

static int eic770x_nascent_init(void)
{
	unsigned long hwpf;	// Hardware Prefetcher 0 : 0x104095C1BE241 | Hardware Prefetcher 1 : 0x38c84e

	hwpf = 0x104095C1BE241UL;
	__asm__ volatile("csrw 0x7c3 , %0" : : "r"(hwpf));
	hwpf = 0x38c84eUL;
	__asm__ volatile("csrw 0x7c4 , %0" : : "r"(hwpf));

	return 0;
}

static int eic770x_early_init(bool cold_boot)
{
	if (cold_boot)
		sbi_system_reset_add_device(&eic770x_reset);

	return 0;
}

static int eic770x_final_init(bool cold_boot)
{
	void *fdt;

	if (!cold_boot)
		return 0;

	fdt = sbi_scratch_thishart_arg1_ptr();
	eic770x_modify_dt(fdt);

	return 0;
}

static int eic770x_console_init(void)
{
	return uart8250_init(EIC770X_UART0_ADDR,
			     EIC770X_UART_CLK,
			     EIC770X_UART_BAUDRATE,
			     0x2,
			     0x2,
			     0);
}

static int eic770x_irqchip_init(bool cold_boot)
{
	int rc;
	u32 hartid = current_hartid();

	if (cold_boot) {
		rc = plic_cold_irqchip_init(&plic);
		if (rc)
			return rc;
	}

	return plic_warm_irqchip_init(&plic, 2 * hartid ,2 * hartid + 1);
}

static int eic770x_ipi_init(bool cold_boot)
{
        int rc;

        if (cold_boot) {
                rc = aclint_mswi_cold_init(&mswi);
                if (rc)
                        return rc;
        }

        return aclint_mswi_warm_init();

}

static u64 eic770x_get_tlbr_flush_limit(void)
{
	return EIC770X_TLB_RANGE_FLUSH_LIMIT;
}

static int eic770x_timer_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = aclint_mtimer_cold_init(&mtimer, NULL);
		if (rc)
			return rc;
	}

	return aclint_mtimer_warm_init();
}

static int generic_pmu_init(void)
{
	return fdt_pmu_setup(fdt_get_address());
}

static uint64_t generic_pmu_xlate_to_mhpmevent(uint32_t event_idx,
					       uint64_t data)
{
	uint64_t evt_val = 0;

	/* data is valid only for raw events and is equal to event selector */
	if (event_idx == SBI_PMU_EVENT_RAW_IDX)
		evt_val = data;
	else {
		/**
		 * Generic platform follows the SBI specification recommendation
		 * i.e. zero extended event_idx is used as mhpmevent value for
		 * hardware general/cache events if platform does't define one.
		 */
		evt_val = fdt_pmu_get_select_value(event_idx);
		if (!evt_val)
			evt_val = (uint64_t)event_idx;
	}

	return evt_val;
}

const struct sbi_platform_operations platform_ops = {
	.nascent_init		= eic770x_nascent_init,
	.early_init		= eic770x_early_init,
	.final_init		= eic770x_final_init,
	.console_init		= eic770x_console_init,
	.irqchip_init		= eic770x_irqchip_init,
	.ipi_init		= eic770x_ipi_init,
	.get_tlbr_flush_limit	= eic770x_get_tlbr_flush_limit,
	.timer_init		= eic770x_timer_init,
	.pmu_init		= generic_pmu_init,
	.pmu_xlate_to_mhpmevent = generic_pmu_xlate_to_mhpmevent,
};

const struct sbi_platform platform = {
	.opensbi_version	= OPENSBI_VERSION,
	.platform_version	= SBI_PLATFORM_VERSION(0x0, 0x01),
	.name			= "ESWIN EIC770X",
	.features		= 0,
	.hart_count		= EIC770X_HART_COUNT,
	.hart_index2id		= eic770x_hart_index2id,
	.hart_stack_size	= SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
	.heap_size		= SBI_PLATFORM_DEFAULT_HEAP_SIZE(EIC770X_HART_COUNT),
	.platform_ops_addr	= (unsigned long)&platform_ops
};
