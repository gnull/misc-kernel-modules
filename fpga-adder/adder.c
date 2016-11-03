#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/device.h>

#include <linux/kdev_t.h>

static struct class *adder_class;

struct adder_priv {
	int a, b;
	struct device *dev;
};

static ssize_t a_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int res;
	int err;
	struct adder_priv *priv;

	priv = dev_get_drvdata(dev);

	err = kstrtoint(buf, 0, &res);
	if (err) {
		dev_err(dev, "Wrong number format");
		return err;
	}

	priv->a = res;
	dev_info(dev, "a = %d", priv->a);

	return count;
}
DEVICE_ATTR_WO(a);

static ssize_t b_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int res;
	int err;
	struct adder_priv *priv;

	priv = dev_get_drvdata(dev);

	err = kstrtoint(buf, 0, &res);
	if (err) {
		dev_err(dev, "Wrong number format");
		return err;
	}

	priv->b = res;
	dev_info(dev, "b = %d", priv->b);

	return count;
}
DEVICE_ATTR_WO(b);

static ssize_t sum_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct adder_priv *priv;

	priv = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", priv->a + priv->b);
}
DEVICE_ATTR_RO(sum);

static const struct attribute *adder_attrs[] = {
	&dev_attr_a.attr,
	&dev_attr_b.attr,
	&dev_attr_sum.attr,
	NULL
};

static const struct attribute_group adder_attr_group = {
	.attrs = (struct attribute **) adder_attrs,
};

static int adder_probe(struct platform_device *pdev)
{
	int err;
	struct adder_priv *priv;
	struct device *dev;

	dev_info(&pdev->dev, "probing");

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	BUG_ON(!priv);

	dev = device_create(adder_class, &pdev->dev, MKDEV(0, 0), NULL,
			"yet-another-adder");
	BUG_ON(IS_ERR(priv->dev));

	priv->dev = dev;
	dev_set_drvdata(dev, priv);
	platform_set_drvdata(pdev, priv);

	dev_info(dev, "device created");

	err = sysfs_create_group(&dev->kobj, &adder_attr_group);
	BUG_ON(err);

	dev_info(dev, "files created");

	return 0;
}

static int adder_remove(struct platform_device *pdev)
{
	struct adder_priv *priv;

	priv = platform_get_drvdata(pdev);

	device_unregister(priv->dev);

	dev_info(&pdev->dev, "device removed");

	return 0;
}

static const struct of_device_id adder_id_table[] = {
	{.compatible = "linux-piter,adder"},
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

static int adder_init(void)
{
	int err;

	adder_class = class_create(THIS_MODULE, "adder");
	BUG_ON(IS_ERR(adder_class));
	pr_info("class created\n");

	err = platform_driver_register(&adder_drv);
	BUG_ON(err);

	return 0;
}
module_init(adder_init);

static void adder_exit(void)
{
	platform_driver_unregister(&adder_drv);

	class_destroy(adder_class);
	pr_info("class destroyed\n");
}
module_exit(adder_exit);

MODULE_AUTHOR("Ivan Oleynikov");
MODULE_LICENSE("GPL");
