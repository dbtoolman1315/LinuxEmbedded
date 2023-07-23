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
#include <linux/platform_device.h>

#include "led_resource.h"

static void led_dev_release(struct device *dev)
{
}


static struct resource resources[] = {
        {
                .start = GROUP_PIN(5,1),
                .flags = IORESOURCE_IRQ,
                .name = "myself_led_pin",
        },

};

static struct platform_device st_led_pdev = 
{
        .name = "led_myself",
        .num_resources = ARRAY_SIZE(resources),
        .resource = resources,
        .dev = {
                .release = led_dev_release,
         },
};


static int __init led_drv_init(void)
{
	int err;
	err = platform_device_register(&st_led_pdev);
	return 0;
}

static void __exit led_drv_exit(void)
{
	platform_device_unregister(&st_led_pdev);
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");


