#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

static struct input_dev *um_dev;
static char *usb_buf;//
static int length;//长度
static dma_addr_t usb_buf_phys;
static struct urb *um_urb;//usb请求块

static void usb_mouse_irq(struct urb *urb)
{
	static unsigned char pre_val;

#if 0
	int i;
	static int cnt =0;
	printk("data  cnt %d   ",++cnt);
	for(i=0;i<length;i++)
	{
		printk("%02x  ",usb_buf[i]);
	}
	printk("\n");
#endif

	/*USB鼠标数据含义（联想 NM50）
	 *data【1】 bit0 左键 bit1 右键 bit2 中键
	 */
	if((pre_val &(1<<0)) != (usb_buf[1] &(1<<0)))
	{
		input_event(um_dev,EV_KEY,KEY_L, (usb_buf[1] &(1<<0)) ? 1:0);
		input_sync(um_dev);
	}

	if((pre_val &(1<<1)) != (usb_buf[1] &(1<<1)))
	{
		input_event(um_dev,EV_KEY,KEY_S, (usb_buf[1] &(1<<1)) ? 1:0);
		input_sync(um_dev);
	}

	if((pre_val &(1<<2)) != (usb_buf[1] &(1<<2)))
	{
		input_event(um_dev,EV_KEY,KEY_ENTER, (usb_buf[1] &(1<<2)) ? 1:0);
		input_sync(um_dev);
	}



	pre_val = usb_buf[1];

	/*重新提交urb*/
	usb_submit_urb(um_urb,GFP_KERNEL);
}
static int usb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	int pipe;//源

	printk("find usb mouse\n");
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;

	interface = intf->cur_altsetting;
	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;

	/*a分配一个input_dev结构体*/
	um_dev = input_allocate_device();
	/*b设置*/
	/*b.1能产生哪类事件*/
	set_bit(EV_KEY,um_dev->evbit);
	set_bit(EV_REP,um_dev->evbit);
	/*b.2能产生这类里面的哪些事件*/
	set_bit(KEY_L,um_dev->keybit);
	set_bit(KEY_S,um_dev->keybit);
	set_bit(KEY_ENTER,um_dev->keybit);
	/*c注册*/
	input_register_device(um_dev);
	/*d硬件相关的操作*/
	/*数据传输3 个要素:源，目的，长度*/
	/*源： USB设备的某个端点*/

	/*长度*/
	length = endpoint->wMaxPacketSize;

	pipe = usb_rcvintpipe(dev,endpoint->bEndpointAddress);
	/*目的*/
	usb_buf = usb_buffer_alloc(dev,length,GFP_ATOMIC,&usb_buf_phys);
	/*使用三要素*/
	/*分配一个urb*/
	um_urb = usb_alloc_urb(0,GFP_KERNEL);
	/*使用三要素设置 urb*/
	usb_fill_int_urb(um_urb, dev, pipe,usb_buf,length,
				 usb_mouse_irq, NULL, endpoint->bInterval);
	um_urb->transfer_dma = usb_buf_phys;
	um_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/*使用URB( 提交 urb)*/
	usb_submit_urb(um_urb,GFP_KERNEL);

	return 0;
}

static void usb_mouse_disconnect(struct usb_interface *intf)
{
	struct usb_device *dev = interface_to_usbdev(intf);

	printk("usb_disconnect\n");
	usb_kill_urb(um_urb);
	usb_free_urb(um_urb);
	usb_buffer_free(dev,length,usb_buf,usb_buf_phys);
	input_unregister_device(um_dev);
	input_free_device(um_dev);

}

static struct usb_device_id usb_mouse_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }	/* Terminating entry */
};

/*1、分配*/
static struct usb_driver usb_mouse_driver = {
	.name		= "usbmouse",
	.probe		= usb_mouse_probe,
	.disconnect	= usb_mouse_disconnect,
	.id_table	= usb_mouse_id_table,
};


static int usb_mouse_init(void)
{
	/*2、注册*/
	usb_register(&usb_mouse_driver);
	return 0;
}

static void usb_mouse_exit(void)
{
	usb_deregister(&usb_mouse_driver);
}

module_init(usb_mouse_init);
module_exit(usb_mouse_exit);
MODULE_LICENSE("GPL");

