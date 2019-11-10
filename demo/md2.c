#include <linux/init.h>
#include <linux/module.h>
#include "md.h"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Joe <mrromanjoe@gmail.com>");

static int __init md_init(void)
{
	printk("+ module md2 start!\n");
	printk("+ data string exported from md1: %s\n", md1_data);
	printk("+ string returted md1_proc() is: %s\n", md1_proc());
	printk("+ string returned from md1_intvalue is: %i\n", md1_intvalue());
	return 0;
}

static void __exit md_exit(void)
{
	printk("+ module md2 unloaded!\n");
}

module_init(md_init);
module_exit(md_exit);