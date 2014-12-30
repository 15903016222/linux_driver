#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>


#include <plat/regs-adc.h>
#include <mach/regs-gpio.h>

static struct input_dev *s3c_ts_dev;

static struct timer_list ts_timer;


struct s3c_ts_regs{
	unsigned long ADCCON;
	unsigned long ADCTSC;
	unsigned long ADCDLY;
	unsigned long ADCDAT0;
	unsigned long ADCDAT1;
	unsigned long ADCUPDN;
};

static volatile struct s3c_ts_regs* s3c_ts_regs;

static void enter_wait_pen_down_mode(void)
{
	s3c_ts_regs->ADCTSC = 0xd3;
}
static void enter_wait_pen_up_mode(void)
{
	s3c_ts_regs->ADCTSC = 0x1d3;
}
static void enter_measure_xy_mode(void)
{
	/*ADCTSC
	 *bit[2]=1 1=Auto Sequential measurement of X-position, Y-position.
	 *bit[3]=1 1=XP Pull-up Disable.
	 */
	s3c_ts_regs->ADCTSC = (1<<3)|(1<<2);
}

static void start_ADC(void)
{
	/*ADCCON
	 * bit[0] = 1= A/D conversion starts and this bit is cleared after the start- up.
	 */
	s3c_ts_regs->ADCCON |= (1<<0);
}
static irqreturn_t pen_down_up_irq(int irq,void *dev_id)
{
	if(s3c_ts_regs->ADCDAT0 & (1<<15) )//松开了
	{
		printk("pen up\n");
		enter_wait_pen_down_mode();
	}
	else//按下了
	{
		enter_measure_xy_mode();
		start_ADC();
	}
	return IRQ_HANDLED;
}

/*过滤*/
static int soft_filter_ts(int x[],int y[])
{
#define ERR_LIMIT 10
	int avr_x,avr_y;
	int det_x,det_y;

	avr_x =(x[0] + x[1])/2;
	avr_y =(y[0] + y[1])/2;

	det_x = (x[2]>avr_x) ? (x[2] - avr_x):(avr_x - x[2]);
	det_y = (y[2]>avr_y) ? (y[2] - avr_y):(avr_y - y[2]);

	if((det_x > ERR_LIMIT)||(det_y > ERR_LIMIT))
		return 0;

	avr_x =(x[1] + x[2])/2;
	avr_y =(y[1] + y[2])/2;

	det_x = (x[3]>avr_x) ? (x[3] - avr_x):(avr_x - x[3]);
	det_y = (y[3]>avr_y) ? (y[3] - avr_y):(avr_y - y[3]);

	if((det_x > ERR_LIMIT)||(det_y > ERR_LIMIT))
		return 0;

	return 1;

}

static void s3c_ts_timer_function(unsigned long data)
{
	if(s3c_ts_regs->ADCDAT0 & (1<<15) ) //已经松开
	{
		input_report_abs(s3c_ts_dev,ABS_PRESSURE,0);
		input_report_key(s3c_ts_dev,BTN_TOUCH,0);
		input_sync(s3c_ts_dev);
		enter_wait_pen_down_mode();
	}
	else
	{
		/*测量x,y坐标*/
		enter_measure_xy_mode();
		start_ADC();
	}
}


