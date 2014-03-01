/*
 * arch/arm/mach-kirkwood/topkick-setup.c
 *
 * USI Topkick Setup
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mv643xx_eth.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include <linux/platform_data/mmc-mvsdio.h>
#include "common.h"
#include "mpp.h"

static struct mtd_partition topkick_nand_parts[] = {
	{
		.name	= "u-boot",
		.offset	= 0,
		.size	= 0x180000
	}, {
		.name	= "u-boot-env",
		.offset	= 0x180000,
		.size	= 128 * 1024
	}, {
		.name	= "uImage",
		.offset	= 2 * 1024 * 1024,
		.size	= 6 * 1024 * 1024
	}, {
		.name	= "rootfs",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL
	},
};

static struct mv643xx_eth_platform_data topkick_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0),
};

static struct mv_sata_platform_data topkick_sata_data = {
	.n_ports	= 1,
};

static struct mvsdio_platform_data topkick_mvsdio_data = {
	.gpio_card_detect = 47, /* MPP47 used as SD card detect */
};


static struct gpio_led topkick_led_pins[] = {
	{
		.name			= "status:blue:disk",
		.default_trigger	= "none",
		.gpio			= 21,
		.active_low		= 1,
	},
	{
		.name			= "status:red:fault",
		.default_trigger	= "none",
		.gpio			= 37,
		.active_low		= 1,
	},
	{
		.name			= "status:blue:health",
		.default_trigger	= "default-on",
		.gpio			= 38,
		.active_low		= 1,
	},
	{
		.name			= "status:green:misc",
		.default_trigger	= "none",
		.gpio			= 39,
		.active_low		= 1,
	},
	{
		.name			= "status:orange:misc",
		.default_trigger	= "none",
		.gpio			= 48,
		.active_low		= 1,
	},
};

static struct gpio_led_platform_data topkick_led_data = {
	.leds		= topkick_led_pins,
	.num_leds	= ARRAY_SIZE(topkick_led_pins),
};

static struct platform_device topkick_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &topkick_led_data,
	}
};

static unsigned int topkick_mpp_config[] __initdata = {
	MPP12_SD_CLK,	/* SDIO Clock */
	MPP13_SD_CMD,	/* SDIO Cmd */
	MPP14_SD_D0,	/* SDIO Data 0 */
	MPP15_SD_D1,	/* SDIO Data 1 */
	MPP16_SD_D2,	/* SDIO Data 2 */
	MPP17_SD_D3,	/* SDIO Data 3 */
	MPP21_GPIO,	/* LED Blue SATA */
	MPP35_GPIO,	/* USB Power Enable */
	MPP36_GPIO,	/* SATA Power Enable */
	MPP37_GPIO,	/* LED Red  System */
	MPP38_GPIO,	/* LED Blue System */
	MPP39_GPIO,	/* LED Green Misc/WiFi */
	MPP43_GPIO,	/* WOL Eth WOL */
	MPP44_GPIO,	/* SW GW Mode */
	MPP45_GPIO,	/* SW AP Mode */
	MPP46_GPIO,	/* SW Power Off */
	MPP47_GPIO,	/* SDIO Detect */
	MPP48_GPIO,	/* LED Orange Misc/WiFi */
	0
};

static void __init topkick_init(void)
{
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init();

	/* setup gpio pin select */
	kirkwood_mpp_conf(topkick_mpp_config);

	kirkwood_uart0_init();
	kirkwood_nand_init(ARRAY_AND_SIZE(topkick_nand_parts), 40);

	if (gpio_request(35, "USB Power Enable") != 0 ||
	    gpio_direction_output(35, 1) != 0)
		printk(KERN_ERR "can't set up GPIO 35 (USB Power Enable)\n");
	if (gpio_request(36, "SATA Power Enable") != 0 ||
	    gpio_direction_output(36, 1) != 0)
		printk(KERN_ERR "can't set up GPIO 36 (SATA Power Enable)\n");
	kirkwood_ge00_init(&topkick_ge00_data);
	kirkwood_ehci_init();
	kirkwood_sata_init(&topkick_sata_data);
	kirkwood_sdio_init(&topkick_mvsdio_data);

	platform_device_register(&topkick_leds);
}

MACHINE_START(TOPKICK, "USI Topkick")
	/* Maintainer: Mike Brown <mbrown@archlinuxarm.org> */
	.atag_offset	= 0x100,
	.init_machine	= topkick_init,
	.map_io		= kirkwood_map_io,
	.init_early	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
	.init_time      = kirkwood_timer_init,
	.restart	= kirkwood_restart,
MACHINE_END
