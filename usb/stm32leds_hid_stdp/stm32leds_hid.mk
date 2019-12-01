################################################################################
# stm32leds_hid.mk
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
# \copyright
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

COMPILER ?= GCC_ARM

ifneq ($(COMPILER), GCC_ARM)
$(error Only GCC ARM is supported at this moment)
endif

CUR_APP_PATH = $(CURDIR)

include ./targets.mk
include ./libs.mk
include ./toolchains.mk

# Application-specific DEFINES
DEFINES_APP := 

# Collect stm32leds_hid application sources
SOURCES_APP := $(wildcard $(CUR_APP_PATH)/source/*.c)

# Collect include directories for STM32_USB_OTG_Driver
INCLUDE_DIRS_APP := $(CUR_APP_PATH)/include
 
# Collect includes from application
INCLUDE_DIRS_APP := $(addprefix -I, $(INCLUDE_DIRS_APP))

ASM_FILES_APP :=

ifneq ($(COMPILER), GCC_ARM)
$(error Only GCC ARM is supported at this moment)
endif
ifeq ($(MAKEINFO) , 2)
$(info ==============================================================================)
$(info = Application specific files =)
$(info ==============================================================================)
$(info = Source files =)
$(info $(SOURCES_APP))
$(info ==============================================================================)
$(info = Included directories =)
$(info $(INCLUDE_DIRS_APP))
$(info ==============================================================================)
$(info = Added macros =)
$(info $(DEFINES_APP))
$(info = Linker script =)
$(info ==============================================================================)
$(info $(LINKER_SCRIPT))
endif
