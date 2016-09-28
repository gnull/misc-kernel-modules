#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/i2c.h>

static struct i2c_client *client = NULL;

void spawn_eeprom(struct i2c_adapter *adapter)
{
	struct i2c_board_info info = {
		.type = "24c02",
		.addr = 0x53,
	};

	client = i2c_new_device(adapter, &info);
	BUG_ON(!client);

	printk("successfully registered i2c client named %s\n",
		client->name);
}

int spawner_probe(struct platform_device *pdev)
{
	int err;
	phandle i2c_bus_h;
	struct device_node *self;
	struct device_node *i2c_bus;
	struct i2c_adapter *adapter;

	self = pdev->dev.of_node;

	err = of_property_read_u32(self, "i2c-adapter", &i2c_bus_h);
	BUG_ON(err);

	i2c_bus = of_find_node_by_phandle(i2c_bus_h);
	BUG_ON(!i2c_bus);

	printk("node %s has i2c-bus property pointing at %s\n",
		self->name, i2c_bus->name);

	adapter = of_find_i2c_adapter_by_node(i2c_bus);
	of_node_put(i2c_bus);
	BUG_ON(!adapter);

	spawn_eeprom(adapter);

	printk("found i2c adapter named as %s\n", adapter->name);

	printk("probed\n");
	return 0;
}

int spawner_remove(struct platform_device *pdev)
{
	printk("removed\n");
	return 0;
}

static const struct of_device_id spawner_id_table[] = {
	{.compatible = "some-fancy-prefix,at24-spawner"},
	{}
};
MODULE_DEVICE_TABLE(of, spawner_id_table);

static struct platform_driver spawner_drv = {
	.remove = spawner_remove,
	.driver = {
		.name = "at24_spawner",
		.of_match_table = spawner_id_table,
	}
};

static int __init spawner_init(void)
{
	return platform_driver_probe(&spawner_drv, spawner_probe);
}

static void __exit spawner_exit(void)
{
	platform_driver_unregister(&spawner_drv);
}

MODULE_AUTHOR("Ivan Oleynikov");
MODULE_LICENSE("GPL");
module_init(spawner_init);
module_exit(spawner_exit);
