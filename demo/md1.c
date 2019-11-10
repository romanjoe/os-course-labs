#include <linux/init.h>
#include <linux/module.h>

#include "md.h"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Joe <mrromanjoe@gmail.com>");
char * md1_data = "Hello, class!!!";
extern char * md1_proc(void)
{
	return md1_data;
}

static char * md1_local(void)
{
	return md1_data;
}

extern int md1_intvalue(void)
{ 
	int a = 10;
	int b = 4;

	return a+b; 
}

EXPORT_SYMBOL(md1_data);
EXPORT_SYMBOL(md1_proc);
EXPORT_SYMBOL(md1_intvalue);

static int __init md_init(void)
{
	printk("+ module md1 start!\n");
	return 0;
}

static void __exit md_exit(void)
{
	printk("+ module md1 unloaded!\n");
}
module_init(md_init);
module_exit(md_exit);
