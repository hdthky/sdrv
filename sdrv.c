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

#if defined(CONFIG_X86_64)

#define SDRV_INSTRUCTION_POINTER(pt_regs) ((pt_regs)->ip)
#define SDRV_SYSCALL_NUM(pt_regs)         ((pt_regs)->ax)
#define SDRV_FUNC_CALL_ARG0(pt_regs)      ((pt_regs)->di)
#define SDRV_FUNC_CALL_ARG1(pt_regs)      ((pt_regs)->si)
#define SDRV_FUNC_CALL_ARG2(pt_regs)      ((pt_regs)->dx)
#define SDRV_FUNC_CALL_ARG3(pt_regs)      ((pt_regs)->cx)
#define SDRV_FUNC_CALL_ARG4(pt_regs)      ((pt_regs)->r8)
#define SDRV_FUNC_CALL_ARG5(pt_regs)      ((pt_regs)->r9)
#define SDRV_RETURN_VALUE(pt_regs)        ((pt_regs)->ax)

#elif defined(CONFIG_ARM64)

#define SDRV_INSTRUCTION_POINTER(pt_regs) ((pt_regs)->pc)
#define SDRV_SYSCALL_NUM(pt_regs)         ((pt_regs)->regs[8])
#define SDRV_FUNC_CALL_ARG0(pt_regs)      ((pt_regs)->regs[0])
#define SDRV_FUNC_CALL_ARG1(pt_regs)      ((pt_regs)->regs[1])
#define SDRV_FUNC_CALL_ARG2(pt_regs)      ((pt_regs)->regs[2])
#define SDRV_FUNC_CALL_ARG3(pt_regs)      ((pt_regs)->regs[3])
#define SDRV_FUNC_CALL_ARG4(pt_regs)      ((pt_regs)->regs[4])
#define SDRV_FUNC_CALL_ARG5(pt_regs)      ((pt_regs)->regs[5])
#define SDRV_RETURN_VALUE(pt_regs)        ((pt_regs)->regs[0])

#elif defined(CONFIG_LOONGARCH)

#define SDRV_INSTRUCTION_POINTER(pt_regs) ((pt_regs)->csr_era)
#define SDRV_SYSCALL_NUM(pt_regs)         ((pt_regs)->orig_a0)
#define SDRV_FUNC_CALL_ARG0(pt_regs)      ((pt_regs)->regs[4])
#define SDRV_FUNC_CALL_ARG1(pt_regs)      ((pt_regs)->regs[5])
#define SDRV_FUNC_CALL_ARG2(pt_regs)      ((pt_regs)->regs[6])
#define SDRV_FUNC_CALL_ARG3(pt_regs)      ((pt_regs)->regs[7])
#define SDRV_FUNC_CALL_ARG4(pt_regs)      ((pt_regs)->regs[8])
#define SDRV_FUNC_CALL_ARG5(pt_regs)      ((pt_regs)->regs[9])
#define SDRV_RETURN_VALUE(pt_regs)        ((pt_regs)->regs[4])

#endif

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
    struct file *file = (struct file *)SDRV_FUNC_CALL_ARG0(regs);
    unsigned long prot =  (unsigned long)SDRV_FUNC_CALL_ARG1(regs);
	unsigned long flags =  (unsigned long)SDRV_FUNC_CALL_ARG2(regs);

    pr_info("%px %lu %lu\n", file, prot, flags);

    return 0;
}

static struct kretprobe rp = {
    .kp.symbol_name = "security_mmap_file",
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
