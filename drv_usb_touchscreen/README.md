对于较高版本的linux版本，直接通过两种方式：
1. 通过make menuconfig进行手动配置
make menuconfig
Device Drivers --->

HID support --->

Special HID drivers --->
[] HID Multitouch panels
将 [ ] HID Multitouch panels 更改为 [] HID Multitouch panels
表示将该选了该功能

    直接修改配置文件 .config vim .config 将 # CONFIG_HID_MULTITOUCH is not set 更改为 CONFIG_HID_MULTITOUCH=y

	通过以上两种方法，都可以使得linux内核支持AMT-P3007电容触摸屏

	对于版本较低的linux内核，内核没有支持AMT-P3007电容触摸屏的驱动，必须增加驱动配置文件
	hid-penmount.c 通过 Makefile 将 hid-penmount.c 编译成驱动模块hid-penmount.ko, 直接插
	内核，另外内核的linux/include/linux/hid.h中的 IS_INPUT_APPLICATION 按照下边更改宏定
	义:

#define IS_INPUT_APPLICATION(a) (((a >= 0x00010000) && (a <= 0x00010008)) || (a == 0x00010080) || (a == 0x000c0001) || ((a >= 0x000d0002) && (a <= 0x000d0006)))

	即可

	使用修改过的内核，和编译得到的 hid-penmount.ko 即可将 AMT-P3007 电容触摸屏在低版本的
	内核支持。 
