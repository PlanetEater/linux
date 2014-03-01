/*
 * arch/arm/mach-kirkwood/nsa325-setup.c
 *
 * ZyXEL NSA325 2-Bay Power Media Server Setup
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/ata_platform.h>
#include <linux/mv643xx.h>
#include <linux/mv643xx_eth.h>
#include <linux/phy.h>
#include <linux/marvell_phy.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/leds.h>
#include <linux/pci.h>
#include <linux/input.h>
#include <linux/nsa3xx-hwmon.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include <plat/orion-gpio.h>
#include "common.h"
#include "mpp.h"

static struct mtd_partition nsa325_nand_parts[] = {
       {
               .name = "uboot",
               .offset = 0,
               .size = SZ_1M,
               .mask_flags = MTD_WRITEABLE
       }, {
               .name = "uboot_env",
               .offset = MTDPART_OFS_NXTBLK,
               .size = SZ_512K
       }, {
               .name = "key_store",
               .offset = MTDPART_OFS_NXTBLK,
               .size = SZ_512K
       }, {
               .name = "info",
               .offset = MTDPART_OFS_NXTBLK,
               .size = SZ_512K
       }, {
               .name = "etc",
               .offset = MTDPART_OFS_NXTBLK,
               .size = 10 * SZ_1M
       }, {
               .name = "kernel_1",
               .offset = MTDPART_OFS_NXTBLK,
               .size = 10 * SZ_1M
       }, {
               .name = "rootfs1",
               .offset = MTDPART_OFS_NXTBLK,
               .size = 48 * SZ_1M - SZ_256K
       }, {
               .name = "kernel_2",
               .offset = MTDPART_OFS_NXTBLK,
               .size = 10 * SZ_1M
       }, {
               .name = "rootfs2",
               .offset = MTDPART_OFS_NXTBLK,
               .size = 48 * SZ_1M - SZ_256K
       },
};

static struct i2c_board_info __initdata nsa325_i2c_rtc = {
       I2C_BOARD_INFO("pcf8563", 0x51),
};

static struct mv643xx_eth_platform_data nsa325_ge00_data = {
       .phy_addr       = MV643XX_ETH_PHY_ADDR(1),
};

static struct mv_sata_platform_data nsa325_sata_data = {
       .n_ports        = 2,
};

static struct gpio_keys_button nsa325_button_pins[] = {
       {
               .code           = KEY_RESTART,
               .gpio           = 36,
               .desc           = "Reset",
               .active_low     = 1,
               .debounce_interval      = 1000,
       }, {
               .code           = KEY_COPY,
               .gpio           = 37,
               .desc           = "Copy",
               .active_low     = 1,
               .debounce_interval      = 1000,
       }, {
               .code           = KEY_POWER,
               .gpio           = 46,
               .desc           = "Power",
               .active_low     = 0,
               .debounce_interval      = 1000,
       },
};

static struct gpio_keys_platform_data nsa325_button_data = {
       .buttons        = nsa325_button_pins,
       .nbuttons       = ARRAY_SIZE(nsa325_button_pins),
};

static struct platform_device nsa325_buttons = {
       .name           = "gpio-keys",
       .id             = -1,
       .num_resources  = 0,
       .dev            = {
               .platform_data  = &nsa325_button_data,
       },
};

static struct gpio_led nsa325_led_pins[] = {
       {
               .name                   = "nsa325:green:hdd2",
               .default_trigger        = "ide-disk",
               .gpio                   = 12,
               .active_low             = 0,
       }, {
               .name                   = "nsa325:red:hdd2",
               .default_trigger        = "default-off",
               .gpio                   = 13,
               .active_low             = 0,
       }, {
               .name                   = "nsa325:green:usb",
               .default_trigger        = "default-off",
               .gpio                   = 15,
               .active_low             = 0,
       }, {
               .name                   = "nsa325:green:sys",
               .default_trigger        = "default-off",
               .gpio                   = 28,
               .active_low             = 0,
       }, {
               .name                   = "nsa325:orange:sys",
               .default_trigger        = "default-on",
               .gpio                   = 29,
               .active_low             = 0,
       }, {
               .name                   = "nsa325:green:copy",
               .default_trigger        = "default-off",
               .gpio                   = 39,
               .active_low             = 0,
       }, {
               .name                   = "nsa325:red:copy",
               .default_trigger        = "default-off",
               .gpio                   = 40,
               .active_low             = 0,
       }, {
               .name                   = "nsa325:green:hdd1",
               .default_trigger        = "ide-disk",
               .gpio                   = 41,
               .active_low             = 0,
       }, {
               .name                   = "nsa325:red:hdd1",
               .default_trigger        = "default-off",
               .gpio                   = 42,
               .active_low             = 0,
       },
};

static int nsa325_gpio_blink_set(unsigned gpio, int state,
       unsigned long *delay_on, unsigned long *delay_off)
{
       switch(state) {
               case GPIO_LED_NO_BLINK_LOW:
               case GPIO_LED_NO_BLINK_HIGH:
                       orion_gpio_set_blink(gpio, 0);
                       gpio_set_value(gpio, state);
               break;
               case GPIO_LED_BLINK:
                       orion_gpio_set_blink(gpio, 1);
               break;
       }
       return 0;
}

static struct gpio_led_platform_data nsa325_led_data = {
       .leds           = nsa325_led_pins,
       .num_leds       = ARRAY_SIZE(nsa325_led_pins),
       .gpio_blink_set = nsa325_gpio_blink_set,
};

static struct platform_device nsa325_leds = {
       .name   = "leds-gpio",
       .id     = -1,
       .dev    = {
               .platform_data  = &nsa325_led_data,
       },
};

static struct nsa3xx_hwmon_platform_data nsa325_hwmon_data = {
       /* GPIOs connected to Holtek HT46R065 MCU */
       .act_pin  = 17,
       .clk_pin  = 16,
       .data_pin = 14,
};

