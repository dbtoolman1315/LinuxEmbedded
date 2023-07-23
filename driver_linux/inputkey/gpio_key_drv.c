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
#include <linux/platform_device.h>


struct gpio_key{
	int gpio;
	struct gpio_desc *gpiod;
	int flag;
	int irq;
	int cnt;
	struct timer_list keytimer;
    struct input_dev *inputkey;
} ;
static unsigned char key_n[2] = {KEY_0,KEY_1};
static struct gpio_key *gpio_keys_100ask;

/* 主设备号                                                                 */
static int major = 0;
static struct class *gpio_key_class;

static int g_key = 0;



static irqreturn_t gpio_key_isr(int irq, void *dev_id)
{
	struct gpio_key *gpio_key = (struct gpio_key *)dev_id;
	gpio_key->keytimer.data = dev_id;
	mod_timer(&gpio_key->keytimer, jiffies + msecs_to_jiffies(10));
	return IRQ_HANDLED;
}


void timer_function(unsigned long arg)
{
	int val;
	struct gpio_key *gpio_key = (struct gpio_key *)arg;
	val = gpiod_get_value(gpio_key->gpiod);

	if(val == 1)
	{
		input_report_key(gpio_key->inputkey, key_n[gpio_key->cnt], 1);
		input_sync(gpio_key->inputkey);
	}

	else
	{
		input_report_key(gpio_key->inputkey, key_n[gpio_key->cnt], 0);
		input_sync(gpio_key->inputkey);
	}
}

/* 1. 从platform_device获得GPIO
 * 2. gpio=>irq
 * 3. request_irq
 */
static int gpio_key_probe(struct platform_device *pdev)
{
	int err;
	struct device_node *node = pdev->dev.of_node;
	int count;
	int i;
	int ret;
	enum of_gpio_flags flag;
		
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	count = of_gpio_count(node);
	if (!count)
	{
		printk("%s %s line %d, there isn't any gpio available\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	gpio_keys_100ask = kzalloc(sizeof(struct gpio_key) * count, GFP_KERNEL);
	for (i = 0; i < count; i++)
	{
		gpio_keys_100ask[i].gpio = of_get_gpio_flags(node, i, &flag);
		if (gpio_keys_100ask[i].gpio < 0)
		{
			printk("%s %s line %d, of_get_gpio_flags fail\n", __FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
		gpio_keys_100ask[i].gpiod = gpio_to_desc(gpio_keys_100ask[i].gpio);
		gpio_keys_100ask[i].flag = flag & OF_GPIO_ACTIVE_LOW;
		gpio_keys_100ask[i].irq  = gpio_to_irq(gpio_keys_100ask[i].gpio);
		gpio_keys_100ask[i].cnt = i;
	}

	for (i = 0; i < count; i++)
	{
		err = request_irq(gpio_keys_100ask[i].irq, gpio_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "100ask_gpio_key", &gpio_keys_100ask[i]);
	}


	gpio_key_class = class_create(THIS_MODULE, "100ask_gpio_key_class");
	if (IS_ERR(gpio_key_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "100ask_gpio_key");
		return PTR_ERR(gpio_key_class);
	}

	
	//device_create(gpio_key_class, NULL, MKDEV(major, 0), NULL, "100ask_gpio_key"); /* /dev/100ask_gpio_key */
	for (i = 0; i < count; i++)
	{
		init_timer(&gpio_keys_100ask[i].keytimer);
		gpio_keys_100ask[i].keytimer.function = timer_function;
		

		gpio_keys_100ask[i].inputkey = input_allocate_device();
		gpio_keys_100ask[i].inputkey->name = "Keyinpt";

		__set_bit(EV_KEY, gpio_keys_100ask[i].inputkey->evbit);
		__set_bit(EV_REP, gpio_keys_100ask[i].inputkey->evbit);

		__set_bit(key_n[i], gpio_keys_100ask[i].inputkey->keybit);

		ret = input_register_device(gpio_keys_100ask[i].inputkey);
		if (ret) 
		{
			printk("register input device failed!\r\n");
			return ret;
		}
	}
	
    return 0;
    
}

static int gpio_key_remove(struct platform_device *pdev)
{
	//int err;
	struct device_node *node = pdev->dev.of_node;
	int count;
	int i;

	device_destroy(gpio_key_class, MKDEV(major, 0));
	class_destroy(gpio_key_class);
	unregister_chrdev(major, "100ask_gpio_key");

	count = of_gpio_count(node);
	for (i = 0; i < count; i++)
	{
		free_irq(gpio_keys_100ask[i].irq, &gpio_keys_100ask[i]);
		del_timer_sync(&gpio_keys_100ask[i].keytimer);
		input_unregister_device(gpio_keys_100ask[i].inputkey);
		input_free_device(gpio_keys_100ask[i].inputkey);
	}

	
	kfree(gpio_keys_100ask);
    return 0;
}


static const struct of_device_id ask100_keys[] = {
    { .compatible = "myself_gpio" },
    { },
};

/* 1. 定义platform_driver */
static struct platform_driver gpio_keys_driver = {
    .probe      = gpio_key_probe,
    .remove     = gpio_key_remove,
    .driver     = {
        .name   = "100ask_gpio_key",
        .of_match_table = ask100_keys,
    },
};

/* 2. 在入口函数注册platform_driver */
static int __init gpio_key_init(void)
{
    int err;
    
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	
    err = platform_driver_register(&gpio_keys_driver); 
	
	return err;
}

/* 3. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数
 *     卸载platform_driver
 */
static void __exit gpio_key_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    platform_driver_unregister(&gpio_keys_driver);
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(gpio_key_init);
module_exit(gpio_key_exit);

MODULE_LICENSE("GPL");


