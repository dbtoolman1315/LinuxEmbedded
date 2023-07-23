#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/input.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define KEYNUM	2
#define KEYNAME "keyinput"


struct irq_keydes
{
	int gpio;
	int irqnum;
	unsigned char val;
	char name[10];
	irqreturn_t (*handler)(int,void*);
};

struct keyinput_dev
{
	int major;
	struct class *keyinput_class;
	struct timer_list timer;
	struct device_node *nd;
	struct irq_keydes irqkeydes[KEYNUM];
	unsigned char curkeynum;
	struct input_dev *inputdev;
}ST_KEYINPUT_DEV;

static ST_KEYINPUT_DEV st_keyinput_dev;

static int keyinput_init(void)
{
	int i = 0;
	int ret;
	char name[10];
	st_keyinput_dev.nd = of_find_node_by_path("/myself_gpio");
	if(st_keyinput_dev.nd == NULL)
	{
		printk("no such devtree node\n");
		return -EINVAL;
	}

	for(i = 0; i < KEYNUM; i++)
	{
		st_keyinput_dev.irqkeydes[i].gpio = of_get_named_gpio(st_keyinput_dev.nd,"gpios", i);
		if(st_keyinput_dev.irqkeydes[i].gpio < 0)
		{
			printk("can not find gpio %d\n",i);
		}
	}

	for(i = 0; i < KEYNUM; i++)
	{
		memset(st_keyinput_dev.irqkeydes[i].name,0,sizeof(name));
		sprintf(st_keyinput_dev.irqkeydes[i].name,"KEY%d",i);
		gpio_request(st_keyinput_dev.irqkeydes[i].gpio, st_keyinput_dev.irqkeydes[i].name);
	}
	
}

