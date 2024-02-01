#
# SPDX-License-Identifier: BSD-2-Clause
#
#***************************************************************
#
#  Copyright (C) 2022  Beiing Eswin Co. Ltd
#
#    This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License Version 2
#  as published by the Free Software Foundation.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#  You should have received a copy of the GNU General Public License Version 2
#  along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#
#***************************************************************
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
