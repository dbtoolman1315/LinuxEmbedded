#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>

static int major;
static struct class *class;
static struct gpio_desc *led_gpio;

static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* write(fd, &val, 1); */
static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char status;
	//struct inode *inode = file_inode(file);
	//int minor = iminor(inode);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&status, buf, 1);

	/* 根据次设备号和status控制LED */
	gpiod_set_value(led_gpio, status);
	
	return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
	//int minor = iminor(node);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 根据次设备号初始化LED */
	gpiod_direction_output(led_gpio, 0);
	
	return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}


static struct file_operations fopr =
{
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.read    = led_drv_read,
	.write   = led_drv_write,
	.release = led_drv_close,
};



static int led_tree_probe(struct platform_device *pdev)
{
	led_gpio = gpiod_get(pdev->dev, "led", 0);
	if (IS_ERR(led_gpio)) {
		dev_err(&pdev->dev, "Failed to get GPIO for led\n");
		return PTR_ERR(led_gpio);
	}

	major = register_chrdev(0, "led_tree",&fopr);

	class = class_create(THIS_MODULE, "myself_led_tree_class");
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "led_tree");
		gpiod_put(led_gpio);
		return PTR_ERR(led_class);
	}

	device_create(class, NULL, MKDEV(major, 0), NULL, "myself_led_tree_0");

	return 0;
}

static int led_tree_remove(struct platform_device *pdev)
{
	device_destroy(class, MKDEV(major, 0));
	class_destroy(class);
	unregister_chrdev(major, "led_tree");
	gpiod_put(led_gpio);
}

static struct of_device_id devid =
{
	.compatible = "myself_ledtree",
};

static struct platform_driver led_driver = 
{
	.probe = 
	.remove = 
	.driver = 
	{
		.name = "myself_led_tree",
		.compatible = devid,
	},
};


static int __init led_init(void)
{
	int err;
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = platform_driver_register(&led_driver);

	return 0;
}

static void __exit led_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    platform_driver_unregister(&led_driver);
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");

