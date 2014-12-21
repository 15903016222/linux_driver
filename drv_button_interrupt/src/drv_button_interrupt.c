#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/irqs.h>//这个在/opt/EmbedSky/linux-2.6.30.4/arch/arm/mach-s3c2410/include/mach 路径
#include <linux/interrupt.h>

MODULE_LICENSE("Dual BSD/GPL");

static struct class *buttondrv_class;
static struct class_devices *buttondrv_class_dev;

/*  */
static DECLARE_WAIT_QUEUE_HEAD(button_wait_q);
/*中断事件标志，中断服务程序将他置1，read函数将他置0*/
static volatile int ev_press =0;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;

static unsigned keyval;

struct pin_desc{
	unsigned int pin;
	unsigned int key_value;
};
/*按键按下时是：0x01 0x02 0x03 0x04*/
/*按键松开时是：0x81 0x82 0x83 0x84*/
struct pin_desc pins_desc[4] =
{
	{S3C2410_GPF1,0x01},
	{S3C2410_GPF4,0x02},
	{S3C2410_GPF2,0x03},
	{S3C2410_GPF0,0x04},
};
/*
 * 确定按键值
 */
static irqreturn_t buttons_irq(int irq,void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *) dev_id;
	unsigned int pinval;
	pinval = s3c2410_gpio_getpin(pindesc -> pin);
	if(pinval)//松开
	{
		keyval = 0x80|pindesc->key_value;
	}
	else
	{
		keyval = pindesc->key_value;
	}
	ev_press =1;//中断发生
	wake_up_interruptible(&button_wait_q);
	printk("button is pressed : %d \n",irq);
	return IRQ_HANDLED;
}
static int button_dev_open(struct inode *inode ,struct file* file)
{
	//配置按键的引脚 GPF0,1,2,4为输入引脚
	request_irq(IRQ_EINT1,buttons_irq, IRQ_TYPE_EDGE_BOTH,"key1",&pins_desc[0]);
	request_irq(IRQ_EINT4,buttons_irq, IRQ_TYPE_EDGE_BOTH,"key2",&pins_desc[1]);
	request_irq(IRQ_EINT2,buttons_irq, IRQ_TYPE_EDGE_BOTH,"key3",&pins_desc[2]);
	request_irq(IRQ_EINT0,buttons_irq, IRQ_TYPE_EDGE_BOTH,"key4",&pins_desc[3]);

	return 0;
}
ssize_t button_dev_read(struct file *file,char __user *buf,size_t size,loff_t *ppos)
{
	if(size !=1)
	{
		return -EINVAL;
	}
	/*如果没有按键动作发生  就休眠*/
	wait_event_interruptible(button_wait_q,ev_press);
	/*如果有按键动作发生，直接返回*/
	copy_to_user(buf,&keyval,1);
	ev_press = 0;
	return 0;
}
int button_dev_close(struct inode* inode ,struct file *file)
{
	free_irq(IRQ_EINT1,&pins_desc[0]);
	free_irq(IRQ_EINT4,&pins_desc[1]);
	free_irq(IRQ_EINT2,&pins_desc[2]);
	free_irq(IRQ_EINT0,&pins_desc[3]);

	return 0;
}
static struct file_operations button_sdv_fops =
{
	.owner 		= THIS_MODULE,
	.open  		= button_dev_open,
	.read 		= button_dev_read,
	.release 	= button_dev_close,
};
int major;
static int button_dev_init(void)//入口函数
{
	major = register_chrdev(0,"button_drv",&button_sdv_fops);

	buttondrv_class = class_create(THIS_MODULE,"button_drv");
	if(IS_ERR(buttondrv_class))
		return PTR_ERR(buttondrv_class);
	buttondrv_class_dev= device_create(buttondrv_class,NULL,MKDEV(major,0),NULL,"wq_button");
		if(unlikely(IS_ERR(buttondrv_class_dev)))
			return PTR_ERR(buttondrv_class_dev);

	/*映射物理地址*/
	gpfcon = (volatile unsigned long *) ioremap(0x56000050 ,16);
	gpfdat = gpfcon + 1;

	return 0;
}
static void button_dev_exit(void)
{
	unregister_chrdev(major,"button_drv");
	device_unregister(buttondrv_class_dev);
	class_destroy(buttondrv_class);

	iounmap(gpfcon);
}
module_init(button_dev_init);
module_exit(button_dev_exit);
