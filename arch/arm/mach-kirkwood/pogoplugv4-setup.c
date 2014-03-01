/*
 * arch/arm/mach-kirkwood/pogoplugv4-setup.c
 *
 * Pogoplug Series 4 Setup
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/partitions.h>
#include <linux/ata_platform.h>
#include <linux/mv643xx_eth.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/leds.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/spi/flash.h>
#include <linux/spi/spi.h>
#include <linux/input.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include <linux/platform_data/mmc-mvsdio.h>
#include <plat/orion-gpio.h>
#include "common.h"
#include "mpp.h"

#define POGOPLUGV4_GPIO_USB_VBUS		10

static struct mtd_partition ppv4_nand_parts[] = {
	{
		.name	= "u-boot",
		.offset	= 0,
		.size	= 2 * SZ_1M
	}, {
		.name	= "uImage",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= 3 * SZ_1M
	}, {
		.name	= "uImage2",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= 3 * SZ_1M
	}, {
		.name	= "failsafe",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= SZ_8M
	}, {
		.name	= "root",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL
	},
};

static struct mv643xx_eth_platform_data pogoplugv4_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0),
};

static struct mv_sata_platform_data pogoplugv4_sata_data = {
	.n_ports	= 1,
};

static struct gpio_keys_button pogoplugv4_button_pins[] = {
	{
		.code		= KEY_EJECTCD,
		.gpio		= 29,
		.desc		= "Eject Button",
		.active_low	= 1,
	},
};

static struct gpio_keys_platform_data pogoplugv4_button_data = {
	.buttons	= pogoplugv4_button_pins,
	.nbuttons	= ARRAY_SIZE(pogoplugv4_button_pins),
};

static struct platform_device pogoplugv4_buttons = {
	.name		= "gpio-keys",
	.id		= -1,   
	.num_resources  = 0,
	.dev		= {
		.platform_data  = &pogoplugv4_button_data,
	},
};

static struct gpio_led pogoplugv4_led_pins[] = {
	{
		.name			= "status:green:health",
		.default_trigger	= "default-on",
		.gpio			= 22,
		.active_low		= 1,
	},
	{
		.name			= "status:red:fault",
		.default_trigger	= "none",
		.gpio			= 24,
		.active_low		= 1,
	},
};

static struct gpio_led_platform_data pogoplugv4_led_data = {
	.leds		= pogoplugv4_led_pins,
	.num_leds	= ARRAY_SIZE(pogoplugv4_led_pins),
};

static struct platform_device pogoplugv4_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &pogoplugv4_led_data,
	}
};

static struct mvsdio_platform_data ppv4_mvsdio_data = {
	.gpio_card_detect = 27,
};

static unsigned int ppv4_mpp_config[] __initdata = {
	MPP27_GPIO,	/* SD card detect */
	MPP29_GPIO,	/* Eject button */
	MPP22_GPIO,	/* Green LED */
	MPP24_GPIO,	/* Red LED */
	MPP12_SD_CLK,
	MPP13_SD_CMD,
	MPP14_SD_D0,
	MPP15_SD_D1,
	MPP16_SD_D2,
	MPP17_SD_D3,
	0
};

static const struct flash_platform_data pogoplugv4_spi_slave_data = {
	.type		= "m25p05-nonjedec",
};

static struct spi_board_info __initdata pogoplugv4_spi_slave_info[] = {
	{
		.modalias	= "m25p05-nonjedec",
		.platform_data	= &pogoplugv4_spi_slave_data,
		.irq		= -1,
		.max_speed_hz	= 20000000,
		.bus_num	= 0,
		.chip_select	= 0,
	},
};

static void __init pogoplugv4_init(void)
{
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init();
	kirkwood_mpp_conf(ppv4_mpp_config);

	orion_gpio_set_valid(POGOPLUGV4_GPIO_USB_VBUS, 1);
	if (gpio_request(POGOPLUGV4_GPIO_USB_VBUS, "USB VBUS") != 0 ||
	    gpio_direction_output(POGOPLUGV4_GPIO_USB_VBUS, 1) != 0)
		pr_err("POGOPLUGV4: failed to setup USB VBUS GPIO\n");

	kirkwood_ehci_init();
	kirkwood_ge00_init(&pogoplugv4_ge00_data);
	kirkwood_sata_init(&pogoplugv4_sata_data);
	spi_register_board_info(pogoplugv4_spi_slave_info,
				ARRAY_SIZE(pogoplugv4_spi_slave_info));
	kirkwood_spi_init();
	kirkwood_uart0_init();
	kirkwood_nand_init(ARRAY_AND_SIZE(ppv4_nand_parts), 25);
	kirkwood_sdio_init(&ppv4_mvsdio_data);
	platform_device_register(&pogoplugv4_leds);
	platform_device_register(&pogoplugv4_buttons);
}

static int __init pogoplugv4_pci_init(void)
{
	if (machine_is_pogoplugv4())
		kirkwood_pcie_init(KW_PCIE0);

	return 0;
}
subsys_initcall(pogoplugv4_pci_init);

MACHINE_START(POGOPLUGV4, "Pogoplug V4")
	/* Maintainer: Kevin Mihelich <kevin@archlinuxarm.org> */
	.atag_offset	= 0x100,
	.init_machine	= pogoplugv4_init,
	.map_io		= kirkwood_map_io,
	.init_early	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
	.init_time      = kirkwood_timer_init,
	.restart	= kirkwood_restart,
MACHINE_END
