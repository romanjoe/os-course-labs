### Patch to fix wrong binary files in `scripts`

There is a bug in official release of Raspbian Buster Lite distribution, that affecs linux kernel module development.

When you execute `make` command to build modules in standard manner from your work directory, `make` is actually executing in `/lib/modules/4.19.75-v7+/build` and build process uses some kernel infrastructure for build.

When you first try to compile a module on Raspbian you get error.
    
    /bin/sh: 1: scripts/basic/fixdep: Exec format error

This is because of wrong platform binary provided in distribution as result of post cross compilation script.
All binaries should be for ARM 32-bit plalform, instead they are build for 64 bit.

Comparison of included and fixed `fixdep` binaries:

Was:

    file /lib/modules/4.19.75-v7+/build/scripts/basic/fixdep
    fixdep: ELF 64-bit LSB pie executable ARM aarch64, version 1 (SYSV), dynamically linked, interpreter /lib/ld-, for GNU/Linux 3.7.0,
    
Fixed:

    file /lib/modules/4.19.75-v7+/build/scripts/basic/fixdep
    fixdep: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0,

**Get and apply patch**

Ensure you have installed kernel headers and build tools:

    sudo apt-get install raspberrypi-kernel-headers
    sudo apt-get install -y build-essential bc bison flex libssl-dev

Apply patch for fix wrong `fixdep` bin format.

    cd /lib/modules/4.19.75-v7+/build

    sudo wget https://raw.githubusercontent.com/romanjoe/os-course-labs/master/patches/headers-debian-byteshift.patch -O - | sudo patch -p1

Recompile executables in `scripts` folder.

    make scripts