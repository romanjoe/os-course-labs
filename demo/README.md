## Linux kernel modules demo

This folder contains sample code for Linux kernel modules demonstrated on first lection of a short introductory course for second year students. 

Examples show basic concepts of Linux kernel modules organization, compilation and usage in runnig kernel, analysing results.

### Demonstrated commands and analysis of their output

**On board:**

Execute regular compilation on Raspberry Pi itself.

    make CROSS=0 - get error

    file fixdep - wrong
    ELF 64-bit LSB pie executable ARM aarch64, version 1 (SYSV), dynamically linked, interpreter /lib/ld-, for GNU/Linux 3.7.0, BuildID[sha1]=a1692535ab3b192a09bac27032606f64ec1815cb, not stripped

    file fixdep - recompiled
    ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0

    make CROSS=0 - success

Show info about type of compiled kernel object file.

    file md1.ko

Show symbols exported by module.

    cat Module.symverse

Show info about regular object file, kernel object file and relation to platform.

    modinfo m1.o
    modinfo md1.ko

    uname -a

Installation of module from build folder using `insmod`.

    insmod md2.ko - get error

Installation of md1.ko then dependent md2.ko using `insmod`.
    
    insmod md1.ko
    insmod md2.ko

Show system jounal records to find modules prints information.

    dmesg

Now try more reliable util `modprobe` from build forlder

    modprobe md1.ko - get error

Move kernel module object files to appropriate place and build dependencies list.

    cp md1.ko /lib/modules/4.19.85.v7+
    depmod -a
    modprobe md1.ko

**Cross compilation on host**

Execute full command to run compilation in Docker container with folder mount.

    docker run -v /Users/rnok/ucu/os-course-labs/demo:/home/workdir -it romanjoe/rpi-cc

Execute bash script to run compilation of modules inside a Docker container with appropriate environment.

    ./run make

Copy results to the board.

    scp -r demo pi@raspberrypi.local:/home/pi

For following modules installation and other commands see previous section.

### Kernel modules cheetsheet

**`depmod`** - handles dependency descriptions for loadable kernel modules
**`insmod`** - install loadable kernel module
**`modprobe`** - high level handling of loadable modules. `udev` relies upon modprobe to load drivers for automatically detected hardware
**`rmmod`** - unload specified loadable module
**`lsmod`** - show list of loaded modules

**`dmesg`** - used to read system journal located in `/var/log/messages`. Kernel `printk` issues messages here

**Useful debug tools**

`nm` from gcc toolchain - analyse names inside modules

    nm md1.ko | grep ' %module_name%'

`objdump` from gcc toolchain - displays various information about object file

    objdump -h %module_name%.ko 

`readelf` from gcc toolchain - tool to display info from elf files

    readelf -s %modules_name%.ko