static irqreturn_t adc_irq(int irq,void *dev_id)
{
	static int cnt = 0;
	static int x[4],y[4];
	/*优化2：
	 * 如果adc完成时，发现触摸笔已经松开，则丢弃此次结果
	 * */
	if(s3c_ts_regs->ADCDAT0 & (1<<15) )//松开了
	{
		cnt = 0;
		input_report_abs(s3c_ts_dev,ABS_PRESSURE,0);
		input_report_key(s3c_ts_dev,BTN_TOUCH,0);
		input_sync(s3c_ts_dev);
		enter_wait_pen_down_mode();
	}
	else
	{
		/*优化3：多次测量，求平均值
		 *
		 */
		++cnt;
		x[cnt] =s3c_ts_regs->ADCDAT0 & 0x3ff;
		y[cnt] =s3c_ts_regs->ADCDAT1 & 0x3ff;
		if(cnt == 4)
		{
			/*优化4
			 * 软件过滤
			 */
			if(soft_filter_ts)
			{
				int avaragex = (x[0]+x[1]+x[2]+x[3])/4;
				int avaragey = (y[0]+y[1]+y[2]+y[3])/4;
				//printk("adc_irq cnt = %d,x= %d,y= %d \n",cnt,avaragex,avaragey);
				input_report_abs(s3c_ts_dev,ABS_X,avaragex);
				input_report_abs(s3c_ts_dev,ABS_Y,avaragey);
				input_report_abs(s3c_ts_dev,ABS_PRESSURE,1);
				input_report_key(s3c_ts_dev,BTN_TOUCH,1);
				input_sync(s3c_ts_dev);
			}
			cnt = 0;
			enter_wait_pen_up_mode();
			/*启动定时器，处理常按 滑动的情况*/
			mod_timer(&ts_timer,jiffies +HZ/100);

		}
		else
		{
			enter_measure_xy_mode();
			start_ADC();
		}
	}
	return IRQ_HANDLED;
}

static int drv_ts_init(void)
{
	struct clk* clk;

	/*1、分配一个input ——dev结构体*/
	s3c_ts_dev = input_allocate_device();
	if (!s3c_ts_dev)
	{
		printk(KERN_ERR "Unable to allocate the input device !!\n");
		return -ENOMEM;
	}

	/*2、设置*/
	/*2、1能产生哪类事件*/
	set_bit(EV_KEY,s3c_ts_dev->evbit);
	set_bit(EV_ABS,s3c_ts_dev->evbit);
	/*2.2能够产生这类事件里面的哪些事件*/
	set_bit(BTN_TOUCH, s3c_ts_dev->keybit);

	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);


	/*3、注册*/
	input_register_device(s3c_ts_dev);

	/*4、硬件相关的操作*/
	/*4.1使能ADC时钟clkcon[15]*/
	clk = clk_get(NULL,"adc");
	if(!clk)
	{
		printk(KERN_ERR"falied to find adc clock source\n");
		return -ENOENT;
	}

	clk_enable(clk);

	/*设置s3c2440的ADC/ts寄存器*/
	s3c_ts_regs = ioremap(0x58000000,sizeof(struct s3c_ts_regs));
	/*ADCCON
	 *p_clock  50MHz
	 *bit[14]=1:A/D converter prescaler enable
	 *bit[13:6] =49 A/D converter prescaler value
	 *A/D converter freq. = 50MHz/(49+1) = 1MHz
	 *bit[0] A/D conversion starts by enable.先设为0
	 */
	s3c_ts_regs->ADCCON = (1<<14)|(49<<6);

	request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM,"ts_pen",NULL);

	request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM,"adc",NULL);

	/*优化1
	 *设置adc——delay为最大值
	 *电压稳定后，再出发 IRQ_TC中断。
	 */
	s3c_ts_regs->ADCDLY = 0xffff;

	/*优化5
	 *使用定时器处理常按，滑动的情况
	 */
	init_timer(&ts_timer);
	ts_timer.function = s3c_ts_timer_function;
	add_timer(&ts_timer);

	/*进入按键等待模式*/
	enter_wait_pen_down_mode();

	return 0;
}

static void drv_ts_exit(void)
{
	free_irq(IRQ_ADC,NULL);
	free_irq(IRQ_TC,NULL);
	iounmap(s3c_ts_regs);
	input_unregister_device(s3c_ts_dev);
	input_free_device(s3c_ts_dev);
	del_timer(&ts_timer);
}

module_init(drv_ts_init);
module_exit(drv_ts_exit);
MODULE_LICENSE("GPL");

