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

/*disclaimer: not all of Raspberry pins
 * are available to be used as GPIOs */
#define USE_GPIOS_NUM 14   /* number of GPIOs that we are going to allocate */
#define MAX_GPIO_NUMBER 27 /* maximum number of GPIO available for allocation */
#define DEVICE_NAME "gpio_lkm" /* name that will be assigned to this device in /dev fs */
#define BUF_SIZE 512
#define NUM_COM 4 /* number of commands that this driver support */

/* buffer with set of supported commands */
const char * commands[NUM_COM] = {"out", "in", "low", "high"};
/* enumerators to match commands with values for following processing */
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
    /* declare struct cdev that will represent
     * our char device in inode structure. 
     * inode is used by kernel to represent file objects */
    struct cdev cdev; 
    /* declare pin of struct gpio type,
     * provided by include/linux/gpio.h */
    struct gpio pin;
    enum state state;
    enum direction dir;
};

/* to implement a char device driver we need to satisfy some
 * requirements. one of them is an implementation of mandatory
 * methods defined in struct file_operations
 *  - read
 *  - write
 *  - open
 *  - release
 * think about device as about a simple file. these are basic
 * operations you do with all regular files. same for char device
 * signatures of functions defined in linux/include/fs.h may be
 * called a virtual methods in OOP terminology, that you need to
 * implemt. all these are repsented as callback functions
 */
static int gpio_lkm_open(struct inode *inode, struct file *filp);
static int gpio_lkm_release(struct inode *inode, struct file *filp);
static ssize_t gpio_lkm_read (struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t gpio_lkm_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos);

/* declare structure gpio_lkm_fops which holds 
 * our implementations of callback functions,
 * in instance of struct file_operations for our
 * char device driver
 */
static struct file_operations gpio_lkm_fops =
{
    .owner = THIS_MODULE,
    .open = gpio_lkm_open,
    .release = gpio_lkm_release,
    .read = gpio_lkm_read,
    .write = gpio_lkm_write,
};

/* declare prototypes of init and exit functions.
 * implementation of these 2 functions is mandatory
 * for each linux kernel module. they serve to
 * satisfy kernel request to initialize module
 * when you insmod'ing it in system and release
 * allocated resources when you prform rmmod command */
static int gpio_lkm_init(void);
static void gpio_lkm_exit(void);

/* declare an array of gpio_lkm_dev device structure objects
 * which represent each of our pins as a char device */
struct gpio_lkm_dev *gpio_lkm_devp[USE_GPIOS_NUM];
/* */
static dev_t first;
/* declare pointer to our device class. this will
 * be used to satisfy udev kernel service requirements.
 * udev service automatically creates devices in /dev
 * filesystem and populates /sysfs filesystem with
 * values provided by drivers when calling corresponding APIs
 */
static struct class *gpio_lkm_class;

/*
* which_command - used to decode string reprecentations of
* command received from user space request via write request.
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

/* comprehensive reading about read/write/open/release char device methods
*  https://www.oreilly.com/library/view/linux-device-drivers/0596005903/ch03.html
*/

/*
* gpio_lkm_open - Open GPIO device
* this is implementation of previously declared function
* open from our file_operations structure.
* this function will be called each time device recives
* open file operation applied to its name in /dev
* open method call creates struct file instance
*/
static int gpio_lkm_open (struct inode *inode, struct file *filp)
{
    struct gpio_lkm_dev *gpio_lkm_devp;
    unsigned int gpio;

    /* call include/linux/fs.h api to get minor 
     * number from input inode struct */
    gpio = iminor(inode);
    /* print obtained number to system journal */
    printk(KERN_INFO "[GPIO-LKM] - GPIO[%d] opened\n", gpio);
    /* this macro basically tells kernel to match name cdev
     * of type struct gpio_lkm_dev to where first argument points to
     * see struct inode definition in fs.h line 679 and more 
     * info about macro here
     * https://radek.io/2012/11/10/magical-container_of-macro/
     *  */
    gpio_lkm_devp = container_of(inode->i_cdev, struct gpio_lkm_dev, cdev);
    /* assign a pointer to struct representing our 
     * device to its corresponding file object */
    filp->private_data = gpio_lkm_devp;
    
    /* zero returns stand for success in kernel programming */
    return 0;
}

