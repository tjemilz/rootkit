#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>



static int __init rootkit_init(void) {
    printk(KERN_INFO "Hello world\n");

    return 0;
}


static void __exit rootkit_exit(void) {
    printk(KERN_INFO "Exiting\n");
}


module_init(rootkit_init);
module_exit(rootkit_exit);
MODULE_LICENSE("GPL");