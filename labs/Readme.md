# Lab Work 1

## Variant 1

Implement a module, that expose number of Button presses via node in `/dev` file system.  

**Technical requirements:**

1. Use provided GPIO driver gpio_lkm.c as base
2. Extend its functionality with interrupt handler, which will signalize of button presses
3. Expose kernel module symbol from gpio_lkm.c so that other modules can get this value
4. Implement separate module, which will consume counts data from `gpio_lkm.c` driver
5. Implement device node exposition to `/dev` file system, which returns number of counts at read request
6. Light up LED connected to board after 10 presses which will signalized device error
7. Any read/write operations to gpios should be refused after that

**Challenge:**

Think about handling situalion, when some other service would try to change state or direction of GPIOs, that you assigned to be connected to button and LED.

**_Other things to consider:_**

   - debouncing mechanism for a button, maybe checking time between triggers
   - handling situation when button is pushed constantly, like it stuck looks like it is also a device error and LED should start flashing signalizing about it


### Variant 2

Implement a 7 segment display controller driver that expose its interface to `/sys`

**Technical requirements:**

1. Use provided GPIO driver gpio_lkm.c as base
2. Extend its functionality with functions to deal with 7 seg display and export them to kerned space
3. Implement separate 7seg driver module that use these interfaces to lighten up display
4. Expose user API to /sys file system. It should support setting a number on display via write request and
getting a number currently displayed via read request

**Challange:**

Think about implementation of displaying letters task as board developer. Which approach is more convenient for users of your board - have decoding library in user space on in-driver decoding implementation with exposed interface.

**_Other things to consider:_**

We all try to save some energy. It would be good for our 7seg display driver to have sleep mode to save some watts. We need to be able to turn it off after 10 sec without read of write request. But still need to recover displaying new data after new write request or display previous data at read request. 


### Bonus:

Build linux kernel with your newly developed drivers included in linux source code tree. Add a node to Kconfig file in /drivers/char to describe your driver and expose it to kernel configuration system. Not forget to add dependencies. It should also be installed on board and work.

# Lab Work 2

This lab work is intended to master understanding of some higher level features and properties of device drivers in Linux.

**Topics to cover in this lab work:**

1. Linux Device Trees - writing, compiling 
2. Installation of drives and device tree binaries to a system
3. Bus devices drivers on I2C, USB examples
4. Autodetection (plug and play) and device/driver binding
5. Industrial I/O kernel subsystem as an example of higher level interface for certain kind of drivers (sensors)

**Folders to work with:**

* kobject
* iio
* usb

**Excercise:**

1. Acquire one of `I2C` sensors available in your lab, for example module with `bmp085`, `bmp180`, `vl53l0x` chip or other magnetometer, accelerometer, gyroscope, etc.

2. Follow `iio/Readme.md` and perform following:

   * determine if driver for your device is present in kernel
   * if not - find driver in Linux Kernel and compile it following insrutctions
   * write and compile device tree file
   * install device tree file and kernel module driver file to Raspberry Pi and enable loading at bootup
   * enshure sensor is available in `/sys/bus/iio/devices/iio:device*`
   * check interfaces provided by driver to `/sys/bus/iio/devices/iio:device*` subsystem and explore its implementation in your sensor driver code

3. Follow `usb/Readme.md` and perform following:

   * acquire `STM32F4Discovery` board if available and proceed with section `Preparing STM32F4Discovery board for use with example`
   * acquire some USB HID device, for example mouse
   * compile and install stm32leds_driver to your system, using guide from step 2
   * compile and run user space program `kobject/uevent`
   * instert USB device to Raspberry Pi and ensure plug and play events are displayed
   * find out VID:PID pair for your device and check what driver is binded to it
   * unbind this driver and bind your device with `STM32Leds` driver
   * write `udev` rule for automatic binding of your device with `STM32Leds` driver, following instruction
   * reboot system and ensure plug and play USB HID device is binded automatically with your device

`NOTE:` there is a branch in this repository called `stm32_leds_fw` where code and instructuions on how to build USB HID device implementation on `STM32F4Discovery` provided. `Standard Peripheral Driver` framework is used there.

It will be improved to support `STM32F411Discovery` board widely available in your lab and newer implementation of BSP for STM32 - `STM32 HAL and LL` framework.
