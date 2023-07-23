#include <linux/module.h>

#include <linux/fs.h>
#include <linux/io.h>
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
#include <asm/io.h>

#include "gpio_drv.h"

struct iomux {
    volatile unsigned int unnames[23];
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO00; /* offset 0x5c */
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO01;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO02;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO05;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO06;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO07;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO08;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO09;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_UART1_TX_DATA;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_UART1_RX_DATA;
    volatile unsigned int IOMUXC_SW_MUX_CTL_PAD_UART1_CTS_B;
};

struct imx6ull_gpio {
    volatile unsigned int dr;
    volatile unsigned int gdir;
    volatile unsigned int psr;
    volatile unsigned int icr1;
    volatile unsigned int icr2;
    volatile unsigned int imr;
    volatile unsigned int isr;
    volatile unsigned int edge_sel;
};

/* enable GPIO4 */
static volatile unsigned int *CCM_CCGR3; 

/* enable GPIO5 */
static volatile unsigned int *CCM_CCGR1; 

/* set GPIO5_IO03 as GPIO */
static volatile unsigned int *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1;

/* set GPIO4_IO14 as GPIO */
static volatile unsigned int *IOMUXC_SW_MUX_CTL_PAD_NAND_CE1_B;

static struct iomux *iomux;

static struct imx6ull_gpio *gpio4;
static struct imx6ull_gpio *gpio5;

static int key_opr_init(int which)
{
	if (!CCM_CCGR1)
    {
        CCM_CCGR1 = ioremap(0x20C406C, 4);
        CCM_CCGR3 = ioremap(0x20C4074, 4);
        IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1 = ioremap(0x229000C, 4);
		IOMUXC_SW_MUX_CTL_PAD_NAND_CE1_B        = ioremap(0x20E01B0, 4);

        iomux = ioremap(0x20e0000, sizeof(struct iomux));
        gpio4 = ioremap(0x020A8000, sizeof(struct imx6ull_gpio));
        gpio5 = ioremap(0x20AC000, sizeof(struct imx6ull_gpio));
    }

    if (which == 0)
    {
        /* 1. enable GPIO5 
         * CG15, b[31:30] = 0b11
         */
        *CCM_CCGR1 |= (3<<30);
        
        /* 2. set GPIO5_IO01 as GPIO 
         * MUX_MODE, b[3:0] = 0b101
         */
        *IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1 = 5;

        /* 3. set GPIO5_IO01 as input 
         * GPIO5 GDIR, b[1] = 0b0
         */
        gpio5->gdir &= ~(1<<1);
    }
    else if(which == 1)
    {
        /* 1. enable GPIO4 
         * CG6, b[13:12] = 0b11
         */
        *CCM_CCGR3 |= (3<<12);
        
        /* 2. set GPIO4_IO14 as GPIO 
         * MUX_MODE, b[3:0] = 0b101
         */
        IOMUXC_SW_MUX_CTL_PAD_NAND_CE1_B = 5;

        /* 3. set GPIO4_IO14 as input 
         * GPIO4 GDIR, b[14] = 0b0
         */
        gpio4->gdir &= ~(1<<14);
    }	
}


static int key_opr_read (int which) /* 读button, which-哪个 */
{
    //printk("%s %s line %d, button %d, 0x%x\n", __FILE__, __FUNCTION__, __LINE__, which, *GPIO1_DATAIN);
    if (which == 0)
        return (gpio5->psr & (1<<1)) ? 1 : 0;
    else
        return (gpio4->psr & (1<<14)) ? 1 : 0;
}
    
struct key_operations kopr = 
{
	.count = 1,
	.init = key_opr_init ,
	.opr = key_opr_read,
};

static int __init key_board_init(void)
{
	register_key_operations(&kopr);
	return 0;
}

static void __exit key_board_exit(void)
{
	unregister_key_operations(&kopr);
}

module_init(key_board_init);
module_exit(key_board_exit);
MODULE_LICENSE("GPL");