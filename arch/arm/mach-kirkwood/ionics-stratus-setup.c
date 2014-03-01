/*
 * arch/arm/mach-kirkwood/ionics-stratus-setup.c
 *
 * Ionics Stratus Board Setup arcNumber 4184
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/mtd/partitions.h>
#include <linux/mv643xx_eth.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include <linux/platform_data/mmc-mvsdio.h>
#include "common.h"
#include "mpp.h"

static struct mtd_partition ionics_stratus_nand_parts[] = {
	{
		.name = "u-boot",
		.offset = 0,
		.size = SZ_1M
	}, {
		.name = "uImage",
		.offset = MTDPART_OFS_NXTBLK,
		.size = SZ_4M
	}, {
		.name = "root",
		.offset = MTDPART_OFS_NXTBLK,
		.size = MTDPART_SIZ_FULL
	},
};

static struct mv643xx_eth_platform_data ionics_stratus_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0),
};

static struct mvsdio_platform_data ionics_stratus_mvsdio_data = {
	/* unfortunately the CD signal has not been connected */
};

static struct gpio_led ionics_stratus_led_pins[] = {
	{
		.name			= "status:green:led1",
		.default_trigger	= "none",
		.gpio			= 44,
		.active_low		= 1,
	},
	{
		.name           	= "status:green:led2",
		.default_trigger    	= "none",
		.gpio           	= 40,
		.active_low     	= 1,
	},
	{
		.name           	= "status:green:led3",
		.default_trigger	= "default-on",
		.gpio			= 36,
		.active_low		= 1,
	},
        {
                .name                   = "status:green:led4",
                .default_trigger        = "default-on",
                .gpio                   = 39,
                .active_low             = 1,
        },

};

static struct gpio_led_platform_data ionics_stratus_led_data = {
	.leds		= ionics_stratus_led_pins,
	.num_leds	= ARRAY_SIZE(ionics_stratus_led_pins),
};

static struct platform_device ionics_stratus_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
	.platform_data	= &ionics_stratus_led_data,
	}
};

static unsigned int ionics_stratus_mpp_config[] __initdata = {
	MPP21_GPIO,	/* USB PORT 1 Pw Enable */
	MPP32_GPIO,	/* USB PORT 2 Pw Enable */
	MPP48_GPIO,	/* WIFI Power Down */
	MPP49_GPIO,	/* WIFI Host Wakeup */
	MPP42_GPIO,	/* WIFI MAC Wakeup */
	MPP44_GPIO,	/* LED 1 */
	MPP40_GPIO,	/* LED 2 */
	MPP36_GPIO,	/* LED 3 */
	MPP39_GPIO,	/* LED 4 */
	MPP23_GPIO,	/* BTN 1 */
	MPP37_GPIO,	/* BTN 2 */
	MPP38_GPIO,	/* BTN 3 */
	0
};

static void __init ionics_stratus_init(void)
{
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init();

	/* setup gpio pin select */
	kirkwood_mpp_conf(ionics_stratus_mpp_config);

	kirkwood_uart0_init();
	kirkwood_nand_init(ARRAY_AND_SIZE(ionics_stratus_nand_parts), 25);

	kirkwood_ehci_init();

	kirkwood_ge00_init(&ionics_stratus_ge00_data);
	kirkwood_sdio_init(&ionics_stratus_mvsdio_data);

	platform_device_register(&ionics_stratus_leds);
}

MACHINE_START(IONICS_STRATUS, "Ionics Plug Computer Plus - Stratus")
	/* Maintainer: Mike Brown <mbrown@archlinuxarm.org> */
	.atag_offset	= 0x100,
	.init_machine	= ionics_stratus_init,
	.map_io		= kirkwood_map_io,
	.init_early	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
	.init_time      = kirkwood_timer_init,
	.restart	= kirkwood_restart,
MACHINE_END
