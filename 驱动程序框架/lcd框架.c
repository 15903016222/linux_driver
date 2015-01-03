#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <mach/fb.h>

static struct fb_info *s3c_lcd;


static int lcd_init()
{
	int ret;
	/*1、分配一个fb_info结构体*/
	s3c_lcd = framebuffer_alloc(0,NULL);
	if (!s3c_lcd)
			return -ENOMEM;
	/*2、设置*/
	/*2.1设置固定的参数：*/

	/*2.2设置可变的参数*/

	/*2.3设置操作函数*/

	/*2.4其他的设置*/

	/*3、硬件相关的操作*/
	/*3.1配置GPIO用于LCD*/

	/*3.2根据LCD手册设置LCD控制器，频率等*/

	/*3.3分配framebuffer，并把地址告诉LCD控制器*/

	/*4、注册*/
	ret = register_framebuffer(s3c_lcd);
		if (ret < 0) {
			printk(KERN_ERR "Failed to register framebuffer device: %d\n",
				ret);
	return 0;
}

static void lcd_exit()
{

}


module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL")；
