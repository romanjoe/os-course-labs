### stm32_leds_hid firmware to implement USB HID device

This folder contains code to build a firmware for `STMF4Discovery` board to make it a USB HID device.

This firmware is then used to demonstrate functionality of Linux USB device driver and Linux user space application. Driver code and user space application for it could be found in level up directory.

#### Functionality

TDB

#### Supported targets

Currently supported target is `STM32F407xx` which is a chip installed on `STM32F4Discovery` board.

#### Folder contents

`cmsis`    - headers for `CM4` ARM core
`device`   - headers for `stm32fxx` family
`drivers`  - source files to support USB core functionality and OTG mode, provided by `ST`, Standard Peripheral Driver - STM periphery support package
`include`  - files related to current application
`linker`   - linker script used for compilation of application. It is default script for `stm32f407xx` family
`source`   - source code of application
`*.mk Makefile` - makefiles to describe build procedure for application.
`program.sh` - helper script for programming of target board using `OpenOCD`

#### Build project

To build `Debug` configuration of application use command:

    make clean app TARGET=STM32F407xx MAKEINFO=1 BUILDCFG=Debug TOOLCHAIN_PATH=%YOUR_GCC_INSTALLATION_PATH%

To build `Release` configuration of application use command:

    make clean app TARGET=STM32F407xx MAKEINFO=1 BUILDCFG=Debug TOOLCHAIN_PATH=%YOUR_GCC_INSTALLATION_PATH%

To delete `BUILD` directory contents:

    make clean

#### Program binary

Build system produces a set of binaries - `.elf`, `.hex`, `bin` format. Suggested way to program a board is to use `OpenOCD`, as it is a crossplatform solution.

On Windows `STM32 ST-LINK Utility` can be used.

To program board with `OpenOCD` - install recent version of `OpenOCD`, change pathes to executable and scripts in `program.sh` or define environment variables `OPENOCD_BIN` and `OPENOCD_SCRIPTS`, `TARGET`, `BUILDCFG`

* Example for MacOS and Linux:

Add following string to your `~/.bashrc`. (Use pathes where you have OpenOCD installed)

    export OPENOCD_BIN=/usr/local/bin/openocd
    export OPENOCD_SCRIPTS=/usr/local/share/openocd

* Example for Windows:

Add to symbols `OPENOCD_BIN=C:\openocd\bin\openocd.exe` and `OPENOCD_SCRIPTS=C:\openocd\scripts` to system `Path`.

Or just hard code variables in `program.sh` script.

    OPENOCD_BIN=/usr/local/bin/openocd
    OPENOCD_SCRIPTS=/usr/local/share/openocd
    TARGET=STM32F407xx
    BUILDCFG=Debug

Third variant is to use provided `tasks.json` for `VSCode`. It has same variables needed to be set and then you can do all the jobs from `VSCode` tasks.

    "env": {
        "TARGET": "STM32F407xx",
        "OPENOCD_BIN": "/usr/local/bin/openocd",
        "OPENOCD_SCRIPTS": "/usr/local/share/openocd",
        "BUILDCFG": "Debug"
    }

Program board using next command:

    ./program.sh -a stm32leds_hid -p

Erase memory using next command:

    ./program.sh -e

#### Debugging

This folder includes configuration for debugging in `VSCode` environment.

To use it intall `VSCode` extension `Cortex Debug` and redefine pathes in `.vscode/settings.json` for appropriate to your system.

    {
        "cortex-debug.armToolchainPath": "/User/rnok/toolchains/gcc-arm-none-eabi-6/bin",
        "cortex-debug.openocdPath": "/usr/local/bin/openocd"
    }

Modify path to configuration file for `OpenOCD` in `.vscode/launch.json`:

    "configFiles": [
        "/usr/local/share/openocd/scripts/board/stm32f4discovery.cfg"
    ]