static struct platform_device nsa325_hwmon = {
       .name           = "nsa3xx-hwmon",
       .id             = -1,
       .num_resources  = 0,
       .dev            = {
               .platform_data  = &nsa325_hwmon_data,
       },
};

static unsigned int nsa325_mpp_config[] __initdata = {
       MPP8_TW0_SDA,   /* PCF8563 RTC chip   */
       MPP9_TW0_SCK,   /* connected to TWSI  */
       MPP12_GPO,      /* HDD2 LED (green)   */
       MPP13_GPIO,     /* ? HDD2 LED (red) ? */
       MPP14_GPIO,     /* MCU DATA pin (in)  */
       MPP15_GPIO,     /* USB LED (green)    */
       MPP16_GPIO,     /* MCU CLK pin (out)  */
       MPP17_GPIO,     /* MCU ACT pin (out)  */
       MPP21_GPIO,     /* USB power          */
       MPP28_GPIO,     /* SYS LED (green)    */
       MPP29_GPIO,     /* SYS LED (orange)   */
       MPP36_GPIO,     /* reset button       */
       MPP37_GPIO,     /* copy button        */
       MPP38_GPIO,     /* VID B0             */
       MPP39_GPIO,     /* COPY LED (green)   */
       MPP40_GPIO,     /* COPY LED (red)     */
       MPP41_GPIO,     /* HDD1 LED (green)   */
       MPP42_GPIO,     /* HDD1 LED (red)     */
       MPP43_GPIO,     /* HTP pin            */
       MPP44_GPIO,     /* buzzer             */
       MPP45_GPIO,     /* VID B1             */
       MPP46_GPIO,     /* power button       */
       MPP47_GPIO,     /* HDD2 power         */
       MPP48_GPIO,     /* power off          */
       0
};

#define NSA325_GPIO_WATCHDOG   14
#define NSA325_GPIO_USB_POWER  21
#define NSA325_GPIO_HDD2_POWER 47
#define NSA325_GPIO_POWER_OFF  48

