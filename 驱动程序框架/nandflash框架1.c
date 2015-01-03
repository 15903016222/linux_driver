#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>

#include <plat/regs-nand.h>
#include <plat/nand.h>

static struct nand_chip *s3c_nand;
static struct mtd_info *mtd;

static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{
		/*取消选中
		 * NFCONT【1】=0*/
	}
	else
	{
		/*选中
		 * NFCONT【1】=1*/
	}
}

static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int data,unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{
		/*发命令
		 * NFCMMD = dat*/
	}
	else
	{
		/*发数据
		 * NFADDR = dat*/
	}
}

static int	s3c2440_dev_ready(struct mtd_info *mtd)
{
	/*NFSTAT
	 * */
	return "NFSTAT的bit【0】";
}

static int nand_init(void)
{
	/*1、分配一个 nandchip结构体*/
	s3c_nand = kzalloc(sizeof(struct nand_chip),GFP_KERNEL);
	/*2、设置*/
	/*设置nand_chip 结构体是为了给nand_scan用的  通过了解nand_scan如何使用，就可以了解怎么设置 nand_chip*/
	/*
	 * 它应该提供：选中，发命令，发地址，发数据，读数据，判断状态的功能。
	 */
	/*3、硬件相关的操作*/

	/*4、使用nand_scan*/
	mtd = kzalloc(sizeof(struct mtd_info),GFP_KERNEL);
	mtd->owner = THIS_MODULE;
	mtd->priv = s3c_nand;

	s3c_nand->select_chip = s3c2440_select_chip;
	s3c_nand->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand->IO_ADDR_R   ="NFDATA的虚拟地址";
	s3c_nand->IO_ADDR_W   ="NFDATA的虚拟地址";
	s3c_nand->dev_ready   =s3c2440_dev_ready;

	nand_scan(mtd,1);// 识别 NandFlash，构造mtd_info结构体


	/*5、add_mtd_partitions*/

	return 0;
}
static void nand_exit(void)
{

}

module_init(nand_init);
module_exit(nand_exit);
MODULE_LICENSE("GPL");
