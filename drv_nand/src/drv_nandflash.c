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



struct s3c_nand_regs
{
	 unsigned long nfconf  ;
	 unsigned long nfcont  ;
	 unsigned long nfcmd   ;
	 unsigned long nfaddr  ;
	 unsigned long nfdata  ;
	 unsigned long nfeccd0 ;
	 unsigned long nfeccd1 ;
	 unsigned long nfeccd  ;
	 unsigned long nfstat  ;
	 unsigned long nfestat0;
	 unsigned long nfestat1;
	 unsigned long nfmecc0 ;
	 unsigned long nfmecc1 ;
	 unsigned long nfsecc  ;
	 unsigned long nfsblk  ;
	 unsigned long nfeblk  ;
};

static struct nand_chip *s3c_nand;
static struct mtd_info *mtd;
static struct s3c_nand_regs *s3c_nand_regs;

static struct mtd_partition s3c_nand_parts[] = {
[0] = {
	.name	= "wangqi_Board_uboot",
	.offset	= 0x00000000,
	.size	= 0x00040000,
},
[1] = {
	.name	= "wangqi_Board_kernel",
	.offset	= 0x00200000,
	.size	= 0x00200000,
},
[2] = {
	.name	= "wangqi_Board_yaffs2",
	.offset	= 0x00400000,
	.size	= 0x0FB80000,
}
};

static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{
		/*取消选中
		 * NFCONT【1】=1*/
		s3c_nand_regs->nfcont |= (1<<1) ;
	}
	else
	{
		/*选中
		 * NFCONT【1】=0*/
		s3c_nand_regs->nfcont &= ~(1<<1);
	}
}

static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int data,unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{
		/*发命令
		 * NFCMMD = dat*/
		s3c_nand_regs->nfcmd = data;
	}
	else
	{
		/*发数据
		 * NFADDR = dat*/
		s3c_nand_regs->nfaddr = data;
	}
}

static int	s3c2440_dev_ready(struct mtd_info *mtd)
{
	/*NFSTAT[0]
	 * */
	return s3c_nand_regs->nfstat & (1<<0);
}

static int nand_init(void)
{
	struct clk *clk;

	/*1、分配一个 nandchip结构体*/
	s3c_nand = kzalloc(sizeof(struct nand_chip),GFP_KERNEL);
	s3c_nand_regs = ioremap(0x4E000000,sizeof(struct s3c_nand_regs));
	/*2、设置*/
	/*设置nand_chip 结构体是为了给nand_scan用的  通过了解nand_scan如何使用，就可以了解怎么设置 nand_chip*/
	/*
	 * 它应该提供：选中，发命令，发地址，发数据，读数据，判断状态的功能。
	 */


	/*3、硬件相关的操作  根据nandflash的datasheet 设置时间参数*/
	/*使能nandflash控制器的时钟*/
	clk = clk_get(NULL,"nand");
	clk_enable(clk);   //clkcon的bit4

	/*HCLOK = 100MHz   10ns
	 * TACLS:发出cle/ale之后多长时间才发出 nWE信号，从NAND手册可知cle/ale与nWE可以同时发出，所以TACLS值可以为0
	 * TWRPH0 nWE信号的脉冲宽度  从手册上： HCLK *（TWRPH0 + 1） >=12ns,所以 TWRPH0 >= 1
	 * TWRPH1 nWE变为高电平后，CLE/ALE多长时间才能变为低电平，nand手册可知 它要>=5ns  所以TWRPH1>=0
	 * */
#define  TACLS    0
#define  TWRPH0   1
#define  TWRPH1   0
	s3c_nand_regs->nfconf = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);

	/*NFCONT:
	 * BIT1 设为1，取消片选
	 * BIT0 设为1，使能NAND_FLASH控制器
	 */
	s3c_nand_regs->nfcont = (1<<1) |(1<<0);


	/*4、使用nand_scan*/
	mtd = kzalloc(sizeof(struct mtd_info),GFP_KERNEL);
	mtd->owner = THIS_MODULE;
	mtd->priv = s3c_nand;

	s3c_nand->select_chip = s3c2440_select_chip;
	s3c_nand->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand->IO_ADDR_R   = &s3c_nand_regs->nfdata;
	s3c_nand->IO_ADDR_W   = &s3c_nand_regs->nfdata;
	s3c_nand->dev_ready   =s3c2440_dev_ready;
	s3c_nand->ecc.mode    = NAND_ECC_SOFT;//ECC校验

	nand_scan(mtd,1);// 识别 NandFlash，构造mtd_info结构体


	/*5、add_mtd_partitions*/
	add_mtd_partitions(mtd,s3c_nand_parts,3);

	//add_mtd_device(mtd);//这样可以将整个nandflash构造成一个分区，而不用其他的配置

	return 0;
}
static void nand_exit(void)
{
	del_mtd_partitions(mtd);
	kfree(mtd);
	kfree(s3c_nand);
	iounmap(s3c_nand_regs);
}

module_init(nand_init);
module_exit(nand_exit);
MODULE_LICENSE("GPL");
