// SPDX-License-Identifier: GPL-2.0
/*
 * Example USB driver to interact with custom firmware running
 * on STM32F4Discovery development board.
 *
 * Copyright (C) 2017 Roman Okhrimenko <mrromanjoe@gmail.com>
 * 
 * Description on this driver and companion user space app and
 * STM32F4Discovery firmware can be found here
 * <https://github.com/romanjoe/os-course-labs/usb>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kref.h>
#include <asm/uaccess.h>

#define DRIVER_AUTHOR "Roman Okhrimenko <mrromanjoe@gmail.com>"
#define DRIVER_DESC "STM32-Discovery as HID device example"

/* 
 * VID:PID reported by system for STM32F4Discovery board
 */
#define USB_VID 0x0477
#define USB_PID 0x5620

/* 
 * usb_device_id table to pass to usb_driver scructure
 * for device association purpose
 */
static struct usb_device_id stm32leds_table[] =
{
    {USB_DEVICE(USB_VID, USB_PID)},
    {},
};

/*
 * this macro allows user space tools to figure out
 * what devices this driver can control
*/
MODULE_DEVICE_TABLE (usb, stm32leds_table);

/* 
 * reserve a minor number for devices from the usb maintainer
 */
#define STM32LEDS_MINOR_BASE    192

/* 
 * struct of type usb_driver is mandatory
 */
static struct usb_driver stm32leds_driver;

/* 
 * per device structure to contain data 
 */
struct stm32leds
{
    /* internal usb dev for stm32leds*/
    struct usb_device * udev;
    /* interface for it */
    struct usb_interface * interface;
    /* buffer to receive data */
    unsigned char * int_in_buffer;
    /* size of buffer */
    size_t          int_in_size;
    /* addr of interrupt IN endpoint*/
    __u8            int_in_endpointAddr;
    /* addr of interrupt OUT endpoint*/
    __u8            int_out_endpointAddr;
    __u8            int_out_interval;
    __u8            int_in_interval;

    /* save state of leds. */
    unsigned char   red;
    unsigned char   green;
    unsigned char   blue;

    /* locks for syncronization */
    struct mutex    sysfslock;
    spinlock_t      lock;

    /* refference counter */
    struct kref     kref;
};

#define to_stm32leds_dev(d) container_of(d, struct stm32leds, kref)

/* 
 * clean up function to free up memory and ref counters
 */
static void stm32leds_delete(struct kref *kref)
{
    struct stm32leds * dev = to_stm32leds_dev(kref);

    usb_put_dev(dev->udev);
    kfree(dev->int_in_buffer);
    kfree(dev);
}

/* 
 * open file_operations implemetation
 * as like for basic char dev
 */
static int stm32leds_open(struct inode *inode, struct file *file)
{
    struct stm32leds *dev;
    struct usb_interface *interface;
    int subminor;
    int retval = 0;

    subminor = iminor(inode);

    interface = usb_find_interface(&stm32leds_driver, subminor);
    if (!interface) {
        pr_err("[stm32leds] - %s - error, can't find device for minor %d",
             __FUNCTION__, subminor);
        retval = -ENODEV;
        goto exit;
    }

    dev = usb_get_intfdata(interface);

    if (!dev) {
        retval = -ENODEV;
        goto exit;
    }
    
    /* 
     * increment our usage count for the device
     */
    kref_get(&dev->kref);

    /*
     * save object in the file's private structure
     */
    file->private_data = dev;

exit:
    return retval;
}

/*
 * corresponding release function to end up work
 * free up resources put refs
 */
static int stm32leds_release(struct inode *inode, struct file *file)
{
    struct stm32leds *dev;

    /*
     * check if device exist
     */
    dev = (struct stm32leds *)file->private_data;
    if (dev == NULL)
        return -ENODEV;

    /* 
     * decrement the count on our device
     * call clean up function
     */
    kref_put(&dev->kref, stm32leds_delete);

    return 0;
}

/*
 * implementation of file_operations read method
 * the method will be used to return number times
 * button was pressed on STM32F4Discovery board
 */
