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
		/*ȡ��ѡ��
		 * NFCONT��1��=0*/
	}
	else
	{
		/*ѡ��
		 * NFCONT��1��=1*/
	}
}

static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int data,unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{
		/*������
		 * NFCMMD = dat*/
	}
	else
	{
		/*������
		 * NFADDR = dat*/
	}
}

static int	s3c2440_dev_ready(struct mtd_info *mtd)
{
	/*NFSTAT
	 * */
	return "NFSTAT��bit��0��";
}

static int nand_init(void)
{
	/*1������һ�� nandchip�ṹ��*/
	s3c_nand = kzalloc(sizeof(struct nand_chip),GFP_KERNEL);
	/*2������*/
	/*����nand_chip �ṹ����Ϊ�˸�nand_scan�õ�  ͨ���˽�nand_scan���ʹ�ã��Ϳ����˽���ô���� nand_chip*/
	/*
	 * ��Ӧ���ṩ��ѡ�У����������ַ�������ݣ������ݣ��ж�״̬�Ĺ��ܡ�
	 */
	/*3��Ӳ����صĲ���*/

	/*4��ʹ��nand_scan*/
	mtd = kzalloc(sizeof(struct mtd_info),GFP_KERNEL);
	mtd->owner = THIS_MODULE;
	mtd->priv = s3c_nand;

	s3c_nand->select_chip = s3c2440_select_chip;
	s3c_nand->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand->IO_ADDR_R   ="NFDATA�������ַ";
	s3c_nand->IO_ADDR_W   ="NFDATA�������ַ";
	s3c_nand->dev_ready   =s3c2440_dev_ready;

	nand_scan(mtd,1);// ʶ�� NandFlash������mtd_info�ṹ��


	/*5��add_mtd_partitions*/

	return 0;
}
static void nand_exit(void)
{

}

module_init(nand_init);
module_exit(nand_exit);
MODULE_LICENSE("GPL");