static void nsa325_power_off(void)
{
       gpio_set_value(NSA325_GPIO_POWER_OFF, 1);
}

static int __initdata usb_power = 1; /* default "on" */

static int __init nsa325_usb_power(char *str)
{
       usb_power = strncmp(str, "off", 3) ? 1 : 0;
       return 1;
}
/* Parse boot_command_line string nsa325_usb_power=on|off */
__setup("nsa325_usb_power=", nsa325_usb_power);

/* the nsa325 uses the 88E1310S Alaska, and has an MCU attached to the LED[2] via tristate interrupt */
static int nsa325_phy_fixup(struct phy_device *phydev)
{
       int err;
       int temp;
       /* go to page 3 */
       err = phy_write(phydev, 22, 3);
       if (err < 0)
           return err;
       /* read page 3, register 17 */
       temp = phy_read(phydev, 17);
       /* clear bit 4, set bit 5 */
       temp &= ~(1<<4);
       temp |= (1<<5);
       /* write page 3, register 17 */
       err = phy_write(phydev, 17, temp);
       if (err < 0)
           return err;
       /* go to page 0 */
       err = phy_write(phydev, 22, 0);
       if (err < 0)
           return err;

       return 0;
}

static void __init nsa325_init(void)
{
       /*
        * Basic setup. Needs to be called early.
        */
       kirkwood_init();

       kirkwood_mpp_conf(nsa325_mpp_config);

       /* setup the phy fixup */
       phy_register_fixup_for_uid(MARVELL_PHY_ID_88E1318S,MARVELL_PHY_ID_MASK,nsa325_phy_fixup);
       kirkwood_ge00_init(&nsa325_ge00_data);

       kirkwood_uart0_init();
       kirkwood_nand_init(ARRAY_AND_SIZE(nsa325_nand_parts), 40);

       /* turn off the watchdog */
       gpio_set_value(NSA325_GPIO_WATCHDOG, 1);

       /* turn on the second drive bay */
       if (gpio_request(NSA325_GPIO_HDD2_POWER, "HDD2 Power Enable") ||
           gpio_direction_output(NSA325_GPIO_HDD2_POWER,1))
               pr_err("nsa325: failed to configure HDD2 power enable GPIO\n");
       gpio_free(NSA325_GPIO_HDD2_POWER);

       kirkwood_sata_init(&nsa325_sata_data);

       platform_device_register(&nsa325_leds);
       platform_device_register(&nsa325_buttons);
       platform_device_register(&nsa325_hwmon);

       kirkwood_i2c_init();
       i2c_register_board_info(0, &nsa325_i2c_rtc, 1);

       if (gpio_request(NSA325_GPIO_USB_POWER, "USB Power Enable") ||
           gpio_direction_output(NSA325_GPIO_USB_POWER, usb_power))
               pr_err("nsa325: failed to configure USB power enable GPIO\n");
       gpio_free(NSA325_GPIO_USB_POWER);

       kirkwood_ehci_init();

       if (gpio_request(NSA325_GPIO_POWER_OFF, "power-off") ||
           gpio_direction_output(NSA325_GPIO_POWER_OFF, 0))
               pr_err("nsa325: failed to configure power-off GPIO\n");
       else
               pm_power_off = nsa325_power_off;
}

static int __init nsa325_pci_init(void)
{
        if (machine_is_nsa325())
                kirkwood_pcie_init(KW_PCIE0);

        return 0;
}
subsys_initcall(nsa325_pci_init);

MACHINE_START(NSA325, "ZyXEL NSA325 2-Bay Power Media Server")
       /* Maintainer: Jason Plum <max@warheads.net> */
       .atag_offset    = 0x100,
       .init_machine   = nsa325_init,
       .map_io         = kirkwood_map_io,
       .init_early     = kirkwood_init_early,
       .init_irq       = kirkwood_init_irq,
       .init_time      = kirkwood_timer_init,
       .restart        = kirkwood_restart,
MACHINE_END
