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

#include "gpio_drv.h"
static int major;
static struct class *class;
static struct key_operations *keyopr;

void register_key_operations(struct key_operations *opr)
{
	int i;
	keyopr = opr;
	for(i = 0; i < keyopr->count; i++)
	{
		device_create(class, NULL, MKDEV(major, i), NULL, "myself_key%d",i);
	}
}

void unregister_key_operations(struct key_operations *opr)
{
	int i;
	keyopr = opr;
	for(i = 0; i < keyopr->count; i++)
	{
		device_destroy(class,  MKDEV(major, i));
	}
}

EXPORT_SYMBOL(register_key_operations);
EXPORT_SYMBOL(unregister_key_operations);


static int key_open(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	keyopr->init(minor);
	return 0;
}

static ssize_t key_read (struct file *file, char __user *buf, size_t size, loff_t *off)
{
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	int status;
	status = keyopr->opr(minor);
	copy_to_user(buf, &status, 1);
	return 0;
}

static struct file_operations fop = 
{
	.read = key_read,
	.open = key_open,
};

static int __init key_drv_init(void)
{
	major = register_chrdev(0, "myself_key", &fop);

	class = class_create(THIS_MODULE, "myself_key");
	if(IS_ERR(class))
	{
		unregister_chrdev(major, "myself_key");
		return -1;
	}

	return 0;
}

static void __exit key_drv_exit(void)
{
	class_destroy(class);
	unregister_chrdev(major, "myself_key");
}

module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");

