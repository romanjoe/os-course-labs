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

# Defines whether or not show verbose build output
# 0 - no verbose build info
# 1 - verbose build info
# 2 - very verbose build info
MAKEINFO ?= 1

# Application name by default
APP_NAME ?= stm32leds_hid

# Build config by default
BUILDCFG ?= Debug

# Set of supported applications
APPS := stm32leds_hid

# Output folder
OUT := BUILD/$(APP_NAME)

# Output folder to contain build artifacts
OUT_TARGET := $(OUT)/$(TARGET)

OUT_CFG := $(OUT_TARGET)/$(BUILDCFG)

# Check if application is suppoted by build system
# include all other makefiles inherited from $(APP_NAME).mk
ifneq ($(filter $(APP_NAME), $(APPS)),)
include ./$(APP_NAME).mk
else
$(error Not supported application: '$(APP_NAME)')
endif

# Collect all the source files from other makefiles
C_FILES := $(SOURCES_APP)
C_FILES += $(SOURCES_TRG)
C_FILES += $(SOURCES_DRV)

# Collect all the include dirs from other makefiles
INCLUDE_DIRS := $(INCLUDE_DIRS_APP)
INCLUDE_DIRS += $(INCLUDE_DIRS_TRG)
INCLUDE_DIRS += $(INCLUDE_DIRS_DRV)

# Assign rule to get .o files from .c
O_FILES := $(notdir $(C_FILES:.c=.o)) $(addsuffix .o, $(notdir $(basename $(ASM_FILES))))

# Collect all the macro definitions from other makefiles
DEFINES := $(DEFINES_APP)
DEFINES += $(DEFINES_TRG)
DEFINES += $(DEFINES_DRV)

# Add macros to assembler run as well
AS_FLAGS += $(DEFINES)

# Update CFLAGS at this point as DEFINES are completed
CFLAGS += $(DEFINES)

VPATH = $(dir $(C_FILES) $(ASM_FILES))

#==========================================================#
#======= Path manipulations to support Cygwin =============#

# STDE: For cygwin, adjust paths for compiler
MY_FILES := $(subst /cygdrive/c,c:,$(C_FILES))
#$(info MY_FILES $(MY_FILES))
C_FILES=$(MY_FILES)

MY_DIRS := $(subst /cygdrive/c,c:,$(INCLUDE_DIRS))
#$(info MY_DIRS $(MY_DIRS))
INCLUDE_DIRS=$(MY_DIRS)

MY_ASM_FILES := $(subst /cygdrive/c,c:,$(ASM_FILES))
#$(info MY_ASM_FILES $(MY_ASM_FILES))
ASM_FILES=$(MY_ASM_FILES)

MY_LDFLAGS := $(subst /cygdrive/c,c:,$(LDFLAGS))
#$(info MY_LDFLAGS $(MY_LDFLAGS))
LDFLAGS=$(MY_LDFLAGS)
#==========================================================#

# Output names and directories definitions
OUT_OBJ := $(OUT_CFG)/obj
OUT_APP := $(OUT_CFG)
OUT_FILE_NAME ?= $(OUT_APP)/$(APP_NAME)

# Define build tasks
.PHONY: all app build clean clean_all build_info

all: clean app

app:
	@`mkdir -p ./$(OUT)`
	@`mkdir -p ./$(OUT_TARGET)`
	@`mkdir -p ./$(OUT_CFG)`
	@`mkdir -p ./$(OUT_OBJ)`
	$(MAKE) build -j8

build: $(OUT_APP)/$(APP_NAME).hex
	# Generate address listing of hex file)
	$(GCC_PATH)/bin/arm-none-eabi-objdump $(OUT_APP)/$(APP_NAME).hex -s > $(OUT_APP)/$(APP_NAME).lst
	# Generate info about application size by sections
	$(GCC_PATH)/bin/arm-none-eabi-size --format=SysV $(OUT_APP)/$(APP_NAME).elf

$(OUT_APP)/$(APP_NAME).hex: $(OUT_APP)/$(APP_NAME).elf
	# Generate .hex and .bin artifacts
	$(GCC_PATH)/bin/arm-none-eabi-objcopy --change-addresses=$(HEADER_OFFSET) -O ihex $(OUT_APP)/$(APP_NAME).elf $(OUT_APP)/$(APP_NAME).hex
	$(GCC_PATH)/bin/arm-none-eabi-objcopy -O binary $(OUT_APP)/$(APP_NAME).elf $(OUT_APP)/$(APP_NAME).bin

$(OUT_APP)/$(APP_NAME).elf: $(addprefix $(OUT_OBJ)/, $(O_FILES))
	@echo "LD $@"
ifeq ($(MAKEINFO), 1)
	@echo $(LD) $(O_FILES) $(CC_DEPEND) $(@:.o=.d) -o $@ $(LDFLAGS) -T $(LINKER_SCRIPT) -Wl,-Map,$(OUT_FILE_NAME).map
endif
	@$(LD) $(addprefix $(OUT_OBJ)/, $(O_FILES)) $(CC_DEPEND) $(@:.o=.d) -o $@ $(LDFLAGS) -T $(LINKER_SCRIPT) -Wl,-Map,$(OUT_FILE_NAME).map

# Compile .c source to .o files
$(OUT_OBJ)/%.o: %.c
	@echo "CC $<"
ifeq ($(MAKEINFO), 1)
	@echo $(CC) $(CFLAGS) $(INCLUDE_DIRS) $(CC_DEPEND) $(@:.o=.d) -c $< -o $@
endif
	@$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(CC_DEPEND) $(@:.o=.d) -c $< -o $@
ifeq ($(MAKEINFO), 1)
	@echo
endif

# Compile .S source to .o files
$(OUT_OBJ)/%.o: %.S
	@echo "AS $<"
ifeq ($(MAKEINFO), 1)
	@echo @$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(CC_DEPEND) $(@:.o=.d) -c $< -o $@
endif
	@$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(CC_DEPEND) $(@:.o=.d) -c $< -o $@

# Clean build directory
clean:
	@echo "Cleanup out build directory for $(OUT_TARGET)..."
	rm -rf ./$(OUT_TARGET) BUILD/build.log

clean_all:
	@echo "Cleanup whole BUILD directory..."
	rm -rf BUILD

ifeq ($(MAKEINFO), 1)
$(info ==============================================================================)
$(info = Compilation info: =)
$(info ==============================================================================)
$(info Directories included:)
$(info $(INCLUDE_DIRS))
$(info C Source files:)
$(info $(C_FILES))
$(info Macros defined:)
$(info $(DEFINES))
$(info Assembler files:)
$(info $(ASM_FILES))
$(info GCC compiler options:)
$(info $(CC) $(CFLAGS))
$(info ==============================================================================)
$(info = Compilation Started: =)
$(info ==============================================================================)
endif
