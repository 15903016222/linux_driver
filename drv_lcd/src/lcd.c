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
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <mach/fb.h>
#include <linux/gfp.h>


static int s3c2410fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info);

struct lcd_regs {
	unsigned long	lcdcon1;
	unsigned long	lcdcon2;
	unsigned long	lcdcon3;
	unsigned long	lcdcon4;
	unsigned long	lcdcon5;
    unsigned long	lcdsaddr1;
    unsigned long	lcdsaddr2;
    unsigned long	lcdsaddr3;
    unsigned long	redlut;
    unsigned long	greenlut;
    unsigned long	bluelut;
    unsigned long	reserved[9];
    unsigned long	dithmode;
    unsigned long	tpal;
    unsigned long	lcdintpnd;
    unsigned long	lcdsrcpnd;
    unsigned long	lcdintmsk;
    unsigned long	lpcsel;
};

static volatile struct lcd_regs *lcd_regsp;

static struct fb_ops s3cfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= s3c2410fb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static u32 pseudo_palette[16];
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;

static struct fb_info  *s3c_lcd;
/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int s3c2410fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val;
	if(regno>16)
		return 1;
	else
	{
		//用红绿蓝三原色
		val  = chan_to_field(red,   &info->var.red);
		val |= chan_to_field(green, &info->var.green);
		val |= chan_to_field(blue,  &info->var.blue);
	}
	pseudo_palette[regno] = val;
	return 0;

}
static int lcd_init(void)
{
	int ret;
	/*1、分配一个fb_info结构体*/
	s3c_lcd = framebuffer_alloc(0,NULL);
	if (!s3c_lcd)
			return -ENOMEM;
	/*2、设置*/
	/*2.1设置固定的参数：*/
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 480*272*32/8;
	s3c_lcd->fix.type 	  = FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual   = FB_VISUAL_TRUECOLOR;/*TFT*/
	s3c_lcd->fix.line_length =480*4;

	/*2.2设置可变的参数*/
	s3c_lcd->var.xres			= 480;//分辨率
	s3c_lcd->var.yres	  		= 272;
	s3c_lcd->var.xres_virtual 	= 480;//虚拟分辨率
	s3c_lcd->var.yres_virtual 	= 272;
	s3c_lcd->var.bits_per_pixel = 32;

	s3c_lcd->var.red.offset		= 16;
	s3c_lcd->var.red.length		= 8;

	s3c_lcd->var.green.offset		= 8;
	s3c_lcd->var.green.length		= 8;

	s3c_lcd->var.blue.offset		= 0;
	s3c_lcd->var.blue.length		= 8;

	//s3c_lcd->var.activate 		=FB_ACTIVATE_NOW;//
	/*2.3设置操作函数*/
	s3c_lcd->fbops 				= &s3cfb_ops;

	/*2.4其他的设置*/
	s3c_lcd->pseudo_palette 	= pseudo_palette;
	s3c_lcd->screen_size		= 480*272*32/8;
	/*3、硬件相关的操作*/
	/*3.1配置GPIO用于LCD*/
	gpbcon = ioremap(0x56000010,8);
	gpbdat = gpbcon+1;
	gpccon = ioremap(0x56000020,4);
	gpdcon = ioremap(0x56000030,4);
	gpgcon = ioremap(0x56000060,4);

	*gpccon = 0xaaaaaaaa;//
	*gpdcon = 0xaaaaaaaa;

	*gpgcon	|=(3<<8);//lcd power
	/*3.2根据LCD手册设置LCD控制器，频率等*/
	lcd_regsp = ioremap(0x4D000000,sizeof(struct lcd_regs));

	/*lcdcon1
	 * bit[17:8]   VCLK = VCLK = HCLK / [(CLKVAL+1) x 2]
	 *			  VCLK = 100MHz/[(CLKVAL+1) x 2]
	 * 				10M= 100MHz/[(CLKVAL+1) x 2]
	 * 				CLKVAL=4
	 * bit[6:5]:	0b11 : TFT
	 * bit[4:1]     BBPMODE  0b1101 = 24bbp
	 * bit[0]       0 = Disable the video output and the LCD control signal.
	 */
	lcd_regsp->lcdcon1 	= (4<<8)|(3<<5)|(0x0d<<1);

	/*lcdcon2  垂直方向的时间参数
	 * bit[31:24] VBPD ,VSYNC之后再过多久才能发出第一行数据
	 * 			LCD手册  tvb =2
	 * 			VBPD=1;
	 * bit[23:14] 272行   LINEVAL=272-1=271
	 * bit[13:6] VFPD发出最后一行数据之后再过多长时间发出 VSYNC
	 * 			LCD手册  tvf = 2
	 * 			VFPD =2-1=1
	 * bit[5:0]:VSPW VSYNC信号的脉冲宽度，LCD手册 tvp=10;所以 VSPW =10-1 =9
	 */
	lcd_regsp->lcdcon2 = (1<<24)|(271<<14)|(1<<6)|(9<<0);

	/*lcdcon3  水平方向的时间参数
	 * bit [25:19]HBPD,VSYNc之后过多长时间才能发出第一行数据
	 * 				LCD手册 thb =2
	 * 				HBPD =1
	 * bit[18:8] 480列， HPZVAL=480-1 =479
	 * bit[7:0]  HFPD 发出最后一行，最后一个数据之后，再过多长时间俺发出 HSYNC
	 * 				LCD手册 thf=2 所以 HFPD=2-1=1
	 */
	lcd_regsp->lcdcon3 =(1<<19)|(479<<8)|(1<<0);

	/*水平方向的同步信号
	 * bit[7:0] HSPW,HSYNC信号的脉冲宽度 LCD手册 THp=41,所以HSPW =41-1=40
	 */
	lcd_regsp->lcdcon4 = 40;


	/*信号的极性
	 * bit[11] 1=RGB565 format  24bpp不用设
	 * bit[10] 0 = The video data is fetched at VCLK falling edge
	 * bit[9]  1 = HSYNC 信号要反转
	 * bit[8]  1 = VSYNC 信号要反转
	 * bit[6]  0 = VDEN  不需要反转
	 * bit[3]  0 = PWREN 输出零
	 * bit[1]  0 = BSWP
	 * bit[0]  0 = HWSWP
	 */
	lcd_regsp->lcdcon5= (1<<9)|(1<<8);
	/*3.3分配framebuffer，并把地址告诉LCD控制器*/
	s3c_lcd->screen_base = dma_alloc_writecombine(NULL,s3c_lcd->fix.smem_len,&s3c_lcd->fix.smem_start,GFP_KERNEL);
	lcd_regsp->lcdsaddr1 = (s3c_lcd->fix.smem_start>>1) & ~(3<<30);
	lcd_regsp->lcdsaddr2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len)>>1) & 0x1fffff;
	lcd_regsp->lcdsaddr3 = (480 *32/16); //一行的长度（单位 ：2字节）

	/*启动lcd*/
	lcd_regsp->lcdcon1 |=(1<<0);//使能LCD控制器
	lcd_regsp->lcdcon5 |=(1<<3);//使能LCD本身


	/*4、注册*/
	ret = register_framebuffer(s3c_lcd);
		if (ret < 0) {
			printk(KERN_ERR "Failed to register framebuffer device: %d\n",ret);
		}
	return 0;
}

static void lcd_exit(void)
{
	unregister_framebuffer(s3c_lcd);
	lcd_regsp->lcdcon1 &=~(1<<0);//关闭LCD控制器
	lcd_regsp->lcdcon5 &=~(1<<3);//关闭LCD本身
	dma_free_writecombine(NULL,s3c_lcd->fix.smem_len,s3c_lcd->screen_base,s3c_lcd->fix.smem_start);

	iounmap(lcd_regsp);
	iounmap(gpbcon);
	iounmap(gpccon);
	iounmap(gpdcon);
	iounmap(gpgcon);

	framebuffer_release(s3c_lcd);
}


module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");
