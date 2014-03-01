/*
 * Excito BUBBA|3 led driver.
 *
 * Copyright (C) 2010 Excito Elektronik i Skåne AB
 * Author: "Tor Krill" <tor@excito.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This driver provides an interface to the GPIO functionality on BUBBA|3
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/bubba3.h>
#include <asm/mach-types.h>

/* Mark this file for ident */
static char* ver="0.1";
static char* build=__DATE__ " " __TIME__;

#define DEVNAME "bubbatwo"
#define LED_DEFAULT_FREQ 0x8000

/* Meta information for this module */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tor Krill <tor@excito.com>");
MODULE_DESCRIPTION("BUBBA|3 led driver");

/* Forwards */
static int b3_probe(struct platform_device  *dev);
static int b3_remove(struct platform_device *dev);
static void b3_led_on(void);

#define MODE_OFF	0
#define MODE_BLINK	1
#define MODE_LIT	2
#define BUZZ_OFF	0
#define BUZZ_ON		1
#define LED_BLUE	0
#define LED_RED		1
#define LED_GREEN	2
#define LED_BOOT	3
#define LED_INSTALL	LED_GREEN

struct b3_stateinfo{
	u32 mode;
	u32 freq;
	u32 buzz;
	u32 color;
};

static struct b3_stateinfo b3_data;

static void b3_led_reset(void)
{
	gpio_set_value(B3_LED_INTERVAL,0);
	udelay(1800);
	gpio_set_value(B3_LED_INTERVAL,1);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,0);
	udelay(1800);
}

static void b3_led_train_start(void)
{
	gpio_set_value(B3_LED_INTERVAL,1);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,0);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,1);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,0);
	udelay(1800);
}

static void b3_led_train_end(void)
{
	gpio_set_value(B3_LED_INTERVAL,1);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,0);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,1);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,0);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,1);
	udelay(10);
	gpio_set_value(B3_LED_INTERVAL,0);
	udelay(1800);
}

static void b3_led_color(u32 color){
	gpio_set_value(B3_FRONT_LED_RED,0);
	gpio_set_value(B3_FRONT_LED_BLUE,0);
	gpio_set_value(B3_FRONT_LED_GREEN,0);

	switch(color){
	case LED_BOOT:
		gpio_set_value(B3_FRONT_LED_RED,1);
		gpio_set_value(B3_FRONT_LED_BLUE,1);
		break;
	case LED_RED:
		gpio_set_value(B3_FRONT_LED_RED,1);
		break;
	case LED_GREEN:
		gpio_set_value(B3_FRONT_LED_GREEN,1);
		break;
	case LED_BLUE:
	default:
		gpio_set_value(B3_FRONT_LED_BLUE,1);
		break;
	}
}

static void b3_led_on(void)
{
	b3_led_color(b3_data.color);

	b3_led_reset();

	b3_led_train_start();
	/* NOOP, pass through mode */
	b3_led_train_end();

	gpio_set_value(B3_LED_INTERVAL,1);
}

static void b3_led_off(void)
{
	gpio_set_value(B3_FRONT_LED_RED,0);
	gpio_set_value(B3_FRONT_LED_BLUE,0);
	gpio_set_value(B3_FRONT_LED_GREEN,0);

	b3_led_reset();
}

static void b3_buzz_on(void)
{
	gpio_set_value(B3_BUZZER_ENABLE,1);
}

static void b3_buzz_off(void)
{
	gpio_set_value(B3_BUZZER_ENABLE,0);
}


static struct platform_device *b3_device;

static struct platform_driver b3_driver = {
		.driver = {
				.name = DEVNAME,
				.owner = THIS_MODULE,
		},
		.probe = b3_probe,
		.remove = b3_remove,
};

static ssize_t	b3_show_ledmode(struct device *dev, struct device_attribute *attr, char *buffer)
{
	ssize_t len = 0;
	switch(b3_data.mode){
	case MODE_OFF:
		len = sprintf (buffer+len, "off");
		break;
	case MODE_BLINK:
		len = sprintf (buffer+len, "blink");
		break;
	case MODE_LIT:
		len = sprintf (buffer+len, "lit");
		break;
	default:
		len = sprintf (buffer+len, "unknown");
	}

	return len;
}

static ssize_t b3_store_ledmode(struct device *dev, struct device_attribute *attr,const char *buffer, size_t size)
{

	if(size<1){
		return -EINVAL;
	}
	/* Do a nasty shortcut here only look at first char */
	switch(buffer[0]){
	case 'o':
		b3_data.mode=MODE_OFF;
		b3_led_off ();
		break;
	case 'b':
		/* For now we dont allow blink. */
	case 'l':
		b3_data.mode=MODE_LIT;
		b3_led_on ();
		break;
	default:
		return -EINVAL;
	}

	return size;
}

static ssize_t	b3_show_ledfreq(struct device *dev, struct device_attribute *attr, char *buffer)
{
	ssize_t len = 0;

	len = sprintf (buffer+len, "%u", b3_data.freq);

	return len;
}

static ssize_t b3_store_ledfreq(struct device *dev, struct device_attribute *attr,const char *buffer, size_t size)
{

	b3_data.freq = simple_strtoul(buffer,NULL,0);

	return size;
}

static ssize_t	b3_show_buzzer(struct device *dev, struct device_attribute *attr, char *buffer)
{
	ssize_t len = 0;

	len = sprintf (buffer+len, "%u", b3_data.buzz);

	return len;
}

