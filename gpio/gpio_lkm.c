/*
* gpio_lkm_driver.c - GPIO Loadable Kernel Module
* Implementation - Linux device driver for Raspberry Pi
* Author: Roman Okhrimenko <mrromanjoe@gmail.com>
* Version: 1.0
* License: GPL
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#define USE_GPIOS_NUM 14
#define MAX_GPIO_NUMBER 27
#define DEVICE_NAME "gpio_lkm"
#define BUF_SIZE 512
#define NUM_COM 4

/* set of supported commands */
const char * commands[NUM_COM] = {"out", "in", "low", "high"};
enum commands { set_out  = 0,
                set_in   = 1,
                set_low  = 2,
                set_high = 3,
                na       = NUM_COM+1};

enum direction {in, out};
enum state {low, high};

/*
* struct gpio_lkm_dev - Per gpio pin data structure
* @cdev: instance of struct cdev
* @pin: instance of struct gpio
* @state: logic state (low, high) of a GPIO pin
* @dir: direction of a GPIO pin
*/

struct gpio_lkm_dev
{
    struct cdev cdev;
    struct gpio pin;
    enum state state;
    enum direction dir;
};

/* entry points to file_operations sctruct of char device */
static int gpio_lkm_open(struct inode *inode, struct file *filp);
static int gpio_lkm_release(struct inode *inode, struct file *filp);
static ssize_t gpio_lkm_read (struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t gpio_lkm_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos);

/* tie declarations with interface */
static struct file_operations gpio_lkm_fops =
{
    .owner = THIS_MODULE,
    .open = gpio_lkm_open,
    .release = gpio_lkm_release,
    .read = gpio_lkm_read,
    .write = gpio_lkm_write,
};

/* init and exit functions - must be implemented in each module */
static int gpio_lkm_init(void);
static void gpio_lkm_exit(void);

/* Global varibles for GPIO driver */
struct gpio_lkm_dev *gpio_lkm_devp[USE_GPIOS_NUM];
static dev_t first;
static struct class *gpio_lkm_class;

/*
* which_command - decode string command to 
* coresponding enum value
*/
static unsigned int which_command(const char * com)
{
    unsigned int i;

    for(i = 0 ; i < NUM_COM; i++)
    {
        if(!strcmp(com, commands[i]))
            return i;
    }
    return na;
}

/*
* gpio_lkm_open - Open GPIO device node in /dev
*
* This function allocates GPIO interrupt resource when requested
* on the condition that interrupt flag is enabled and pin direction
* set to input, then allow the specified GPIO pin to set interrupt.
*/
static int gpio_lkm_open (struct inode *inode, struct file *filp)
{
    struct gpio_lkm_dev *gpio_lkm_devp;
    unsigned int gpio;
    gpio = iminor(inode);

    printk(KERN_INFO "[GPIO-LKM] - GPIO[%d] opened\n", gpio);
    gpio_lkm_devp = container_of(inode->i_cdev, struct gpio_lkm_dev, cdev);
    filp->private_data = gpio_lkm_devp;
    
    return 0;
}

/*
* gpio_lkm_release - Release GPIO pin
*
* - releases GPIO resource when the device is closed
*/

static int gpio_lkm_release (struct inode *inode, struct file *filp)
{
    unsigned int gpio;
    struct gpio_lkm_dev *gpio_lkm_devp;
    gpio_lkm_devp = container_of(inode->i_cdev, struct gpio_lkm_dev, cdev);
    gpio = iminor(inode);

    printk(KERN_INFO "[GPIO-LKM] - Closing GPIO %d\n", gpio);

    return 0;
}

/*
* gpio_lkm_read - Read the state of GPIO pins
*
* - allows reading levels of GPIO device
*/
static ssize_t gpio_lkm_read ( struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    unsigned int gpio;
    ssize_t retval;
    char byte;

    gpio = iminor(filp->f_path.dentry->d_inode);

    for (retval = 0; retval < count; ++retval)
    {
        byte = '0' + gpio_get_value(gpio);
    
        if(put_user(byte, buf+retval))
            break;
    }
    
    return retval;
}

/*
* gpio_lkm_write - Write to GPIO pin
*
* - implements setting GPIO pin direction and level
*/
static ssize_t gpio_lkm_write ( struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    unsigned int gpio, len = 0;
    char kbuf[BUF_SIZE];
    struct gpio_lkm_dev *gpio_lkm_devp = filp->private_data;

    gpio = iminor(filp->f_path.dentry->d_inode);

    len = count < BUF_SIZE ? count-1 : BUF_SIZE-1;

    if(raw_copy_from_user(kbuf, buf, len) != 0)
        return -EFAULT;

    kbuf[len] = '\0';

    printk(KERN_INFO "[GPIO_LKM] - Got request from user: %s\n", kbuf);

    switch(which_command(kbuf))
    {
    case set_in:
    {
        if (gpio_lkm_devp->dir != in)
        {
            printk(KERN_INFO "[GPIO_LKM] - Set GPIO%d direction: input\n", gpio);
            /* Set direction input */
            gpio_direction_input(gpio);
            /* Mark state in device struct */
            gpio_lkm_devp->dir = in;
        }
        break;
    }
    case set_out:
    {
        if (gpio_lkm_devp->dir != out)
        {
            printk(KERN_INFO " Set GPIO%d direction: ouput\n", gpio);
            /* Set direction output and low level */
            gpio_direction_output(gpio, low);
            /* Mark state in device struct */
            gpio_lkm_devp->dir = out;
            gpio_lkm_devp->state = low;
        }
        break;
    }
    case set_high:
    {
        if (gpio_lkm_devp->dir == in) 
        {
            printk("[GPIO_LKM] - Cannot set GPIO %d, direction: input\n", gpio);
            return -EPERM;
        }
        else
        {
            printk("[GPIO_LKM] - got to set_high\n");
            gpio_set_value(gpio, high);
            gpio_lkm_devp->state = high;
        }
        break;
    }
    case set_low:
        if (gpio_lkm_devp->dir == in) 
        {
            printk("[GPIO_LKM] - Cannot set GPIO %d, direction: input\n", gpio);
            return -EPERM;
        }
        else
        {
            gpio_set_value(gpio, low);
            gpio_lkm_devp->state = low;
        }
        break;
    default:
            printk(KERN_ERR "[GPIO_LKM] - Invalid input value\n");
            return -EINVAL;
        break;

    }

    *f_pos += count;
    return count;
}

