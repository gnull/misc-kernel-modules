#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/printk.h>

struct enc_private {
	// TODO
	bool ok;
};

static ssize_t hello_world_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "Hello World!\n");
}
DEVICE_ATTR_RO(hello_world);

static int encryptor_probe(struct platform_device *pdev)
{
	int err;

	err = device_create_file(&pdev->dev, &dev_attr_hello_world);
	BUG_ON(err);

	return 0;
}

static int encryptor_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_hello_world);
	return 0;
}

static const struct of_device_id encryptor_id_table[] = {
	{.compatible = "some-fancy-prefix,lfsr-encryptor"},
	{}
};
MODULE_DEVICE_TABLE(of, encryptor_id_table);

static struct platform_driver encryptor_drv = {
	.probe  = encryptor_probe,
	.remove = encryptor_remove,
	.driver = {
		.name = "at24_encryptor",
		.of_match_table = encryptor_id_table,
	}
};
module_platform_driver(encryptor_drv);

MODULE_AUTHOR("Ivan Oleynikov");
MODULE_LICENSE("GPL");
