
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/property.h>
#include <linux/acpi.h>
#include <linux/i2c.h>
#include <linux/nvmem-provider.h>
#include <linux/regmap.h>
#include <linux/pm_runtime.h>
#include <linux/gpio/consumer.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

static int major;
static struct class *ap3216_class;

static struct i2c_client *ap3216_client;

static ssize_t ap3216c_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char kernel_buf[6];
	int val;
	
	if (size != 6)
		return -EINVAL;

	val = i2c_smbus_read_word_data(ap3216_client, 0xA); /* read IR */
	kernel_buf[0] = val & 0xff;
	kernel_buf[1] = (val>>8) & 0xff;
	
	val = i2c_smbus_read_word_data(ap3216_client, 0xC); /* read 光强 */
	kernel_buf[2] = val & 0xff;
	kernel_buf[3] = (val>>8) & 0xff;

	val = i2c_smbus_read_word_data(ap3216_client, 0xE); /* read 距离 */
	kernel_buf[4] = val & 0xff;
	kernel_buf[5] = (val>>8) & 0xff;
	
	err = copy_to_user(buf, kernel_buf, size);
	return size;
}

static int ap3216c_open (struct inode *node, struct file *file)
{
	i2c_smbus_write_byte_data(ap3216_client, 0, 0x4);
	/* delay for reset */
	mdelay(20);
	i2c_smbus_write_byte_data(ap3216_client, 0, 0x3);
	mdelay(250);
	return 0;
}
static struct file_operations ap3216_ops = 
{
	.owner = THIS_MODULE,
	.open = ap3216c_open,
	.read = ap3216c_read,
};

static int ap3216_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *ap3216_dev;
	ap3216_client = client;
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	major = register_chrdev(0, "ap3216c",&ap3216_ops);
	if(major < 0)
	{
		printk("register_chrdev err\n");
		return -1;

	}
		
	ap3216_class = class_create(THIS_MODULE, "ap3216_class");
	if (IS_ERR(ap3216_class)) 
	{
		printk(KERN_ERR "class_create() failed for ap3216_class\n");
		unregister_chrdev(major, "ap3216c");
		return -1;
	}
	
	ap3216_dev = device_create(ap3216_class, NULL, MKDEV(major, 0), NULL, "ap3216_i2c_dev");
	if (IS_ERR(ap3216_dev)) 
	{
		printk(KERN_ERR "device_create failed for ap3216c");
		class_destroy(ap3216_class);
		unregister_chrdev(major, "ap3216c");
		return -1;
	}
	return 0;
}

static int ap3216_i2c_remove(struct i2c_client *i2c)
{
	device_destroy(ap3216_class, MKDEV(major, 0));
	class_destroy(ap3216_class);
	unregister_chrdev(major, "ap3216c");
	return 0;
}


static const struct i2c_device_id ap3216_id_table[] = {
	{"ap3216c", 0},
	{}
};

static const struct of_device_id ap3216_of_match[] = {
	{ .compatible = "lite-on,ap3216c", },
	{},
};


static struct i2c_driver ap3216_driver = {
	.driver = {
		.name = "ap3216c",
		.of_match_table = ap3216_of_match,
		},
	.probe = ap3216_i2c_probe,
	.remove = ap3216_i2c_remove,
	.id_table = ap3216_id_table,
};


static int __init ap3216_drv_init(void)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return i2c_add_driver(&ap3216_driver);
}

static void __exit ap3216c_drv_exit(void)
{
	i2c_del_driver(&ap3216_driver);
}

module_init(ap3216_drv_init);


module_exit(ap3216c_drv_exit);

MODULE_LICENSE("GPL");



