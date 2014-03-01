/*
 * arch/arm/mach-kirkwood/bubba3-setup.c
 * based on
 * arch/arm/mach-kirkwood/rd88f6281-setup.c
 *
 * For Bubba3 miniserver from Excito
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/physmap.h>
#include <linux/spi/flash.h>
#include <linux/spi/spi.h>
#include <linux/irq.h>
#include <linux/ata_platform.h>
#include <linux/mv643xx_eth.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/bubba3.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <mach/kirkwood.h>
#include <plat/orion-gpio.h>
#include <mach/bridge-regs.h>
#include <plat/time.h>
#include "common.h"
#include "mpp.h"

/*****************************************************************************
 * 2048KB SPI Flash on Boot Device (Numonyx MP25P16)
 ****************************************************************************/

static struct mtd_partition bubba3_flash_parts[] = {
	{
		.name = "u-boot",
		.size = SZ_512K+SZ_256K,
		.offset = 0,
	},
	{
		.name = "env",
		.size = SZ_128K,
		.offset = MTDPART_OFS_NXTBLK,
	},
	{
		.name = "data",
		.size = MTDPART_SIZ_FULL,
		.offset = MTDPART_OFS_NXTBLK,
	},
};

static const struct flash_platform_data bubba3_flash = {
	.type		= "m25p16",
	.name		= "spi_flash",
	.parts		= bubba3_flash_parts,
	.nr_parts	= ARRAY_SIZE(bubba3_flash_parts),
};

static struct spi_board_info __initdata bubba3_spi_slave_info[] = {
	{
		.modalias	= "m25p80",
		.platform_data	= &bubba3_flash,
		.irq		= -1,
		.max_speed_hz	= 40000000,
		.bus_num	= 0,
		.chip_select	= 0,
	},
};

/*****************************************************************************
 * GPIO and keys
 ****************************************************************************/

static struct gpio_keys_button bubba3_buttons[] = {
	[0] = {
		.code		= KEY_POWER,
		.gpio		= B3_POWER_BUTTON,
		.desc		= "Power button",
		.active_low	= 1,
	},
};

static struct gpio_keys_platform_data bubba3_button_data = {
	.buttons	= bubba3_buttons,
	.nbuttons	= ARRAY_SIZE(bubba3_buttons),
};

static struct platform_device bubba3_gpio_buttons = {
	.name		= "gpio-keys",
	.id		= -1,
	.dev		= {
		.platform_data 	= &bubba3_button_data,
	},
};


/*****************************************************************************
 * Ethernet
 ****************************************************************************/

static struct mv643xx_eth_platform_data bubba3_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0x08),
	/* in case a hardcoded MAC address is needed uncomment next line */
	/* .mac_addr	= {0x00, 0x0c, 0xc6, 0x76, 0x76, 0x2b}, */
};

static struct mv643xx_eth_platform_data bubba3_ge01_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0x18),
	/* in case a hardcoded MAC address is needed uncomment next line */
	/* .mac_addr	= {0x00, 0x0c, 0xc6, 0x76, 0x76, 0x2c}, */
};

static struct mv_sata_platform_data bubba3_sata_data = {
	.n_ports	= 2,
};

/*****************************************************************************
 * Timer
 ****************************************************************************/

static unsigned int bubba3_mpp_config[] __initdata = {
	MPP0_SPI_SCn,
	MPP1_SPI_MOSI,
	MPP2_SPI_SCK,
	MPP3_SPI_MISO,
	MPP4_NF_IO6,
	MPP5_NF_IO7,
	MPP6_SYSRST_OUTn,
	MPP7_PEX_RST_OUTn,
	MPP8_TW0_SDA,
	MPP9_TW0_SCK,
	MPP10_UART0_TXD,
	MPP11_UART0_RXD,
	MPP12_GPO,
	MPP13_UART1_TXD,
	MPP14_UART1_RXD,
	MPP15_SATA0_ACTn,
	MPP16_SATA1_ACTn,
	MPP17_SATA0_PRESENTn,
	MPP18_GPO,
	MPP19_GPO,
	MPP20_GE1_TXD0,
	MPP21_GE1_TXD1,
	MPP22_GE1_TXD2,
	MPP23_GE1_TXD3,
	MPP24_GE1_RXD0,
	MPP25_GE1_RXD1,
	MPP26_GE1_RXD2,
	MPP27_GE1_RXD3,
	MPP28_GPIO,
	MPP29_GPIO,
	MPP30_GE1_RXCTL,
	MPP31_GE1_RXCLK,
	MPP32_GE1_TCLKOUT,
	MPP33_GE1_TXCTL,
	MPP34_GPIO,
	MPP35_GPIO,
	MPP36_GPIO,
	MPP37_GPIO,
	MPP38_GPIO,
	MPP39_GPIO,
	MPP40_GPIO,
	MPP41_GPIO,
	MPP42_GPIO,
	MPP43_GPIO,
	MPP44_GPIO,
	MPP45_GPIO,
	MPP46_GPIO,
	MPP47_GPIO,
	MPP48_GPIO,
	MPP49_GPIO,
	0
};

static void __init bubba3_init(void)
{
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init();

	kirkwood_mpp_conf(bubba3_mpp_config);

	kirkwood_uart0_init();

	spi_register_board_info(bubba3_spi_slave_info,
				ARRAY_SIZE(bubba3_spi_slave_info));
	kirkwood_spi_init();

	kirkwood_i2c_init();

	platform_device_register(&bubba3_gpio_buttons);

	/* eth0 */
	kirkwood_ge00_init(&bubba3_ge00_data);

	/* eth1 */
	if (gpio_request(28, "PHY2 reset") != 0 ||
		gpio_direction_input(28) != 0) // high-z
		printk(KERN_ERR "can't deassert GPIO 28 (PHY2 reset)\n");
	else
		kirkwood_ge01_init(&bubba3_ge01_data);

	kirkwood_sata_init(&bubba3_sata_data);

	kirkwood_ehci_init();

}

static int __init bubba3_pci_init(void)
{
	if (machine_is_bubba3())
		kirkwood_pcie_init( KW_PCIE0 | KW_PCIE1 );

	return 0;
}
subsys_initcall(bubba3_pci_init);

MACHINE_START(BUBBA3, "BUBBA3 Kirkwood based miniserver")
	/* Maintainer: Tor Krill <tor@excito.com> */
	.atag_offset	= 0x100,
	.init_machine	= bubba3_init,
	.map_io		= kirkwood_map_io,
	.init_early 	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
        .init_time      = kirkwood_timer_init,
        .restart        = kirkwood_restart,
MACHINE_END
