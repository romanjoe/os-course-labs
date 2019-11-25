### USB HID device driver example

This example driver is designed to be used with companion firmware running on widely available STM32F4Discovery board. STM32 firmware implements OTG HID device. STM32Leds driver is designed to support that device from linux kernel perspective. Example application for linux user space is also provided.

**Preparing STM32F4Discovery board for use with example**

By now `hex` file with firmware provided. Source code will be added as soon as it would be ported to crossplatform build system.

For now flash firmware to the board.

For Mac and Linux the most useful is using OpenOCD:

    $OPENOCD_PATH/bin/openocd -s $OPENOCD_PATH/scripts -f $OPENOCD_PATH/scripts/interface/stlink.cfg -f $OPENOCD_PATH/scripts/target/stm32f4x.cfg -c "init; reset init; program stm32leds_firmware.hex" -c "reset; shutdown"

For Windows users well, but they can also use `STM32 ST-LINK Utility` which is programmer with graphical user interface.

**Manual binding USB device with custom driver.**

Default behaviour for linux system is to bind standard `usbhid` device driver, once new device of that type plugged to the system.

Because of that in order to use device it is needed to perform binding of driver with device. Here is how it can be done manually.

1. First find device id associated with usbhid driver usind `tree` command (may not be installed - `sudo apt-get tree -y`). Example output:
        
        root@raspberrypi:/sys/bus/usb/drivers/usbhid# tree
        .
        |-- 1-1.1.2:1.0 -> ../../../../devices/platform/soc/3f980000.usb/usb1/1-1/1-1.1/1-1.1.2/1-1.1.2:1.0
        |-- bind
        |-- module -> ../../../../module/usbhid
        |-- new_id
        |-- remove_id
        |-- uevent
        `-- unbind

2. USB device id we are interested in is `1-1.1.2:1.0`. Now unbind it from `usbhid`

        echo 1-1.1.2:1.0 > /sys/bus/usb/derivers/usbhid/unbind

3. Chech if it is unbinded:

        pi@raspberrypi:~ $ lsusb -t
        /:  Bus 01.Port 1: Dev 1, Class=root_hub, Driver=dwc_otg/1p, 480M
        |__ Port 1: Dev 2, If 0, Class=Hub, Driver=hub/4p, 480M
            |__ Port 1: Dev 3, If 0, Class=Hub, Driver=hub/3p, 480M
                |__ Port 1: Dev 5, If 0, Class=Vendor Specific Class, Driver=lan78xx, 480M
                |__ Port 2: Dev 6, If 0, Class=Human Interface Device, Driver=, 12M

4. Now bind USB device with STM32Leds driver:

        echo 1-1.1.2:1.0 > /sys/bus/usb/derivers/STM32Leds/bind

5. Ensure that device is now binded with custom STM32Leds driver:

        pi@raspberrypi:~ $ lsusb -t
        /:  Bus 01.Port 1: Dev 1, Class=root_hub, Driver=dwc_otg/1p, 480M
        |__ Port 1: Dev 2, If 0, Class=Hub, Driver=hub/4p, 480M
            |__ Port 1: Dev 3, If 0, Class=Hub, Driver=hub/3p, 480M
                |__ Port 1: Dev 5, If 0, Class=Vendor Specific Class, Driver=lan78xx, 480M
                |__ Port 2: Dev 6, If 0, Class=Human Interface Device, Driver=STM32Leds, 12M   

6. Check if new device `/dev/stm32leds0` appeared.

#### Automation of device driver bindings using Udev

By this time we know how to associate device to particular driver manually. Sure it is useful to understand how to it is done. But what is more useful - not to spend time on routine tasks, but automate them instead.

Most modern Linux distrubutions including Raspbian use service called `udev` to automate management of various tasks related to devices, drivers and other maintainance routines.

`Udev` uses a term `rule` to define its behaviour. We will create custom rule which will do all job on binding for us as soon as we plug `stm32leds` in. We will also change a `user/group` assignment and mode for file `/dev/stm32leds` which repretents our device in devfs.

In Raspbian distribution there is a file exists at `/etc/udev/rules.d/` called `99-com.rules`. Good to know that `udev` applies rules from files in lexical order overwriting previous rules with new ones if they cross. That is why you may find various sets of files which are starting with different numbers in other distributions, depending of intended system behaviour. 

Connect STM32F4Discovery to one of Raspberry Pi USB ports. Perform manual binding from previous paragraph.  Now run folliwing command:

    sudo udevadm info --name=/dev/stm32leds0 --attribute-walk

This will show all the attributes for particular device `stm32leds0` and for all its chain of parent devices. List will be rather long. Here we are looking for unique attributes we can use in an `udev` rule to distinguish our device from other possible clients.

First comes device specified by devpath then its parents. Example output (some attributes ommited): 

    looking at device '/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.1/1-1.1.2/1-1.1.2:1.0/usbmisc/stm32leds0':
    KERNEL=="stm32leds0"
    SUBSYSTEM=="usbmisc"
    DRIVER==""

    looking at parent device '/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.1/1-1.1.2/1-1.1.2:1.0':
    KERNELS=="1-1.1.2:1.0"
    SUBSYSTEMS=="usb"
    DRIVERS=="STM32Leds"

    looking at parent device '/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.1/1-1.1.2':
    KERNELS=="1-1.1.2"
    SUBSYSTEMS=="usb"
    DRIVERS=="usb"
    ATTRS{removable}=="removable"
    ATTRS{idProduct}=="5620"
    ATTRS{idVendor}=="0477"

In case of USB devices obvious candidates to use in rule are VID:PID pair. `SUBSYSTEM=="usbmisc"` defined for our particular device also stands out. And `KERNELS=="1-1.1.2"` looks like our unique parent node.

Lets define rule in `/etc/udev/rules.d/99-com.rules`. Rememder to use `sudo` when opening and saving file.

    KERNELS=="1-1.1.2" SUBSYSTEM=="usbmisc", ATTRS{idVendor}=="0477", ATTRS{idProduct}=="5620" \
    PROGRAM="/bin/sh -c 'echo -n $id:1.0 >/sys/bus/usb/drivers/usbhid/unbind; \
    echo -n $id:1.0 >/sys/bus/usb/drivers/STM32Leds/bind;'"

Now we need to reapply changed rules. This can be done obly by `root`. 

    sudo -i
    udevadm control --reload-rules && udevadm trigger
    exit

Now reconnect USB device and run command:

    lsusb -t

Ensure `Driver=STM32Leds` instead `Driver=usbhid` appeared in our device decription. Run following command to find our device created in `/dev`.

    ls -lart /dev/ | grep stm

Now lets try to connect our device to other USB port of Raspberry Pi. Check `/dev` and `lsusb -t` again. Unfortunately nothing happened at this time and our device was binded to `usbhid` as happedned previously.

Run following command again:

    sudo udevadm info --name=/dev/stm32leds0 --attribute-walk

Looks like `SUBSYSTEM=="usbmisc"` and VID:PID pair remains the same, but our parent node changed to `KERNELS=="1-1.3"`, which does not satisfy rule we created and thus our device was not threatened properly. Lets delete parenting relation from rule.

As we are editing rules file again we will also add some other actions we are interested in. In particular changing `user/group` of our device file, so user space application can interact with device without need to be run under `sudo`. Final rule is the following:

    SUBSYSTEM=="usbmisc", ATTRS{idVendor}=="0477", ATTRS{idProduct}=="5620", OWNER="pi", GROUP="pi" \
    PROGRAM="/bin/sh -c 'echo -n $id:1.0 >/sys/bus/usb/drivers/usbhid/unbind;\
    echo -n $id:1.0 >/sys/bus/usb/drivers/STM32Leds/bind; \
    chown -R pi:pi /dev/stm32leds0 && chmod -R 770 /dev/stm32leds0;'"\

In addition to `bind/unbind` actions Udev will also execute `chown` to change `group/user` to `pi:pi` and `chmod` to change mode of file to `read/write`.

Reapply rules:

    sudo -i
    udevadm control --reload-rules && udevadm trigger
    exit

Reattach usb device and ensure everything work as expected. 

    >: ls -lart /dev/ | grep stm
    crw-rw----  1 pi   pi      180, 192 Nov 25 20:36 stm32leds0

Finally to leave it fully autonomous add `stm32leds_driver.ko` module to list of loaded at boot up, as desribed in `iio/Readme.md` of this repository.

**Using user space application**

Finally we can try to play with our device running user space application `us` that implements primitive functionality to interact with `stm32leds` type of device. It executes from command line and supports a set of following commands.

    -r - turn on red light LED5
    -g - turn on green light LED4
    -b - turn on blue light LED6
    -o - turn on orange light LED3
    -a - turn on all lights
    -f - torn off all lights
    -s - get button presses count
    -h - help

Example usage:

    ./us -rgb - red, greed, blue are on; orange is off