#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/printk.h>

#include <linux/kdev_t.h>

static struct class *adder_class;
static struct device *device;

static ssize_t hello_world_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "Hello World!\n");
}
DEVICE_ATTR_RO(hello_world);

static int adder_probe(struct platform_device *pdev)
{
	int err;

	adder_class = class_create(THIS_MODULE, "adder");
	BUG_ON(IS_ERR(adder_class));

	printk("class created\n");

	device = device_create(adder_class, &pdev->dev, MKDEV(0, 0), NULL,
			"yet-another-adder");
	BUG_ON(IS_ERR(device));

	printk("device created\n");

	err = device_create_file(device, &dev_attr_hello_world);
	BUG_ON(err);

	printk("file created\n");

	return 0;
}

static int adder_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_hello_world);

	printk("file removed\n");

	device_unregister(device);

	printk("device removed\n");

	class_destroy(adder_class);

	printk("class destroyed\n");

	return 0;
}

static const struct of_device_id adder_id_table[] = {
	{.compatible = "some-fancy-prefix,adder"},
	{}
};
MODULE_DEVICE_TABLE(of, adder_id_table);

static struct platform_driver adder_drv = {
	.probe  = adder_probe,
	.remove = adder_remove,
	.driver = {
		.name = "adder",
		.of_match_table = adder_id_table,
	}
};
module_platform_driver(adder_drv);

MODULE_AUTHOR("Ivan Oleynikov");
MODULE_LICENSE("GPL");
