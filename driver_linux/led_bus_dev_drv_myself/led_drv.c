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

#include "led_opr.h"

static int major = 0;
static struct led_opr *st_led_opr;
static struct class *pst_class;

/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* write(fd, &val, 1); */
static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char state;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&state,buf,1);

	/* 根据次设备号和status控制LED */
	st_led_opr->opr(minor, state);
	
	return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
	int minor = iminor(node);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 根据次设备号初始化LED */
	st_led_opr->init(minor);	
	return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

void register_led_opr(struct led_opr *p)
{
	st_led_opr = p;
}

void led_class_create_device(int minor)
{
	device_create(pst_class, NULL, MKDEV(major, minor), NULL, "myself_led%d", minor);
}

void led_class_destory_device(int minor)
{
	device_destroy(pst_class, MKDEV(major, minor));
}


EXPORT_SYMBOL(register_led_opr);
EXPORT_SYMBOL(led_class_create_device);
EXPORT_SYMBOL(led_class_destory_device);
static struct file_operations st_led_fopt = 
{
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.read    = led_drv_read,
	.write   = led_drv_write,
	.release = led_drv_close,
};

static int __init led_drv_init(void)
{
	int err;
	major = register_chrdev(0, "led_myself", &st_led_fopt);

	printk("led_drv init%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	pst_class = class_create(THIS_MODULE, "led_myself_class");
	err = PTR_ERR(pst_class);
	if(IS_ERR(pst_class))
	{
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "led_myself");
		return -1;
	}

	return 0;
}	

static void __exit led_drv_exit(void)
{
	class_destroy(pst_class);
	unregister_chrdev(major, "led_myself");
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");