### Variant 1

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