static ssize_t stm32leds_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    struct stm32leds *dev;
    unsigned long flags;
    int retval;
    char temp[10];

    dev = (struct stm32leds *)file->private_data;

    /*
     * take mutex before reading
    */
    mutex_lock(&dev->sysfslock);
    
    /* 
     * do a blocking bulk read to get data from the USB device
     */
    retval = usb_interrupt_msg(dev->udev,
                  usb_rcvintpipe(dev->udev, dev->int_in_endpointAddr),
                  dev->int_in_buffer,
                  min(dev->int_in_size, count),
                  (int *) &count, HZ * dev->int_in_interval); 
    /*
     * can know relase mutex
     */
    mutex_unlock(&dev->sysfslock);

    /* 
     * protect buffer int_in_buffer from concurency access
     */
    spin_lock_irqsave(&dev->lock, flags);
    /*
     * get data from input buffer
     */
    memcpy(temp, dev->int_in_buffer, 10);
    spin_unlock_irqrestore(&dev->lock, flags);

    /* 
     * if the read was successful, copy the data to userspace
     */
    if (!retval)
    {
        if(raw_copy_to_user(buffer, temp, count))
            retval = -EFAULT;
        else
            retval = count;        
    }

    return retval;
}

/*
 * 
 */
static void stm32leds_write_int_callback(struct urb *urb)
{
    struct stm32leds *dev = urb->context;

    /* sync/async unlink faults aren't errors */
    if (urb->status && 
        !(urb->status == -ENOENT || 
          urb->status == -ECONNRESET ||
          urb->status == -ESHUTDOWN)) {
        dev_dbg(&dev->interface->dev,
            "[stm32leds] - %s - nonzero write bulk status received: %d",
            __FUNCTION__, urb->status);
    }

    /* free up our allocated buffer */
    usb_free_coherent(urb->dev, urb->transfer_buffer_length,
            urb->transfer_buffer, urb->transfer_dma);
}

/*
 * implementation of write function of file_operations
 * method will be called to pass data from user space
 * programm
 */
static ssize_t stm32leds_write(struct file *file,
                            const char __user *user_buffer,
                            size_t count, loff_t *ppos)
{
    struct stm32leds *dev;
    int retval = 0;
    struct urb *urb = NULL;
    char *buf = NULL;

    dev = (struct stm32leds *)file->private_data;

    /* 
     * check if any data provided
     */
    if (count == 0)
        goto exit;

    /* 
     * check if right amount of bytes passed
     */
    if (count != 10)
    {
        dev_dbg(&dev->interface->dev, "[stm32leds] - Wrong bytes number passed in write call\n");
        goto exit;
    }

    /* 
     * create urb and buffer for it, then copy data to transfer to urb
     */
    urb = usb_alloc_urb(0, GFP_KERNEL);
    if(!urb)
    {
        retval = -ENOMEM;
        goto error;
    }

    /* 
     * allocate a dma-consistent buffer for urb
     */
    buf = usb_alloc_coherent(dev->udev, count, GFP_KERNEL, &urb->transfer_dma);
    
    if(!buf)
    {
        dev_dbg(&dev->interface->dev, "[stm32leds] - Error while allocating a dma-consistent buffer for urb\n");
        retval = -ENOMEM;
        goto error;
    }

    if(raw_copy_from_user(buf, user_buffer, count))
    {

        dev_dbg(&dev->interface->dev, "[stm32leds] - Error while copying data from user space\n");
        retval = -EFAULT;
        goto error;
    }

    mutex_lock(&dev->sysfslock);
    /* 
     * configure interrupt URB structure
     * this struct is used by USB clients (this driver) to
     * describe USB request blocks (URBs) that send requests
     * to the USB driver stack
     */
    usb_fill_int_urb(urb, dev->udev,
                    usb_sndintpipe(dev->udev, dev->int_out_endpointAddr),
                    buf, count,
                    stm32leds_write_int_callback,
                    dev, dev->int_out_interval);

    urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    /* 
     * send data out to int port
     */
    retval = usb_submit_urb(urb, GFP_KERNEL);
    
    mutex_unlock(&dev->sysfslock);
    
    if(retval)
    {
        pr_err("[stm32leds] - %s - failed submitting write urb, error %d", __FUNCTION__, retval);
        goto error;
    }

    /* 
     * release reference to used urb, so SUB core can free it up
     */
    usb_free_urb(urb);

exit:
    return retval;
error:
    usb_free_coherent(dev->udev, count, buf, urb->transfer_dma);

    usb_free_urb(urb);
    kfree(buf);
    return retval;
}   

/* 
 * registed standard file_operations struct for char device
 */
static struct file_operations stm32leds_fops = {
    .owner =    THIS_MODULE,
    .read =     stm32leds_read,
    .write =    stm32leds_write,
    .open =     stm32leds_open,
    .release =  stm32leds_release,
};

/* 
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with devfs and the driver core
 */
static struct usb_class_driver stm32leds_class = 
{
    .name = "usb/stm32leds%d",
    .fops = &stm32leds_fops,
    .minor_base = STM32LEDS_MINOR_BASE,
};

/*
 * probe function is called by usb core once it match pluged in
 * device with this driver
 */
