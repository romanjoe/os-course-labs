################################################################################
# targets.mk
#
# This file is a part of demo application of STM32 hid device
#
# Copyright (C) 2017 Roman Okhrimenko <mrromanjoe@gmail.com>
# 
# Description of this application, Linux driver to support it
# and user space application for demonstration can be found here
# <https://github.com/romanjoe/os-course-labs/usb>
#
################################################################################
# Copyright 2018-2019 Cypress Semiconductor Corporation
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

# default TARGET
TARGET ?= STM32F407xx
#
TARGETS := STM32F407xx

CUR_TARG_PATH := $(CURDIR)/device
TRG_PATH  := $(CUR_TARG_PATH)/STM32F4xx

# Collect C source files for TARGET BSP
SOURCES_TRG :=

# Collect dirrectories containing headers for TARGET
INCLUDE_DIRS_TRG := $(TRG_PATH)/Include

# Collect dirrectory containing cmsis headers
INCLUDE_DIRS_TRG += $(CURDIR)/cmsis/Include

# Add device name from BSP makefile to defines
DEFINES_TRG := -D$(TARGET) 
DEFINES_TRG += -DSTM32F4XX 

# Collect all include directories for drivers
INCLUDE_DIRS_TRG := $(addprefix -I,$(INCLUDE_DIRS_TRG))

# Overwite path to linker script if custom is required, otherwise default from BSP is used
ifeq ($(COMPILER), GCC_ARM)
LINKER_SCRIPT := $(CUR_APP_PATH)/linker/$(TARGET).ld
else
$(error Only GCC ARM is supported at this moment)
endif

ifneq ($(COMPILER), GCC_ARM)
$(error Only GCC ARM is supported at this moment)
endif
ifeq ($(MAKEINFO) , 2)
$(info ==============================================================================)
$(info = TARGET specific files =)
$(info ==============================================================================)
$(info = Source files =)
$(info $(SOURCES_TRG))
$(info ==============================================================================)
$(info = Included directories =)
$(info $(INCLUDE_DIRS_TRG))
$(info ==============================================================================)
$(info = Added macros =)
$(info $(DEFINES_TRG))
endif
