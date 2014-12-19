/*相比与第一版的first_drv程序，这次升级主要包括：
 * 1、板子上的四个led可分别控制
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>

static struct class *firstdrv_class;
static struct class_devices *firstdrv_class_dev[4];

volatile unsigned long *gpbcon = NULL;
volatile unsigned long *gpbdat = NULL;

MODULE_LICENSE("Dual BSD/GPL");
static int first_dev_open(struct inode *inode,struct file *file)
{
	int minor = MINOR(inode ->i_rdev);//获得次设备号
	switch(minor)
	{
		case 0://GPB5 配置为输出
		{
			*gpbcon &=~(0x3<<(5*2));
			*gpbcon |=(0x1<<(5*2));
			break;
		}
		case 1://GPB6 配置为输出
		{
			*gpbcon &=~(0x3<<(6*2));
			*gpbcon |=(0x1<<(6*2));
			break;
		}
		case 2://GPB7 配置为输出
		{
			*gpbcon &=~(0x3<<(7*2));
			*gpbcon |=(0x1<<(7*2));
			break;
		}
		case 3://GPB8 配置为输出
		{
			*gpbcon &=~(0x3<<(8*2));
			*gpbcon |=(0x1<<(8*2));
			break;
		}

	}
	/*配置 GPB 5 6 7 8为输出*/
	//*gpbcon &=~((0x3<<(5*2))|(0x3<<(6*2))|(0x3<<(7*2))|(0x3<<(8*2)));//先清零
	//*gpbcon |=((0x1<<(5*2))|(0x1<<(6*2))|(0x1<<(7*2))|(0x1<<(8*2)));// 配置为1 （输出）
	//printk("first dev open\n");
	return 0;
}
static ssize_t first_dev_write(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
{
	int val;
	int minor = MINOR(file->f_dentry->d_inode->i_rdev);//获得次设备号
	copy_from_user(&val,buf,count);//从用户空间向内核空间拷贝数据
	switch(minor)
	{
		case 0://GPB5
		{
			if(val == 1)//亮
			{
				*gpbdat &=~(1<<5);
			}
			else//灭
			{
				*gpbdat|=(1<<5);
			}
			break;
		}
		case 1://GPB6
		{
			if(val == 1)//亮
			{
				*gpbdat &=~(1<<6);
			}
			else//灭
			{
				*gpbdat|=(1<<6);
			}
			break;
		}
		case 2://GPB7
		{
			if(val == 1)//亮
			{
				*gpbdat &=~(1<<7);
			}
			else//灭
			{
				*gpbdat|=(1<<7);
			}
			break;
		}
		case 3://GPB8
		{
			if(val == 1)//亮
			{
				*gpbdat &=~(1<<8);
			}
			else//灭
			{
				*gpbdat|=(1<<8);
			}
			break;
		}
	}


//	if(val == 1)
//	{
//		//点灯
//		*gpbdat &=~((1<<5)|(1<<6)|(1<<7)|(1<<8));
//	}
//	else
//	{
//		//灭灯
//		*gpbdat|=(1<<5)|(1<<6)|(1<<7)|(1<<8);
//	}
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
	int minor =0;
	major = register_chrdev(0,"first_drv",&first_sdv_fops);//注册,主设备号写零，表示内核自动分配主设备号，并作为函数返回值返回
	firstdrv_class = class_create(THIS_MODULE,"first_drv");
	if(IS_ERR(firstdrv_class))
		return PTR_ERR(firstdrv_class);
	for(minor=0;minor<4;minor++)
	{
		firstdrv_class_dev[minor] = device_create(firstdrv_class,NULL,MKDEV(major,minor),NULL,"wq_led%d",minor);
			if(unlikely(IS_ERR(firstdrv_class_dev[minor])))
				return PTR_ERR(firstdrv_class_dev[minor]);
	}


	/*映射物理地址*/
	gpbcon = (volatile unsigned long *) ioremap(0x56000010,16);
	gpbdat = gpbcon + 1;
	//printk("init   major= %d\n",major);
	return 0;
}

void first_dev_exit(void)
{
	//printk("exit\n");
	int minor;

	unregister_chrdev(major,"first_drv");//卸载
	for(minor=0;minor<4;minor++)
	{
		device_unregister(firstdrv_class_dev[minor]);
	}
	class_destroy(firstdrv_class);
	iounmap(gpbcon);
}
module_init(first_drv_init);
module_exit(first_dev_exit);