static int stm32leds_probe(struct usb_interface *interface, 
                            const struct usb_device_id *id)
{
    struct stm32leds *dev = NULL;
    struct usb_host_interface *iface_desc;
    struct usb_endpoint_descriptor *endpoint;
    size_t buffer_size;
    int i;
    int retval = -ENOMEM;

    dev = kzalloc(sizeof(struct stm32leds), GFP_KERNEL);
    if(dev == NULL)
    {
        dev_err(&interface->dev, "Out of memory\n");
        goto error;
    }
    kref_init(&dev->kref);

    dev->udev = usb_get_dev(interface_to_usbdev(interface));
    dev->interface = interface;

    spin_lock_init(&dev->lock);
    mutex_init(&dev->sysfslock);
    
    /*
     * aquire and set info about endpoints
     */
    iface_desc = interface->cur_altsetting;
    for(i = 0; i < iface_desc->desc.bNumEndpoints; i++)
    {
        endpoint = &iface_desc->endpoint[i].desc;

        /*
         * search for interrupt IN endpoint in interface
         */
        if(!dev->int_in_endpointAddr &&
            usb_endpoint_is_int_in(endpoint))
        {
            /* when interrupt IN endpoint found */
            buffer_size = endpoint->wMaxPacketSize;
            dev->int_in_size = buffer_size;
            dev->int_in_interval = endpoint->bInterval;
            dev->int_in_endpointAddr = endpoint->bEndpointAddress;
            dev->int_in_buffer = kzalloc(buffer_size, GFP_KERNEL);
            if(!dev->int_in_buffer)
            {
                pr_err("[stm32leds] - Can't allocate memory for int_in_buffer");
                goto error;
            }
            dev_info(&interface->dev, "[stm32leds] - endpointInaddr = %x", dev->int_in_endpointAddr);
        }
        /* 
         * search for interrupt IN endpoint in interface
         */
        if(!dev->int_out_endpointAddr &&
            usb_endpoint_is_int_out(endpoint))
        {
            dev->int_out_endpointAddr = endpoint->bEndpointAddress;
            dev->int_out_interval = endpoint->bInterval;
            dev_info(&interface->dev, "[stm32leds] - endpointOutaddr = %x", dev->int_out_endpointAddr);
        }
    }
    if (!(dev->int_in_endpointAddr && dev->int_out_endpointAddr))
    {
        pr_err("[stm32leds] - Could not find any interrupt type endpoints");
        goto error;
    }

    /* 
     * save our data pointer in this interface device
     */
    usb_set_intfdata(interface, dev);
    /* 
     * we can register the device now, as it is ready
     */
    retval = usb_register_dev(interface, &stm32leds_class);
    if (retval)
    {
        pr_err("[stm32leds] - Not able to get a minor num for this device");
        usb_set_intfdata(interface, NULL);
        goto error;
    }

    /* 
     * inform user of success
     */
    dev_info(&interface->dev, "[stm32leds] - USB STM32Leds device now attached to STM32Leds-%d", interface->minor);
    
    return 0;

error:
    if(dev)
        kref_put(&dev->kref, stm32leds_delete);
    return retval;
}

/*
 * disconnect is a "remove" method for usb_device
 */
static void stm32leds_disconnect(struct usb_interface *interface)
{
    struct stm32leds *dev;
    int minor = interface->minor;

    dev = usb_get_intfdata(interface);
    usb_set_intfdata(interface, NULL);

    /* 
     * give back assigned minor number
     */
    usb_deregister_dev(interface, &stm32leds_class);

    /* 
     * decrement usage coun ref
     */
    kref_put(&dev->kref, stm32leds_delete);

    dev_info(&interface->dev, "[stm32leds] - USB STM32Leds #%d device now disconnected\n", minor);
}

/* 
 * LDD3 page 348 - you gonna read it anyways
 * why not right know?
 */
static struct usb_driver stm32leds_driver =
{
    .name = "STM32Leds",
    .id_table = stm32leds_table,
    .probe = stm32leds_probe,
    .disconnect = stm32leds_disconnect,
};

/*
 * standard __init and __exit functions implemented
 * linux kernel module
 */
static int __init stm32leds_init(void)
{
    int result;

    /* register driver with the USB subsystem */ 
    result = usb_register(&stm32leds_driver);
    if(result)
        pr_err("usb_register failed. Error number %d", result);

    return result;
}

static void __exit stm32leds_exit(void)
{
    /* deregister this driver with the USB subsystem */
    usb_deregister(&stm32leds_driver);
}

module_init(stm32leds_init);
module_exit(stm32leds_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Okhrimenko <mrromanjoe@gmail.com>");
MODULE_DESCRIPTION("STM32F4Discovery Leds USB driver");