CROSS := 1

PWD = $(shell pwd)
TARGET1 = hmc5843

ifneq ($(CROSS), 1)
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build
else
	KDIR = /home/linux

	export ARCH := arm
	export CROSS_COMPILE := arm-linux-gnueabihf-
endif

obj-m := $(TARGET1)_core.o $(TARGET1)_i2c.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@rm -f *.o *.cmd *.flags *.mod.c *.order
	@rm -f .*.*.cmd *~ *.*~ TODO.*
	@rm -fR .tmp*

disclean: clean
	@rm *.ko *.symvers