/*
* gpio_lkm_release - Release GPIO device
* this function is called whenever kernel tries to remove driver
* module from system. here all 
*/
static int gpio_lkm_release (struct inode *inode, struct file *filp)
{
    unsigned int gpio;
    
    gpio = iminor(inode);

    /* remove pointer our device data, that was assigned in open 
     * if any resources was allocated they should be dealocated
     * here before return
    */
    filp->private_data = NULL;
    /* print debug message to system journal - good practice */
    printk(KERN_INFO "[GPIO-LKM] - Closing GPIO %d\n", gpio);

    return 0;
}

/*
* gpio_lkm_read - Read method implementation for GPIO device
* this method will be called whenever read command is applied
* to our device in /dev. data that should be provided to user
* space 
*/
static ssize_t gpio_lkm_read ( struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    unsigned int gpio;
    ssize_t retval;
    char byte;

    /* determine which device is read from inode sctructure.
     * remember, we have 14 GPIOs at max and each is effectively
     * separate device using same driver
     */
    gpio = iminor(filp->f_path.dentry->d_inode);

    /* get count amount of values from GPIO device */
    for (retval = 0; retval < count; ++retval)
    {
        /* use kernel gpio API functions to get
         * value of gpio by minor numer of device
         */
        byte = '0' + gpio_get_value(gpio);

        /* use special macro to copy data from kernel space
         * co user space. API related to user space
         * interactions are found in arm/asm/uaccess.h
         */
        if(put_user(byte, buf+retval))
            break;
    }
    
    return retval;
}

