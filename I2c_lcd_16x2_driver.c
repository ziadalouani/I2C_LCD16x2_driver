#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#define I2C_DEVICE_SLAVE_ADDR 0x27 // to get from device-tree laster
#define SLAVE_DEVICE_NAME ("ZIAD_LCD") // to get from device-tree later
#define I2C_BUS_NUMBER 1 // to get from device-tree later improvement later

static int lcd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int lcd_remove (struct i2c_client * client) ;

static struct i2c_adapter * i2c_adapter = NULL;    // I2C Adapter Structure
static struct i2c_client * i2c_client_oled = NULL; // I2C Cient Structure 

static struct i2c_board_info lcd_i2c_board_info = { I2C_BOARD_INFO(SLAVE_DEVICE_NAME, I2C_DEVICE_SLAVE_ADDR) };
static const struct i2c_device_id lcd_id[] = {
    {SLAVE_DEVICE_NAME, 0},
    {}    
};

static struct i2c_driver lcd_driver_behaviour = {
    .driver = {
    .name = SLAVE_DEVICE_NAME,
    .owner = THIS_MODULE,
    },
    .probe = lcd_probe,
    .remove = lcd_remove,
    .id_table = lcd_id,
};

static int lcd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    printk("Hi Device Probed \n ! ");
    return 0;
}
static int lcd_remove (struct i2c_client * client) {
    printk("Goodbye ! \n");
    return 0;
}

static int __init module_insert(void){
    int ret = -1;
    i2c_adapter = i2c_get_adapter(I2C_BUS_NUMBER);
    if (i2c_adapter)
    {
        i2c_client_oled = i2c_new_device(i2c_adapter, &lcd_i2c_board_info);
        if (i2c_client_oled){

             i2c_add_driver(&lcd_driver_behaviour);
             ret = 0;
        }
        i2c_put_adapter(i2c_adapter);
    }
    pr_info("Driver Added!!!\n");
    return 0;
}

static void __exit remove_module(void){
  i2c_unregister_device(i2c_client_oled);
  i2c_del_driver(&lcd_driver_behaviour);
  pr_info("Driver Removed!!!\n");
}


module_init(module_insert);
module_exit(remove_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ziad Al");
MODULE_DESCRIPTION("Simple I2C Device driver for 16x2 LCD Screen");
MODULE_VERSION("1.00");