static ssize_t b3_store_buzzer(struct device *dev, struct device_attribute *attr,const char *buffer, size_t size)
{

	b3_data.buzz = simple_strtoul(buffer,NULL,0);

	b3_data.buzz = (b3_data.buzz>0) ? BUZZ_ON : BUZZ_OFF;

	if(b3_data.buzz==BUZZ_ON){
		b3_buzz_on();
	}else{
		b3_buzz_off();
	}

	return size;
}

static ssize_t	b3_show_color(struct device *dev, struct device_attribute *attr, char *buffer)
{
	ssize_t len = 0;

	len = sprintf (buffer+len, "%u", b3_data.color);

	return len;
}

static ssize_t b3_store_color(struct device *dev, struct device_attribute *attr,const char *buffer, size_t size)
{

	b3_data.color = simple_strtoul(buffer,NULL,0);

	b3_data.color = (b3_data.color>LED_BOOT) ? LED_BOOT : b3_data.color;

	b3_led_color(b3_data.color);

	return size;
}

static struct gpio bubba_gpios[] = {
#ifdef CONFIG_BUBBA3_INSTALL
	{ B3_FRONT_LED_RED, GPIOF_OUT_INIT_LOW, "Red LED"},
	{ B3_FRONT_LED_BLUE, GPIOF_OUT_INIT_LOW, "Blue LED"},
	{ B3_FRONT_LED_GREEN, GPIOF_OUT_INIT_HIGH, "Green LED"},
#else
	{ B3_FRONT_LED_RED, GPIOF_OUT_INIT_HIGH, "Red LED"},
	{ B3_FRONT_LED_BLUE, GPIOF_OUT_INIT_HIGH, "Blue LED"},
	{ B3_FRONT_LED_GREEN, GPIOF_OUT_INIT_LOW, "Green LED"},
#endif
	{ B3_LED_INTERVAL, GPIOF_OUT_INIT_HIGH, "LED interval"},
	{ B3_BUZZER_ENABLE, GPIOF_OUT_INIT_LOW, "Buzzer"}
};

static int request_ioresources(void)
{

	if(gpio_request_array(bubba_gpios, ARRAY_SIZE(bubba_gpios))<0){
		return -EINVAL;
	}

	return 0;
}

DEVICE_ATTR(ledmode, 0644, b3_show_ledmode, b3_store_ledmode);
DEVICE_ATTR(ledfreq, 0644, b3_show_ledfreq, b3_store_ledfreq);
DEVICE_ATTR(buzzer, 0644, b3_show_buzzer, b3_store_buzzer);
DEVICE_ATTR(color, 0644, b3_show_color, b3_store_color);

static int b3_probe(struct platform_device *dev)
{
	int ret=0;

	if(request_ioresources()){
		return -EINVAL;
	}

	ret = device_create_file(&b3_device->dev, &dev_attr_ledmode);
	if(ret){
		return -EINVAL;
	}

	ret = device_create_file(&b3_device->dev, &dev_attr_ledfreq);
	if(ret){
		device_remove_file(&b3_device->dev, &dev_attr_ledmode);
		return -EINVAL;
	}

	ret = device_create_file(&b3_device->dev, &dev_attr_buzzer);
	if(ret){
		device_remove_file(&b3_device->dev, &dev_attr_ledfreq);
		device_remove_file(&b3_device->dev, &dev_attr_ledmode);
		return -EINVAL;
	}

	ret = device_create_file(&b3_device->dev, &dev_attr_color);
	if(ret){
		device_remove_file(&b3_device->dev, &dev_attr_buzzer);
		device_remove_file(&b3_device->dev, &dev_attr_ledfreq);
		device_remove_file(&b3_device->dev, &dev_attr_ledmode);
		return -EINVAL;
	}

	b3_data.mode = MODE_LIT;
	b3_data.freq = LED_DEFAULT_FREQ;
	b3_data.buzz = BUZZ_OFF;
#ifdef CONFIG_BUBBA3_INSTALL
	b3_data.color = LED_INSTALL;
#else
	b3_data.color = LED_BOOT;
#endif
	return ret;
}

static int b3_remove(struct platform_device *dev)
{

	device_remove_file (&b3_device->dev, &dev_attr_ledmode);
	device_remove_file (&b3_device->dev, &dev_attr_ledfreq);
	device_remove_file(&b3_device->dev, &dev_attr_buzzer);
	device_remove_file(&b3_device->dev, &dev_attr_color);

	gpio_free_array(bubba_gpios, ARRAY_SIZE(bubba_gpios));
	return 0;
}

static int __init bubba3_init(void){
        int result;

        if(!machine_is_bubba3()){
                return -EINVAL;
        }
 

        result = platform_driver_register(&b3_driver);
        if (result < 0) {
                printk(KERN_ERR "bubba3: Failed to register driver\n");
                return result;
        }

		b3_device = platform_device_alloc(DEVNAME,-1);
		platform_device_add(b3_device);

		printk(KERN_INFO "BUBBA3: driver ver %s (build %s) loaded\n",ver,build);

        return result;

}

static void __exit bubba3_cleanup(void){

        platform_device_del(b3_device);
        platform_driver_unregister(&b3_driver);

        printk(KERN_INFO "bubba3 driver removed\n");

}
/* register init and cleanup functions */
module_init(bubba3_init);
module_exit(bubba3_cleanup);