/*
* gpio_lkm_init - Initialize GPIO device driver
*
* - dynamically register a character device major
* - create "GPIO" class
* - claim GPIO resource
* - initialize the per-device data structure gpio_lkm_dev
* - register character device to the kernel
* - create device nodes to expose GPIO resource
*/
static int __init gpio_lkm_init(void)
{
    int i, ret, index = 0;

    if (alloc_chrdev_region(&first, 0, USE_GPIOS_NUM, DEVICE_NAME) < 0)
    {
        printk(KERN_DEBUG "Cannot register device\n");
        return -1;
    }

    if ((gpio_lkm_class = class_create( THIS_MODULE, DEVICE_NAME)) == NULL)
    {
        printk(KERN_DEBUG "Cannot create class %s\n", DEVICE_NAME);
        unregister_chrdev_region(first, USE_GPIOS_NUM);

        return -EINVAL;
    }

    for (i = 0; i <= MAX_GPIO_NUMBER; i++)
    {
        if ( i == 4 || i == 17 || i == 18 || i == 27 ||
            i == 22 || i == 23 || i == 24 || i == 25 ||
            i == 5 || i == 6 || i == 13 || i == 12 || 
            i == 16 || i == 26)
        {
            gpio_lkm_devp[index] = kmalloc(sizeof(struct gpio_lkm_dev), GFP_KERNEL);

            if (!gpio_lkm_devp[index])
            {
                printk(KERN_DEBUG "[GPIO_LKM]Bad kmalloc\n");
                return -ENOMEM;
            }

            if (gpio_request_one(i, GPIOF_OUT_INIT_LOW, NULL) < 0)
            {
                printk(KERN_ALERT "[GPIO_LKM] - Error requesting GPIO %d\n", i);
                return -ENODEV;
            }

            gpio_lkm_devp[index]->dir = out;
            gpio_lkm_devp[index]->state = low;
            gpio_lkm_devp[index]->cdev.owner = THIS_MODULE;

            cdev_init(&gpio_lkm_devp[index]->cdev, &gpio_lkm_fops);

            if ((ret = cdev_add( &gpio_lkm_devp[index]->cdev, (first + i), 1)))
            {
                printk (KERN_ALERT "[GPIO_LKM] - Error %d adding cdev\n", ret);

                for (i = 0; i <= MAX_GPIO_NUMBER; i++)
                {
                        if ( i == 4 || i == 17 || i == 18 || i == 27 ||
                            i == 22 || i == 23 || i == 24 || i == 25 ||
                            i == 5 || i == 6 || i == 13 || i == 12 || 
                            i == 16 || i == 26)
                        {
                            device_destroy (gpio_lkm_class,
                            MKDEV(MAJOR(first),
                            MINOR(first) + i));
                        }

                }
                
                class_destroy(gpio_lkm_class);
                unregister_chrdev_region(first, USE_GPIOS_NUM);
                return ret;
            }

            if (device_create( gpio_lkm_class,
                                NULL, MKDEV(MAJOR(first),
                                MINOR(first)+i),
                                NULL,
                                "GPIO%d",
                                i) == NULL)
            {
            
                class_destroy(gpio_lkm_class);
                unregister_chrdev_region(first, USE_GPIOS_NUM);
            
                return -1;
            }

            index++;
        }
    }

    printk("[GPIO_LKM] - Driver initialized\n");
    
    return 0;
}

/*
* gpio_lkm_exit - Clean up GPIO device driver when unloaded
*
* - release major number
* - release device nodes in /dev
* - release per-device structure arrays
* - detroy class in /sys
* - set all GPIO pins to output, low level
*/

static void __exit gpio_lkm_exit(void)
{
    int i = 0;

    unregister_chrdev_region(first, USE_GPIOS_NUM);
    for (i = 0; i < USE_GPIOS_NUM; i++)
        kfree(gpio_lkm_devp[i]);

    for (i = 0; i <= MAX_GPIO_NUMBER; i++)
    {
        if ( i == 4 || i == 17 || i == 18 || i == 27 ||
            i == 22 || i == 23 || i == 24 || i == 25 ||
            i == 5 || i == 6 || i == 13 || i == 12 || 
            i == 16 || i == 26)
        {
            gpio_direction_output(i, 0);
            device_destroy ( gpio_lkm_class, MKDEV(MAJOR(first), MINOR(first) + i));
            gpio_free(i);
        }
    }

    class_destroy(gpio_lkm_class);
    printk(KERN_INFO "[GPIO_LKM] - Raspberry Pi GPIO driver removed\n");
}

module_init(gpio_lkm_init);
module_exit(gpio_lkm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Okhrimenko <mrromanjoe@gmail.com>");
MODULE_DESCRIPTION("GPIO Loadable Kernel Module - Linux device driver for Raspberry Pi");