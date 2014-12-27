#include <linux/init.h>
#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>

/*����/����/ע��һ��platform_device*/
/*���ƽ̨�豸����Դ*/
static struct resource led_resource[] = {
    [0] = {
        .start	= 0x56000010,
        .end	= 0x56000010 + 8 - 1,
        .flags	= IORESOURCE_MEM,
    },
    [1] = {
        .start	= 6,
        .end	= 6,
        .flags	= IORESOURCE_IRQ,
    },


};

static void led_release(struct device *dev)
{

}

struct platform_device device_led = {
    .name		= "myled",
    .id			= -1,
    .num_resources	= ARRAY_SIZE(led_resource),
    .resource		= led_resource,
	.dev={
			.release = led_release,
    },
};

static int led_dev_init(void)
{
	platform_device_register(&device_led);
	return 0;
}

static void led_dev_exit(void)
{
	platform_device_unregister(&device_led);
}
module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
