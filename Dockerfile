FROM ubuntu:latest

# Allow the user to override or specify the version of the DTC compiler to use.
# Default to the latest one at the time of this image's creation (1.4.7).
ARG DTC_TAG=v1.4.7

# Environment variables that control the behavior of the scripts.
# Default location to read and write overlay source and binary files.
ENV OVERLAYS_DIR=/overlays
ENV DTC_EXTRA_ARGS=

# Install dependencies.
RUN apt-get update && \
    apt-get install -y build-essential flex bison git libssl-dev bc wget

# Compile and install the dtc compiler.
WORKDIR /opt
RUN git clone https://github.com/dgibson/dtc.git
WORKDIR /opt/dtc
RUN git checkout tags/$DTC_TAG -b release && \
    make && \
    cp dtc /usr/bin/

# Get Cross Compiler from offcial repo
WORKDIR /opt
RUN git clone --progress --verbose https://github.com/raspberrypi/tools.git --depth=1 pitools
WORKDIR /opt/pitools

# Export Cross Compile path to Docker environment variables
ENV TOOLCHAIN /opt/pitools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf
ENV PATH $TOOLCHAIN/bin:$PATH

# Build Raspberry Pi kernel to get working source tree for cross compilation
WORKDIR /home
RUN git clone --depth 1 --branch raspberrypi-kernel_1.20201022-1 https://github.com/raspberrypi/linux.git
WORKDIR /home/linux
RUN wget https://raw.githubusercontent.com/romanjoe/os-course-labs/master/kernel-config/.config && \
    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- oldconfig && \
    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- && \
    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules

WORKDIR /home/workdir

# Copy and compile any overlays into this container so they can later be loaded.
#WORKDIR /overlays
#COPY ./overlays/ /overlays/
#RUN compile_overlays /overlays

