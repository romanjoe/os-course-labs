### Device Tree and IIO subsystem exploration

**Helper tools to use with I2C devices**

In this folder we will work with various I2C sensors connected to Raspberry Pi. In order to ensure that device is connected successfully and perform some other actions python package `i2c-tools` can be used.

    sudo apt-get install i2c-tools -y

Connect some I2C sensor to Raspberry Pi and run command:

    i2cdetect -y 1

Example output for GY-80 breakout module which contains 3 I2C sensors on board.

    EXAMPLE OUTPUT HERE

#### Device tree overlays

This folder contains couple `.dts` files, which describe corresponding hardware sensors. This files use device tree language and intended to be compiled with Device Tree compiler in order to get binary blob that later will be installed to the board.

**Compiling device tree**

Special tool called Device Tree compiler is used for that. Provided Docker image `romanjoe/rpi-cc` alredy has this compiler. Thus following command can be issued:

    ./run dtc -@ -I dts -O dtb -o bmp085.dtbo bmp085.dts

Which extends to:

    docker run -v $PWD:/home/workdir -it romanjoe/rpi-cc dtc -@ -I dts -O dtb -o bmp085.dtbo bmp085.dts

Corresponding `.dtbo` files are compiled binaries that are **OS independent** and can be used with mutliple systems.

**Applying device tree overlays**

Keep in mind that device tree overlays support implemented differently on different boards. In our case for Raspberry Pi running Raspbian Buster Lite next scheme is used. `boot` folder of Raspberry Pi contain configuration file `config.txt` which contains information used by system at boot up time. 

In order to load custom device tree overlay `config.txt` should be populated with next string:

    dtoverlay=bmp085

And corresponding `.dtbo` file should be placed in `/boot/overlays`

In case of `bmp085` example kernel used in Raspbian Buster shipped with compiled driver for this sensor. Thus only device tree entry needed to make it work.

**Getting custom module driver run**

Raspbian Buster kernel does not include driver for `vl530lx-i2c` proximity sensor. To make it work it is needed to have a device tree overlay and corresponding kernel module driver loaded at boot time.

Again impplementation of modules loading may vary between boards and operating systems. In case of Raspberry Pi running Raspbian Buster there is a file `/etc/module-run.d/config`.

In order to load custom module `config` should be populated with next string:

    vl530lx-i2c

Corresponding kernel module object file(s) `.ko` along with `Module.symvers` and `modules.order` should be placed to `/lib/modules/'uname -r'/modules`.

Run following commands to update modules dependencies and to ensure module loaded successully:

    sudo depmod
    sudo modprobe module_name

Driver will be successully loaded after reset as well.


**Explore installed IIO device in running system**

Successfully installed and running IIO devices are found in `/sys/bus/iio/device/` and has a set of standard attributes, defined by IIO interface and dependent on implementation in driver.

Typical attributes would be for `bmp085`:

    -r--r--r-- 1 root root 4096 Nov 25 23:59 name
    -r--r--r-- 1 root root 4096 Nov 25 23:59 in_temp_oversampling_ratio_available
    -rw-r--r-- 1 root root 4096 Nov 25 23:59 in_temp_oversampling_ratio
    -rw-r--r-- 1 root root 4096 Nov 25 23:59 in_temp_input
    -r--r--r-- 1 root root 4096 Nov 25 23:59 in_pressure_oversampling_ratio_available
    -rw-r--r-- 1 root root 4096 Nov 25 23:59 in_pressure_oversampling_ratio
    -rw-r--r-- 1 root root 4096 Nov 25 23:59 in_pressure_input
    -r--r--r-- 1 root root 4096 Nov 25 23:59 dev