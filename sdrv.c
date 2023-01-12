#include <linux/init.h>       /* module_init, module_exit */
#include <linux/module.h>     /* version info, MODULE_LICENSE, MODULE_AUTHOR, printk() */
#include <linux/slab.h>

MODULE_DESCRIPTION("sdrv (simple driver) in linux");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hdthky");

/*===============================================================================================*/
static int sdrv_init(void) {
    int result = 0;
    pr_info("[sdrv] init\n" );

    return result;
}

/*===============================================================================================*/
static void sdrv_exit(void) {
    pr_info("[sdrv] exit\n" );
}

/*===============================================================================================*/
module_init(sdrv_init);
module_exit(sdrv_exit);
