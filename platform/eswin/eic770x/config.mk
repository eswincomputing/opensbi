#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright 2024 Beijing ESWIN Computing Technology Co., Ltd.
#
# Authors:
#   XuXiang <xuxiang@eswincomputing.com>
#   LinMin <linmin@eswincomputing.com>
#   NingYu <ningyu@eswincomputing.com>
#

# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y = -fno-stack-protector

# Command for platform specific "make run"

# Blobs to build
ifeq ("$(chiplet)","BR2_CHIPLET_2")
ifeq ("$(mem_mode)","BR2_MEMMODE_INTERLEAVE")
FW_TEXT_START=0x4000000000
  # This needs to be 2MB aligned for 64-bit system
FW_JUMP_ADDR=0x4000200000
FW_JUMP_FDT_ADDR=0x4078000000
FW_PAYLOAD_OFFSET=0x200000
FW_PAYLOAD_FDT_ADDR=0x4078000000
else
FW_TEXT_START=0x80000000
  # This needs to be 2MB aligned for 64-bit system
FW_JUMP_ADDR=0x80200000
FW_JUMP_FDT_ADDR=0xf8000000
FW_PAYLOAD_OFFSET=0x200000
FW_PAYLOAD_FDT_ADDR=0xf8000000
endif
else #BR2_CHIPLET_1
ifeq ("$(chiplet_die_available)","BR2_CHIPLET_1_DIE0_AVAILABLE")
FW_TEXT_START=0x80000000
  # This needs to be 2MB aligned for 64-bit system
FW_JUMP_ADDR=0x80200000
FW_JUMP_FDT_ADDR=0xf8000000
FW_PAYLOAD_OFFSET=0x200000
FW_PAYLOAD_FDT_ADDR=0xf8000000
else
FW_TEXT_START=0x2000000000
  # This needs to be 2MB aligned for 64-bit system
FW_JUMP_ADDR=0x2000200000
FW_JUMP_FDT_ADDR=0x2078000000
FW_PAYLOAD_OFFSET=0x200000
FW_PAYLOAD_FDT_ADDR=0x2078000000
endif
endif
FW_DYNAMIC=y
FW_JUMP=y
FW_PAYLOAD=y
