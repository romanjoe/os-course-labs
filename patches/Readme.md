### Patch to fix wrong binary files in `scripts`

There is a bug in official release of Raspbian Buster Lite distribution, that affecs linux kernel module development.

When you execute `make` command to build modules in standard manner from your work directory, `make` is actually executing in `/libs/modules/4.19.85-v7+/build` and build process uses some kernel infrastructure for build.

When you first try to compile a module on Raspbian you get error.
    
    /bin/sh: 1: scripts/basic/fixdep: Exec format error

This is because of wrong platform binary provided in distribution as result of post cross compilation script.
All binaries should be for ARM plalform, instead they are build for x86_64 and included in kernel for ARM distribution.

Comparison of included and fixed `fixdep` binaries:

Was:

    file /libs/modules/4.19.85-v7+/build/scripts/basic/fixdep

Fixed:

    file /libs/modules/4.19.85-v7+/build/scripts/basic/fixdep

**Get and apply patch**

Ensure you have installed kernel headers and build tools:

    sudo apt-get install raspberrypi-kernel-headers
    sudo apt-get install -y build-essential bc bison flex libssl-dev

Apply patch for fix wrong `fixdep` bin format.

    cd /libs/modules/4.19.85-v7+/build

    sudo wget https://raw.githubusercontent.com/romanjoe/os-course-labs/master/patches/headers-debian-byteshift.patch -O - | sudo patch -p1

Recompile executables in `scripts` folder.

    make scripts