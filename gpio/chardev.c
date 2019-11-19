#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Okhrimenko <mrromanjoe@gmail.com>");
MODULE_DESCRIPTION("Skeleton module for char device creation.");

#define DEVICE_NAME "foo_bar"
#define CHARDEV_MINOR        19   /* start of minor numbers requested */
#define CHARDEV_MINOR_NUM    1    /* how many minors requested */

/**
 * @brief For more detailed comments please see file gpio_lkm.c 
 * 
 */

static dev_t first; /* global variable for the first device number */
static struct cdev c_dev; /* global variable for the character device structure */
static struct class *cd_class; /* global variable for the device class */

/**
 * @brief Define and implement open()
 * 
 */
static int chardev_open(struct inode *i, struct file *f)
{
	printk(KERN_DEBUG "[chardev] - open() method called\n");
	return 0;
}

/**
 * @brief Define and implement release()
 * 
 */
static int chardev_release(struct inode *i, struct file *f)
{
	printk(KERN_DEBUG "[chardev] - close() method called\n");
	return 0;
}

/**
 * @brief Define and implement read()
 * 
 */
static ssize_t chardev_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_DEBUG "[chardev] - read() method called\n");
	return 0;
}

/**
 * @brief Define and implement write()
 * 
 */
static ssize_t chardev_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_DEBUG "[chardev] - write() method called\n");
	return len;
}

/**
 * @brief Match implementation with modules file_options
 * 
 */
static struct file_operations chardev_fops =
{
	.owner = THIS_MODULE,
	.open = chardev_open,
	.release = chardev_release,
	.read = chardev_read,
	.write = chardev_write
};

/**
 * @brief Initialization function
 * 
 */
static int __init chardev_init(void)
{
	int ret;
	struct device *dev_ret;

	printk(KERN_DEBUG "[chardev] - init functions called");
    /* allocate minor numbers */
	if ((ret = alloc_chrdev_region(&first, CHARDEV_MINOR, CHARDEV_MINOR_NUM, DEVICE_NAME)) < 0)
	{
		return ret;
	}
    /* create class for device */
	if (IS_ERR(cd_class = class_create(THIS_MODULE, DEVICE_NAME)))
	{
		unregister_chrdev_region(first, 1);
		return PTR_ERR(cd_class);
	}

    /* create device */
	if (IS_ERR(dev_ret = device_create(cd_class, NULL, first, NULL, "how_you_like_that_ilon_mask")))
	{
		class_destroy(cd_class);
		unregister_chrdev_region(first, 1);
		return PTR_ERR(dev_ret);
	}
    
    /* init cdev sctructure */
	cdev_init(&c_dev, &chardev_fops);
	if ((ret = cdev_add(&c_dev, first, 1)) < 0)
	{
		device_destroy(cd_class, first);
		class_destroy(cd_class);
		unregister_chrdev_region(first, 1);
		return ret;
	}
	return 0;
}

/**
 * @brief Goobye, deallocate and destroy
 * 
 */
static void __exit chardev_exit(void)
{
	cdev_del(&c_dev);
	device_destroy(cd_class, first);
	class_destroy(cd_class);
	unregister_chrdev_region(first, 1);
	printk(KERN_INFO "[chardev] - unregistered from kernel");
}

module_init(chardev_init);
module_exit(chardev_exit);