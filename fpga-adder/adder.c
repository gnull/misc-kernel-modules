#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/printk.h>

struct adder_private {
	// TODO
	bool ok;
};

static ssize_t hello_world_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "Hello World!\n");
}
DEVICE_ATTR_RO(hello_world);

static int adder_probe(struct platform_device *pdev)
{
	int err;

	err = device_create_file(&pdev->dev, &dev_attr_hello_world);
	BUG_ON(err);

	return 0;
}

static int adder_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_hello_world);
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
