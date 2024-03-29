#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/uaccess.h>              
#include <linux/err.h>


#define I2C_DEVICE_SLAVE_ADDR 0x27 // to get from device-tree later
#define SLAVE_DEVICE_NAME ("ZIAD_LCD") // to get from device-tree later
#define I2C_BUS_NUMBER 1 // to get from device-tree later improvement later
#define En 0b00000100
#define Rw 0b00000010
#define Rs 0b00000001
#define LCD_BACKLIGHT 0x08
#define CMD_REG 0
#define DATA_REG 1
#define LCD_FUNCTIONSET 0x20
#define LCD_2LINE   0x08
#define LCD_5x8DOTS 0x00
#define LCD_4BITMODE 0x00
#define LCD_DISPLAYCONTROL 0x08
#define LCD_DISPLAYON 0x04
#define LCD_CLEARDISPLAY 0x01
#define LCD_ENTRYMODESET 0x04
#define LCD_ENTRYLEFT  0x02

static int lcd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int lcd_remove (struct i2c_client * client) ;

static int lcd_init(void);
static void lcd_cmd_write(char cmd);
static void lcd_data_write(char cmd);


static struct i2c_adapter * i2c_adapter = NULL;    // I2C Adapter Structure
static struct i2c_client * i2c_client_lcd = NULL; // I2C Cient Structure 
// --------------------------
dev_t dev = 0;
static struct class * dev_class;
static struct cdev cdev;
uint8_t * kernel_buffer;
static int      f_open(struct inode *inode, struct file *file);
static int      f_release(struct inode *inode, struct file *file);
static ssize_t  f_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  f_write(struct file *filp, const char *buf, size_t len, loff_t * off);
#define mem_size        1024           //Memory Size

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = f_read,
        .write          = f_write,
        .open           = f_open,
        .release        = f_release,
};
// -----------------------------------------------
static struct i2c_board_info lcd_i2c_board_info = { I2C_BOARD_INFO(SLAVE_DEVICE_NAME, I2C_DEVICE_SLAVE_ADDR) };
static const struct i2c_device_id lcd_id[] = {
    {SLAVE_DEVICE_NAME, 0},
    {}    
};
MODULE_DEVICE_TABLE(i2c, lcd_id);

static struct i2c_driver lcd_driver_behaviour = {
    .driver = {
    .name = SLAVE_DEVICE_NAME,
    .owner = THIS_MODULE,
    },
    .probe = lcd_probe,
    .remove = lcd_remove,
    .id_table = lcd_id,
};
////
static int f_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}


static int f_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}


static ssize_t f_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        //Copy the data from the kernel space to the user-space

        return 0;
}

static ssize_t f_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        //Copy the data to kernel space from the user-space
        // this is scary for now i will used it can i send any size? if so this means i can load anything into kernel space ????? 
        int i=0;
        if( copy_from_user(kernel_buffer, buf, len) )
        {
                pr_err("Data Write : Err!\n");
        }
        pr_info("Data Write : Done!\n");
        lcd_cmd_write(LCD_CLEARDISPLAY);
        while(kernel_buffer[i] != '\0' && i < len){
           lcd_data_write(kernel_buffer[i]);
           i++;
        }
        return len;
}
///
static void lcd_write_4_bit_mode(char data){
 // We need to send data twice with enable high then low
 char buf =  data | En | LCD_BACKLIGHT;
 i2c_master_send(i2c_client_lcd,&buf, 1);
 msleep(100);
 buf=((data & ~En) | LCD_BACKLIGHT);
 i2c_master_send(i2c_client_lcd,&buf, 1);
 msleep(100);
}

static void lcd_cmd_write(char cmd){
    // send first half
    lcd_write_4_bit_mode(CMD_REG | LCD_BACKLIGHT | (cmd & 0xF0));
    lcd_write_4_bit_mode(CMD_REG | LCD_BACKLIGHT | ((cmd << 4) & 0xF0));
}
static void lcd_data_write(char cmd){
    lcd_write_4_bit_mode( DATA_REG | LCD_BACKLIGHT |  (cmd & 0xF0));
    lcd_write_4_bit_mode( DATA_REG | LCD_BACKLIGHT |  ((cmd << 4) & 0xF0));
}


static int lcd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    printk("Hi Device Probed \n ! ");
    lcd_init();
    return 0;
}
static int lcd_remove (struct i2c_client * client) {
    printk("Goodbye ! \n");
    return 0;
}


static int lcd_init(void){
 // Initialize Lcd in 4-bit mode
 lcd_cmd_write(0X3);
 lcd_cmd_write(0X3);
 lcd_cmd_write(0X3);
 lcd_cmd_write(0X2);
 // enable 5x7 mode for chars 
 lcd_cmd_write(LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
 lcd_cmd_write(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
 lcd_cmd_write(LCD_CLEARDISPLAY);
 lcd_cmd_write(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
 msleep(100);
 lcd_cmd_write(0x80);

 return 0;   
}

static int __init module_insert(void){
    int ret = -1;
    i2c_adapter = i2c_get_adapter(I2C_BUS_NUMBER);
    if (i2c_adapter)
    {
        i2c_client_lcd = i2c_new_device(i2c_adapter, &lcd_i2c_board_info);
        if (i2c_client_lcd){

             i2c_add_driver(&lcd_driver_behaviour);
             ret = 0;
        }
        i2c_put_adapter(i2c_adapter);
    }
    pr_info("Driver Added!!!\n");


    /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "lcd_screen")) <0){
                pr_info("Cannot allocate major number\n");
                return -1;
        }
 
        /*Creating cdev structure*/
        cdev_init(&cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&cdev,dev,1)) < 0){
            pr_info("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if(IS_ERR(dev_class = class_create(THIS_MODULE,"class"))){
            pr_info("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"lcd_screen"))){
            pr_info("Cannot create the Device 1\n");
            goto r_device;
        }
        
        /*Creating Physical memory*/
        if((kernel_buffer = kmalloc(mem_size , GFP_KERNEL)) == 0){
            pr_info("Cannot allocate memory in kernel\n");
            goto r_device;
        }
        
        strcpy(kernel_buffer, "Hello_World");
        
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;
        r_device:
        class_destroy(dev_class);
        r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

static void __exit remove_module(void){
  i2c_unregister_device(i2c_client_lcd);
  i2c_del_driver(&lcd_driver_behaviour);
  pr_info("Driver Removed!!!\n");
  kfree(kernel_buffer);
  device_destroy(dev_class,dev);
  class_destroy(dev_class);
  cdev_del(&cdev);
  unregister_chrdev_region(dev, 1);
}

module_init(module_insert);
module_exit(remove_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ziad Al");
MODULE_DESCRIPTION("Simple I2C Device driver for 16x2 LCD Screen");
MODULE_VERSION("1.02");

