#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
MODULE_LICENSE("Dual BSD/GPL");

static struct class *buttondrv_class;
static struct class_devices *buttondrv_class_dev;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;

static int button_dev_open(struct inode *inode ,struct file* file)
{
	//配置按键的引脚 GPF0,1,2,4为输入引脚
	*gpfcon &= ~((0x3<<0*2)|(0x3<<1*2)|(0x3<<2*2)|(0x3<<4*2));

	return 0;
}
ssize_t button_dev_read(struct file *file,char __user *buf,size_t size,loff_t *ppos)
{
	/*返回四个引脚的电平*/
	unsigned char key_vals[4];
	int regval;
	if(size !=sizeof(key_vals))
	{
		return -EINVAL;
	}
	regval = *gpfdat;
	/*读四个引脚的电平*/
	key_vals[0] = (regval &(1<<0))? 1 : 0;
	key_vals[1] = (regval &(1<<1))? 1 : 0;
	key_vals[2] = (regval &(1<<2))? 1 : 0;
	key_vals[3] = (regval &(1<<4))? 1 : 0;
	copy_to_user(buf,key_vals,sizeof(key_vals));
	return sizeof(key_vals);
}
static struct file_operations button_sdv_fops =
{
	.owner = THIS_MODULE,
	.open  = button_dev_open,
	.read = button_dev_read,
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