/*
* gpio_lkm_write - Write method implementation for GPIO device
* this method will be called whenever write command is applied
* to our device in /dev. data that should be provided to user
* space 
*/
static ssize_t gpio_lkm_write ( struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    unsigned int gpio, len = 0;
    char kbuf[BUF_SIZE];
    struct gpio_lkm_dev *gpio_lkm_devp = filp->private_data;

    /* get gpio device minor number 
    */
    gpio = iminor(filp->f_path.dentry->d_inode);

    len = count < BUF_SIZE ? count-1 : BUF_SIZE-1;

    /* one more special kernel macro to copy data between
     * user space memory and kernel space memory
     */
    if(raw_copy_from_user(kbuf, buf, len) != 0)
        return -EFAULT;

    kbuf[len] = '\0';

    printk(KERN_INFO "[GPIO_LKM] - Got request from user: %s\n", kbuf);

    /* perform a switch on recieved command value
     * to determine, what request is received
     */
    switch(which_command(kbuf))
    {
    case set_in:
    {
        if (gpio_lkm_devp->dir != in)
        {
            printk(KERN_INFO "[GPIO_LKM] - Set GPIO%d direction: input\n", gpio);
            /* set direction input */
            gpio_direction_input(gpio);
            /* store state in device struct */
            gpio_lkm_devp->dir = in;
        }
        break;
    }
    case set_out:
    {
        if (gpio_lkm_devp->dir != out)
        {
            printk(KERN_INFO " Set GPIO%d direction: ouput\n", gpio);
            /* set direction output and low level */
            gpio_direction_output(gpio, low);
            /* store state in device struct */
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
* this function is called each time you call
* modprobe or insmod it should implement all the
* needed actions to prepare device to work
* here is a list:
* - dynamically register a character device major
* - create "GPIO" class in /sysfs
* - allocates resource for GPIO device
* - initialize the per-device data structure gpio_lkm_dev
* - register character device to the kernel
* - create device nodes to expose GPIO resource
*/
static int __init gpio_lkm_init(void)
{
    int i, ret, index = 0;

    /* register a range of char GPIO device numbers
     * here we request to allocate certain minor numbers
     * to correspondent GPIO devices pin numbers
     * more info in fs/char_dev.c:245
     */
    if (alloc_chrdev_region(&first, 0, USE_GPIOS_NUM, DEVICE_NAME) < 0)
    {
        printk(KERN_DEBUG "Cannot register device\n");
        return -1;
    }

    /* call create_class to create a udev class to contain the device
     * folder in also created in /sys/class for this module with name
     * defined in DEVICE_NAME
     */
    if ((gpio_lkm_class = class_create( THIS_MODULE, DEVICE_NAME)) == NULL)
    {
        printk(KERN_DEBUG "Cannot create class %s\n", DEVICE_NAME);
        /* clean up what was done in case of faulure
         */
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
            /* allocate memory for sctuctures to contain each GPIO representation
            */
            gpio_lkm_devp[index] = kmalloc(sizeof(struct gpio_lkm_dev), GFP_KERNEL);

            if (!gpio_lkm_devp[index])
            {
                printk(KERN_DEBUG "[GPIO_LKM]Bad kmalloc\n");
                return -ENOMEM;
            }

            /* call kernel gpio API to request one gpio, pass config flags
             * here gpio requested will support In Out directions and initialized
             * with low level
             */
            if (gpio_request_one(i, GPIOF_OUT_INIT_LOW, NULL) < 0)
            {
                printk(KERN_ALERT "[GPIO_LKM] - Error requesting GPIO %d\n", i);
                return -ENODEV;
            }

            /* store data in device scturture to reference somewhere in module
            */
            gpio_lkm_devp[index]->dir = out;
            gpio_lkm_devp[index]->state = low;
            gpio_lkm_devp[index]->cdev.owner = THIS_MODULE;

            /* itialize cdev structure for our device and match it
             * with file_operations defined for it
             */
            cdev_init(&gpio_lkm_devp[index]->cdev, &gpio_lkm_fops);

            /* try to register a char device to the system 
             * all these cdev functions are implemented in fs/char_dev.c
             */
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
                            /* clean up on failure execution
                             *
                             */
                            device_destroy (gpio_lkm_class,
                            MKDEV(MAJOR(first),
                            MINOR(first) + i));
                        }

                }
                
                /* clean up in opposite way from init
                 */
                class_destroy(gpio_lkm_class);
                unregister_chrdev_region(first, USE_GPIOS_NUM);
                return ret;
            }

            /* creates a device and registers it with sysfs
             * all parameters passed shoud be familiar to this point
             * more info - drivers/base/core.c:2672
             */
            if (device_create( gpio_lkm_class,
                                NULL, MKDEV(MAJOR(first),
                                MINOR(first)+i),
                                NULL,
                                "GPIO%d",
                                i) == NULL)
            {
            
                /* do not forget to clean in case of errors
                 */
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
* gpio_lkm_exit - Deinitialize GPIO device driver when unloaded
* this function is called each time you call
* rmmod. it should implement all the oposite to
* initialization actions to deallocate resources
* used by device, unregister it from system
* here is a list:
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
        /* free up memory used by device structures
         */
        kfree(gpio_lkm_devp[i]);

    for (i = 0; i <= MAX_GPIO_NUMBER; i++)
    {
        if ( i == 4 || i == 17 || i == 18 || i == 27 ||
            i == 22 || i == 23 || i == 24 || i == 25 ||
            i == 5 || i == 6 || i == 13 || i == 12 || 
            i == 16 || i == 26)
        {
            /* set default values on used gpio pins
             */
            gpio_direction_output(i, 0);
            /* destroy device
             */
            device_destroy ( gpio_lkm_class, MKDEV(MAJOR(first), MINOR(first) + i));
            gpio_free(i);
        }
    }
    /* destroy class
     */
    class_destroy(gpio_lkm_class);
    printk(KERN_INFO "[GPIO_LKM] - Raspberry Pi GPIO driver removed\n");
}

/* these are stantard macros to mark
 * init and exit functions implemetations
 */
module_init(gpio_lkm_init);
module_exit(gpio_lkm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Okhrimenko <mrromanjoe@gmail.com>");
MODULE_DESCRIPTION("GPIO Loadable Kernel Module - Linux device driver for Raspberry Pi");
