
/*分配/设置/注册一个platform_driver*/

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>
#include <asm/io.h>

static int major;

static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;

static struct class *led_cls;//设备类
static struct class_devices *led_class_dev;//设备

static int pin;

static int led_open(struct inode *inode,struct file *file)
{
	//配置为输出引脚
	*gpio_con &=~(0x3<<(pin*2));
	*gpio_con |=(0x1<<(pin*2));

	return 0;
}

static ssize_t led_write(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
{
	int val;
	copy_from_user(&val,buf,count);//从用户空间向内核空间拷贝数据
	if(val == 1)
	{
		printk("val ==1");
		*gpio_dat &=~(1<<pin);
	}
	else
	{
		printk("val ==0");
		*gpio_dat|=(1<<pin);
	}

	return 0;
}
static struct file_operations led_fops=
{
		.owner = THIS_MODULE,//这个宏在推向编译模块时自动创建  __this_module变量
		.open  = led_open,
		.write = led_write,
};



static int  led_probe(struct platform_device *pdev)
{
	struct resource *res;
	/*根据platform_device的资源进行ioremap*/
	res = platform_get_resource(pdev,IORESOURCE_MEM,0);
	gpio_con = ioremap(res->start,res->end - res->start + 1);
	gpio_dat = gpio_con + 1;

	res = platform_get_resource(pdev,IORESOURCE_IRQ,0);
	pin = res->start;

	/*注册字符设备驱动*/
	printk("led_probe   found  led \n");
	major = register_chrdev(0,"myled",&led_fops);

	led_cls = class_create(THIS_MODULE,"myled");
	if(IS_ERR(led_cls))
		return PTR_ERR(led_cls);
	led_class_dev = device_create(led_cls,NULL,MKDEV(major,0),NULL,"wq_led");
	if(unlikely(IS_ERR(led_class_dev)))
		return PTR_ERR(led_class_dev);

	return 0;
}

static int  led_remove(struct platform_device *pdev)
{
	/*卸载字符设备驱动*/
	device_unregister(led_class_dev);
	class_destroy(led_cls);
	unregister_chrdev(major,"myled");
	iounmap(gpio_con);
	/*根据platform_device的资源进行iounmap*/
	return 0;
}

static struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",
	}
};

static int led_drv_init(void)
{
	platform_driver_register(&led_drv);
	return 0;
}

static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");
