### Set up Docker

*NOTE: Everything you do inside container stays in CONTAINER, not an IMAGE. Changes have to be applied to Dockerfile to persist*

**On Mac via homebrew**

https://medium.com/@yutafujii_59175/a-complete-one-by-one-guide-to-install-docker-on-your-mac-os-using-homebrew-e818eb4cfc3

**Commands to execute before start**

Start docker-daemon process:

    docker-machine start default
    docker-machine env
    eval $(docker-machine env)

Register on Docker Hub repository site to get images.

Login to Docker Hub account before executing `docker` command.

    docker login

Get Docker image with Raspberry Pi cross compilation environment.

    docker image pull romanjoe/rpi-cc:latest

This may take some time, as image is aroung 3GB.

#### rpi-cc Docker image contents

`Ubuntu` - base image
`arm-linux-gnueabihf-` - toolchain for cross compilation from official raspberry pi tools repository
`build utils` - set of packages needed to build kernel
`linux kernel` - source code for linux kernel version 4.19.y used on raspberry pi board
`compiled kernel` - raspberry pi kernel to support cross compilation of modules

### Getting Raspberry Pi ready

#### Hardware required
1. Raspberry Pi model B+
2. Micro SD card 8gb or more
3. Power source capable to provide 2A current
4. USB A to USB micro B cable
5. Externally connected LED or other indication device.

#### Software

1. Download **Raspbian OS Buster Lite** from [official site](https://www.raspberrypi.org/downloads/raspbian)
2. Download **Balena Etcher** software from [Balena](https://www.balena.io/etcher)
3. Use Balena Etcher to flash Raspbian Buster Lite to SD card.

Do not eject SD card yet.

#### Set up wireless connection

## MORE INFO NEEDED HERE

1. Enble `ssh` service on Raspberry Pi
    On Linux and Mac
   
        touch /Volumes/boot/ssh

    On Windows

        Create file in Notepad with name ssh, push space key to make changes to is and save file to /boot folder without any extension.

2. Go to `boot` folder of SD card and create file called `wpa_supplicant.conf` with following content.

        country=US
        ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
        update_config=1

        network={
            ssid="NETWORK-NAME"
            psk="NETWORK-PASSWORD"
        }
Replace NETWORK-NAME and NETWORK-PASSWORD with your local WI-FI network credentials. If your network requires sophisticated authentification method to login - the easiest way to overcome is to use you phone in `Hotspot` mode.

3. Eject SD card from PC and install to Raspberry Pi, wait around 15-20 sec
4. Switch your laptop to same WI-FI network you put into `wpa_supplicant.conf`
5. Establish `ssh` connections to your board.
    
    On Linux and Mac:

        ssh-keygen -R raspberrypi.local
        ssh pi@raspberrypi.local
    
    Here we create new file `raspberrypi.local` of knows hosts type. And then logging in to it with user name `pi`. On **password** request type `raspberry` - this is defalt password of `pi` user.

## ADD INFO for Windows

#### Get updates and linux kernel headers

1. Suppose you are at raspberry pi terminal.
   
        sudo apt-get update -y
        sudo apt-get upgrade -y

2. Install linux kernel headers to Raspberry Pi

        sudo apt-get install raspberrypi-kernel-headers
        sudo apt-get install -y build-essential bc bison flex libssl-dev

    Check folder /libs/modules/4.19.85-v7+/build - it should contain linux kernel folders.

3. Apply patch for fix wrong `fixdep` bin format.

        cd /libs/modules/4.19.85-v7+/build

        sudo wget https://raw.githubusercontent.com/romanjoe/os-course-labs/master/patches/headers-debian-byteshift.patch -O - | sudo patch -p1

4. Recompile executables in `scripts` folder.

        make scripts

5. Copy source code of examples from this repository `demo`, `gpio` to Raspberry Pi.

        scp -r demo pi@raspberry.local:/home/pi/native

    `/home/pi/native` folder will be used for compilation of board.
    `/home/pi/cross` folder will be used to host cross compiled objects.

6. Compile `demo` content.

        cd /home/pi/native/demo

        make CROSS=0

7. if you see no errors on compilation and new files appeared in folder - you are you done with setup successfully.