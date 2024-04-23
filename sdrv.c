#include <linux/init.h>   /* module_init, module_exit */
#include <linux/module.h> /* version info, MODULE_LICENSE, MODULE_AUTHOR, printk() */
#include <linux/slab.h>
#include <linux/kprobes.h>

MODULE_DESCRIPTION("sdrv (simple driver) in linux");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hdthky");

static int rp_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
	void *data = ri->data;

    pr_info("data: %px\n", data);

	return 0;
}

static int rp_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
    pr_info("rp_handler\n");

	return 0;
}

static struct kretprobe rp = {
    .kp.symbol_name = "__x64_sys_execve",
    .data_size = 8,
    .entry_handler = rp_entry_handler,
    .handler = rp_handler,
};

/*===============================================================================================*/
static int sdrv_init(void) {
    int result = 0;
    pr_info("[sdrv] init\n");

    result = register_kretprobe(&rp);
    if (result) {
        pr_err("Failed to register kretprobe hook\n");
    }

    return result;
}

/*===============================================================================================*/
static void sdrv_exit(void) {
    pr_info("[sdrv] exit\n");

    unregister_kretprobe(&rp);
}

/*===============================================================================================*/
module_init(sdrv_init);
module_exit(sdrv_exit);
