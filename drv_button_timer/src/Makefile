ARCH=arm 
KERNELDIR=/opt/EmbedSky/linux-2.6.30.4   
PWD:=$(shell pwd)   
INSTALLDIR=$(PWD)    
CC=arm-linux-gcc
obj-m := drv_button_timer.o   
all:   
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules   
clean:
	rm -rf *.o *.ko *.mod.c *.markers *.order *.symvers     
.PHONY:modules clean       
