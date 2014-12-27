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
	/*1������һ��fb_info�ṹ��*/
	s3c_lcd = framebuffer_alloc(0,NULL);
	if (!s3c_lcd)
			return -ENOMEM;
	/*2������*/
	/*2.1���ù̶��Ĳ�����*/

	/*2.2���ÿɱ�Ĳ���*/

	/*2.3���ò�������*/

	/*2.4����������*/

	/*3��Ӳ����صĲ���*/
	/*3.1����GPIO����LCD*/

	/*3.2����LCD�ֲ�����LCD��������Ƶ�ʵ�*/

	/*3.3����framebuffer�����ѵ�ַ����LCD������*/

	/*4��ע��*/
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

MODULE_LICENSE("GPL")��
