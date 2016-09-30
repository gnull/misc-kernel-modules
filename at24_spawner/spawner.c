#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/i2c.h>
#include <linux/platform_data/at24.h>

static struct i2c_client *client = NULL;

static void setup(struct memory_accessor *macc, void *context)
{
	char buf[257] = {};
	ssize_t len;

	BUG_ON(!macc->read);
	len = macc->read(macc, buf, 0, 256);
	buf[256] = 0;

	printk("got %u bytes from eeprom\n", (unsigned int) len);
	printk("those bytes are: %s\n", buf);

	printk("setup!\n");
}

/* This function reads standard i2c device properties is the same way as
 * function of_i2c_register_devices in drivers/i2c/i2c-core.c */
static void spawn_eeprom(struct device_node *self) {
	int err;
	u32 reg;
	struct device_node *adapter_node;
	struct i2c_adapter *adapter;

	struct at24_platform_data platdata = {
		/* Taken from `at24_ids[]` in drivers/misc/eeprom/at24.c */
		.byte_len = 256,
		/* The value 1 is not the fastest but is guaranteed to work */
		.page_size = 1,
		.flags = AT24_FLAG_READONLY,
		.setup = setup,
	};

	struct i2c_board_info info = {
		.of_node = of_node_get(self),
		.platform_data = &platdata,
		/* .reg and .type are filled below */
	};


	/* save modalias to info.type */
	err = of_modalias_node(self, info.type, sizeof(info.type));
	BUG_ON(err);
	printk("found modalias = %s\n", info.type);

	/* next read reg property */
	err = of_property_read_u32_array(self, "reg", &reg, 1);
	BUG_ON(err);
	BUG_ON(reg > (1 << 10) - 1);
	info.addr = reg;
	printk("found reg = %x casted as %hx\n", reg, info.addr);

	adapter_node = of_get_parent(self);
	BUG_ON(!adapter_node);

	printk("EEPROM node %s has parent I2C bus node %s\n",
		self->name, adapter_node->name);

	adapter = of_find_i2c_adapter_by_node(adapter_node);
	of_node_put(adapter_node);
	BUG_ON(!adapter);

	printk("found i2c adapter named as %s\n", adapter->name);

	/* This request_module is done for the same reasons as in
	 * of_i2c_register_devices function. For details see the commit
	 * 020862648445d7c1b12ea213c152f27def703f3b upstream.*/
	request_module("%s%s", I2C_MODULE_PREFIX, info.type);

	client = i2c_new_device(adapter, &info);
	BUG_ON(!client);

	printk("successfully registered i2c client named %s\n",
		client->name);
}

static void remove_eeprom(void)
{
	i2c_unregister_device(client);
}

static int spawner_probe(struct platform_device *pdev)
{
	int err;
	struct device_node *eeprom;
	phandle eeprom_h;

	err = of_property_read_u32(pdev->dev.of_node, "eeprom-device",
				&eeprom_h);
	BUG_ON(err);

	eeprom = of_find_node_by_phandle(eeprom_h);
	BUG_ON(!eeprom);

	spawn_eeprom(eeprom);

	printk("probed");
	return 0;
}

static int spawner_remove(struct platform_device *pdev)
{
	remove_eeprom();
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
