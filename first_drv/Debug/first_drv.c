#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>

static struct class *firstdrv_class;
static struct class_devices *firstdrv_class_dev;

volatile unsigned long *gpbcon = NULL;
volatile unsigned long *gpbdat = NULL;

MODULE_LICENSE("Dual BSD/GPL");
static int first_dev_open(struct inode *inode,struct file *file)
{
	/*配置 GPB 5 6 7 8为输出*/
	*gpbcon &=~((0x3<<(5*2))|(0x3<<(6*2))|(0x3<<(7*2))|(0x3<<(8*2)));//先清零
	*gpbcon |=((0x1<<(5*2))|(0x1<<(6*2))|(0x1<<(7*2))|(0x1<<(8*2)));// 配置为1 （输出）
	//printk("first dev open\n");
	return 0;
}
static ssize_t first_dev_write(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
{
	int val;
	copy_from_user(&val,buf,count);//从用户空间向内核空间拷贝数据
	if(val == 1)
	{
		//点灯
		*gpbdat &=~((1<<5)|(1<<6)|(1<<7)|(1<<8));
	}
	else
	{
		//灭灯
		*gpbdat|=(1<<5)|(1<<6)|(1<<7)|(1<<8);
	}

	//printk("first dev write\n");
	return 0;
}
static struct file_operations first_sdv_fops =
{
		.owner = THIS_MODULE,
		.open  = first_dev_open,
		.write = first_dev_write,
};
int major;
int first_drv_init(void)
{

	major = register_chrdev(0,"first_drv",&first_sdv_fops);//注册
	firstdrv_class = class_create(THIS_MODULE,"first_drv");
	if(IS_ERR(firstdrv_class))
		return PTR_ERR(firstdrv_class);
	firstdrv_class_dev = device_create(firstdrv_class,NULL,MKDEV(major,0),NULL,"wq_device");
	if(unlikely(IS_ERR(firstdrv_class_dev)))
		return PTR_ERR(firstdrv_class_dev);

	/*映射物理地址*/
	gpbcon = (volatile unsigned long *) ioremap(0x56000010,16);
	gpbdat = gpbcon + 1;
	//printk("init   major= %d\n",major);
	return 0;
}

void first_dev_exit(void)
{
	//printk("exit\n");
	unregister_chrdev(major,"first_drv");//卸载

	device_unregister(firstdrv_class_dev);
	class_destroy(firstdrv_class);
	iounmap(gpbcon);
}
module_init(first_drv_init);
module_exit(first_dev_exit);
