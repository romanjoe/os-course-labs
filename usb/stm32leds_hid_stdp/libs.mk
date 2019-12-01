################################################################################
# libs.mk
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

CUR_DRV_PATH = $(CURDIR)/drivers

# Collect source files for STM32_USB_Device_Library
SOURCES_USB_DEV := $(wildcard $(CUR_DRV_PATH)/STM32_USB_Device_Library/Class/hid_custom/src/*.c)
SOURCES_USB_DEV += $(wildcard $(CUR_DRV_PATH)/STM32_USB_Device_Library/Core/src/*.c)

# Collect source files for STM32_USB_OTG_driver
SOURCES_USB_OTG := $(wildcard $(CUR_DRV_PATH)/STM32_USB_OTG_driver/src/*.c)

# Collect source files for STM32F4xx_StdPeriph_Driver
SOURCES_USB_STD_PER := $(wildcard $(CUR_DRV_PATH)/STM32F4xx_StdPeriph_Driver/src/*.c)

# Collect include directories for STM32_USB_Device_Library
INCLUDE_DIRS_USB_DEV := $(CUR_DRV_PATH)/STM32_USB_Device_Library/Class/hid_custom/inc
INCLUDE_DIRS_USB_DEV += $(CUR_DRV_PATH)/STM32_USB_Device_Library/Core/inc

# Collect include directories for STM32_USB_OTG_Driver
INCLUDE_DIRS_USB_OTG := $(CUR_DRV_PATH)/STM32_USB_OTG_Driver/inc

# Collect include directories for STM32F4xx_StdPeriph_Driver
INCLUDE_DIRS_STD_PER := $(CUR_DRV_PATH)/STM32F4xx_StdPeriph_Driver/inc

# Collected all source files for drivers
SOURCES_DRV := $(SOURCES_USB_DEV)
SOURCES_DRV += $(SOURCES_USB_OTG)
SOURCES_DRV += $(SOURCES_USB_STD_PER)

# Collected all include directories for drivers
INCLUDE_DIRS_DRV := $(addprefix -I,$(INCLUDE_DIRS_USB_DEV))
INCLUDE_DIRS_DRV += $(addprefix -I,$(INCLUDE_DIRS_USB_OTG))
INCLUDE_DIRS_DRV += $(addprefix -I,$(INCLUDE_DIRS_STD_PER))

# Drivers specific definitions and macros
DEFINES_DRV := -DUSE_STDPERIPH_DRIVER

ifneq ($(COMPILER), GCC_ARM)
$(error Only GCC ARM is supported at this moment)
endif
ifeq ($(MAKEINFO) , 2)
$(info ==============================================================================)
$(info = Drivers specific files =)
$(info ==============================================================================)
$(info = Drivers files =)
$(info $(SOURCES_DRV))
$(info ==============================================================================)
$(info = Included directories =)
$(info $(INCLUDE_DIRS_DRV))
$(info ==============================================================================)
$(info = Added macros =)
$(info $(DEFINES_DRV))
endif
