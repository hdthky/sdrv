#include <linux/init.h>       /* module_init, module_exit */
#include <linux/module.h>     /* version info, MODULE_LICENSE, MODULE_AUTHOR, printk() */
#include <linux/fs.h> 	      /* file stuff */
#include <linux/kernel.h>     /* printk() */
#include <linux/errno.h>      /* error codes */
#include <linux/module.h>     /* THIS_MODULE */
#include <linux/cdev.h>       /* char device stuff */
#include <linux/uaccess.h>    /* copy_to_user() */

static const char scdd_file_content[] = "scdd is simple char device driver\0";
static const ssize_t scdd_file_size = sizeof(scdd_file_content);

MODULE_DESCRIPTION("scdd (simple char device driver) in linux");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hdthky");

/*===============================================================================================*/
static ssize_t scdd_read(struct file *filp, char __user *user_buffer, size_t count, loff_t *possition)
{
    printk("[scdd] Device file is read at offset = %i, read bytes count = %u\n",
            (int)*possition, (unsigned int)count);

    if(*possition >= scdd_file_size)
        return 0;

    if(*possition + count > scdd_file_size)
        count = scdd_file_size - *possition;

    if( copy_to_user(user_buffer, scdd_file_content + *possition, count) != 0)
        return -EFAULT;

    *possition += count;
    return count;
}

/*===============================================================================================*/
static ssize_t scdd_write(struct file *filp, const char __user *user_buffer, size_t count, loff_t *possition)
{
    printk("[scdd] write\n");
    
    return 1;
}

/*===============================================================================================*/
static long scdd_ioctl(struct file *filp, unsigned int cmd, unsigned long user_buffer)
{
    printk("[scdd] ioctl: cmd is %i\n", cmd);
    
    return 0;
}

/*===============================================================================================*/
static struct file_operations simple_driver_fops = 
{
    .owner = THIS_MODULE,
    .read = scdd_read,
    .write = scdd_write,
    .unlocked_ioctl = scdd_ioctl,
};

static int device_file_major_number = 236;
static const char device_name[] = "scd"; // simple char device

/*===============================================================================================*/
static int scdd_init(void)
{
    pr_info("[scdd] init: register chrdev\n");

    int major_number = register_chrdev(0, device_name, &simple_driver_fops);

    if(major_number < 0)
    {
        printk("[scdd] init: register_chrdev failed with error code %i\n", major_number);
        return major_number;
    }
    
    device_file_major_number = major_number;

    return 0;
}

/*===============================================================================================*/
static void scdd_exit(void)
{
    pr_info("[scdd] exit: unregister chrdev\n" );

    if(device_file_major_number != 0)
    {
        unregister_chrdev(device_file_major_number, device_name);
    }
}

/*===============================================================================================*/
module_init(scdd_init);
module_exit(scdd_exit);