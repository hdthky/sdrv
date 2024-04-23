#include <linux/init.h>   /* module_init, module_exit */
#include <linux/module.h> /* version info, MODULE_LICENSE, MODULE_AUTHOR, printk() */
#include <linux/slab.h>
#include <linux/kprobes.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>

MODULE_DESCRIPTION("sdrv (simple driver) in linux");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hdthky");

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,7,0)
static int init_kallsyms(void) {
        return 0;
}
#elif IS_ENABLED(CONFIG_KPROBES)
typedef unsigned long(*kallsyms_fn)(const char *);

static kallsyms_fn kallsyms = NULL;

unsigned long kallsyms_lookup_name(const char *name)
{
        return kallsyms(name);
}

static int init_kallsyms(void)
{
        struct kprobe kp = {0};
        int ret = 0;
        kp.symbol_name = "kallsyms_lookup_name";

        ret = register_kprobe(&kp);

        if (ret < 0)
                return ret;

        kallsyms = (kallsyms_fn)kp.addr;

        unregister_kprobe(&kp);

        return ret;
}
#else
#error "No suitable kallsyms acquisition method!"
#endif

int (*rp_get_cmdline)(struct task_struct *task, char *buffer, int buflen);

static int rp_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
    void *data = ri->data;

    pr_info("data: %px\n", data);

    return 0;
}

static int rp_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
    char buf[256];

    if (!rp_get_cmdline)
        rp_get_cmdline = (void *)kallsyms_lookup_name("get_cmdline");

    rp_get_cmdline(current, buf, sizeof(buf));

    pr_info("cmdline: %s\n", buf);

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

    result = init_kallsyms();
    if (result) {
        pr_err("Failed to init kallsyms\n");
        goto out;
    }

    result = register_kretprobe(&rp);
    if (result) {
        pr_err("Failed to register kretprobe hook\n");
        goto out;
    }

out:
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
