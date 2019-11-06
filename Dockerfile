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
    apt-get install -y build-essential flex bison git

# Compile and install the dtc compiler.
WORKDIR /opt
RUN git clone https://github.com/dgibson/dtc.git
WORKDIR /opt/dtc
RUN git checkout tags/$DTC_TAG -b release && \
    make && \
    cp dtc /usr/bin/

WORKDIR /opt
RUN git clone --progress --verbose https://github.com/raspberrypi/tools.git --depth=1 pitools
WORKDIR /opt/pitools
#RUN ln -s /opt/rpi-crosstools-x64/bin /usr/bin/arm-rpi-linux-gnueabihf \
ENV TOOLCHAIN /opt/pitools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf
ENV PATH $TOOLCHAIN/bin:$PATH
# Install scripts into usr/bin so the user can perform device tree commands.
#COPY ./scripts/ /usr/bin/

# Copy and compile any overlays into this container so they can later be loaded.
#WORKDIR /overlays
#COPY ./overlays/ /overlays/
#RUN compile_overlays /overlays

CMD arm-linux-gnueabihf-cpp --version && echo $(TOOLCHAIN)