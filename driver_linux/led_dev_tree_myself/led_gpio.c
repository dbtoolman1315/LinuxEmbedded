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
#include <asm/io.h>
#include <linux/of.h>
#include "led_opr.h"
#include "led_drv.h"
#include "led_resource.h"

static volatile unsigned int *CCM_CCGR1;
static volatile unsigned int *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3;
static volatile unsigned int *GPIO5_GDIR;
static volatile unsigned int *GPIO5_DR;

static u32 g_ledpins[100];
static int g_ledcnt = 0;

static int led_board_init(int which)
{
	int val;
	if(GROUP_PIN(5,1) == g_ledpins[which])
	{
		if(!CCM_CCGR1)
		{
			CCM_CCGR1 = ioremap(0x20C406C, 4);
            IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = ioremap(0x2290014, 4);
            GPIO5_GDIR = ioremap(0x020AC000 + 0x4, 4);
            GPIO5_DR = ioremap(0x020AC000 + 0, 4);      
		}

		*CCM_CCGR1 |= (3<<30);
        
        val = *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3;
        val &= ~(0xf);
        val |= (5);
        *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3 = val;
        
        *GPIO5_GDIR |= (1<<3);
	}
	

	return 0;
}

static int led_board_opr(int which, char state)
{
	if(GROUP_PIN(5,1) == g_ledpins[which])
	{
		if(state)
		{
			*GPIO5_DR &= ~(1 << 3);
		}

		else
		{
			*GPIO5_DR |= (1 <<3);
		}
	}

	return 0;
}

static int led_probe(struct platform_device *pdev)
{
	struct device_node *devnd;
	int err;
	u32 pin;

	devnd = pdev->dev.of_node;
	if(devnd == NULL)
	{
		printk("led_probe\n");
		return -1;
	}

	err = of_property_read_u32(devnd, "pin", &pin);
	g_ledpins[g_ledcnt] = pin;
	led_class_create_device(g_ledcnt);
	g_ledcnt++;
	
	return 0;
}
static int led_remove(struct platform_device *pdev)
{
	struct device_node * devnd;
	int err;
	u32 pin;
	int i;
	devnd = pdev->dev.of_node;

	err = of_property_read_u32(devnd, "pin", &pin);

	for (i = 0; i < g_ledcnt; i++)
	{
		if(g_ledpins[i] == pin)
		{
			g_ledpins[i] = -1;
			led_class_destory_device(i);
			break;
		}
	}

	for (i = 0; i < g_ledcnt; i++)
	{
		if(g_ledpins[i] != -1)
			break;
	}

	if(i == g_ledcnt)
		g_ledcnt = 0;
	
    return 0;
}

static const struct of_device_id myself_led_devid[] = 
{
	{.compatible = "myself,led",},
};

static struct platform_driver st_led_pdrv=
{
	.probe = led_probe,
	.remove = led_remove,
	.driver =
		{
			.name = "led_myself",
			.of_match_table = myself_led_devid,
		},
};


static struct led_opr st_led_opr = 
{
	.init = led_board_init,
	.opr = led_board_opr,
};

static int __init led_driver_init(void)
{
	int err;
	printk("led_driver_init%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	err = platform_driver_register(&st_led_pdrv);
	register_led_opr(&st_led_opr);
	return 0;
}

static void __exit led_driver_exit(void)
{
	platform_driver_unregister(&st_led_pdrv);
}

module_init(led_driver_init);
module_exit(led_driver_exit);

MODULE_LICENSE("GPL");


