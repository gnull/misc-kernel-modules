#include <linux/module.h>
#include <linux/printk.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include <linux/delay.h>

#define pr(fmt, ...) pr_alert("%s: " fmt "\n", THIS_MODULE->name, ##__VA_ARGS__)

int gpio;

static int interraptor_probe(struct platform_device *pdev)
{
	int err;
	int i;

	pr("probe");

	gpio = of_get_named_gpio(pdev->dev.of_node, "sfp-gpios", 0);
	BUG_ON(gpio < 0);

	pr("got gpio %d", gpio);

	err = gpio_request(gpio, "sfp-port-a");
	BUG_ON(err);

	err = gpio_direction_input(gpio);
	BUG_ON(err);

	for (i = 0; i < 100; ++i) {
		int value;

		value = gpio_get_value_cansleep(gpio);
		BUG_ON(value < 0);

		pr("gpio_%d = %d", gpio, value);
		mdelay(500);
	}

	return 0;
}


static int interraptor_remove(struct platform_device *pdev)
{
	pr("remove");

	gpio_free(gpio);

	return 0;
}

static const struct of_device_id interraptor_of_match[] = {
	{ .compatible = "some-fancy-prefix,interraptor" },
	{}
};
MODULE_DEVICE_TABLE(of, interraptor_of_match);

static struct platform_driver interraptor_drv = {
	.probe = interraptor_probe,
	.remove = interraptor_remove,
	.driver = {
		.name = "gpio-interraptor",
		.of_match_table = interraptor_of_match,
	}
};
module_platform_driver(interraptor_drv);

MODULE_AUTHOR("Ivan Oleynikov");
MODULE_LICENSE("GPL");
