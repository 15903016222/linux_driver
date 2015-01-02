#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

static struct gendisk *ramblock_disk;
static struct request_queue *ramblock_queue;

static DEFINE_SPINLOCK(ram_block_lock);//自旋锁

static int major;
#define RAMBLOCK_SIZE (1024*1024)
static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	/*容量 = heads*cylinders*sectors*512 */
	geo->heads     = 2;//面  随便定
	geo->sectors   = RAMBLOCK_SIZE/2/32/512;//扇区
	geo->cylinders =32;//环  随便定
	return 0;
}


static struct block_device_operations ram_block_fops = {
	.owner	= THIS_MODULE,
	.getgeo = ramblock_getgeo,//柱面数
};


static unsigned char *ramblock_buf;
static void do_ram_block_request (struct request_queue * q)
{
	static int r_cnt = 0;
	static int w_cnt = 0;
	struct request *req;

	while ((req = elv_next_request(q)) != NULL)
	{
		/*数据传输三要素： 源，长度，目的*/
		/*源/目的：*/
		unsigned long offset = req->sector<<9;//*512  这个扇区

		/*目的/源：*/
		//req->buffer
		/*长度*/
		unsigned len    = req->current_nr_sectors<<9;//*512

		if(rq_data_dir(req) == READ)
		{
//			printk("read  %d \n",++r_cnt);
			memcpy(req->buffer,ramblock_buf+offset,len);
		}
		if(rq_data_dir(req) == WRITE)
		{
//			printk("write  %d \n",++w_cnt);
			memcpy(ramblock_buf+offset,req->buffer,len);
		}

		end_request(req, 1);	/* wrap up, 0 = fail, 1 = success */
	}


}


static int ram_block_init(void)
{
	/*1.分配一个 gendisk结构体*/
	ramblock_disk = alloc_disk(16);//参数为次设备号个数（分区个数+1）
	/*2.设置*/
	/*2.1分配/设置一个队列：提供读写能力*/
	ramblock_queue = blk_init_queue(do_ram_block_request, &ram_block_lock);

	ramblock_disk->queue = ramblock_queue;
	/*2.2设置其他属性：比如容量*/
	major = register_blkdev(0,"ramblock"); /*cat /proc/devices */

	ramblock_disk->major = major;
	ramblock_disk->first_minor = 0;
	sprintf(ramblock_disk->disk_name, "ramblock");
	ramblock_disk->fops = &ram_block_fops;
	set_capacity(ramblock_disk,RAMBLOCK_SIZE/512);

	/*3.硬件相关的操作*/
	ramblock_buf = kzalloc(RAMBLOCK_SIZE,GFP_KERNEL);
	if (!ramblock_buf)
			return -ENOMEM;

	/*4.注册*/
	add_disk(ramblock_disk);

	return 0;
}

static int ram_block_exit(void)
{
	unregister_blkdev(major,"ramblock");
	del_gendisk(ramblock_disk);
	put_disk(ramblock_disk);
	blk_cleanup_queue(ramblock_queue);

	kfree(ramblock_buf);
}

module_init(ram_block_init);
module_exit(ram_block_exit);

MODULE_LICENSE("GPL");
