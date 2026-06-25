#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/uprobes.h>
#include <linux/sched.h>

MODULE_DESCRIPTION("sdrv (simple driver) in linux");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hdthky");

#define BASH_PATH "/usr/bin/bash"
// nm -D /usr/bin/bash | grep 'execute_command$'
#define EXECUTE_COMMAND_OFFSET 0x4ac80

static struct inode *bash_inode;
static struct uprobe_consumer bash_uc;

static int bash_execute_command_handler(struct uprobe_consumer *self,
					struct pt_regs *regs)
{
	pr_info("[sdrv] bash execute_command: pid=%d comm=%s ip=0x%lx arg0=0x%lx\n",
		current->pid, current->comm, regs->ip, regs->di);
	return 0;
}

static int register_bash_uprobe(void)
{
	struct path path;
	int ret;

	bash_uc.handler = bash_execute_command_handler;

	ret = kern_path(BASH_PATH, LOOKUP_FOLLOW, &path);
	if (ret) {
		pr_err("[sdrv] kern_path(%s) failed: %d\n", BASH_PATH, ret);
		return ret;
	}

	bash_inode = igrab(d_inode(path.dentry));
	path_put(&path);
	if (!bash_inode)
		return -EINVAL;

	ret = uprobe_register(bash_inode, EXECUTE_COMMAND_OFFSET, &bash_uc);
	if (ret) {
		pr_err("[sdrv] uprobe_register failed: %d\n", ret);
		iput(bash_inode);
		bash_inode = NULL;
	}

	return ret;
}

static void unregister_bash_uprobe(void)
{
	if (!bash_inode)
		return;

	uprobe_unregister(bash_inode, EXECUTE_COMMAND_OFFSET, &bash_uc);
	iput(bash_inode);
	bash_inode = NULL;
}

/*===============================================================================================*/
static int sdrv_init(void)
{
	int result;

	pr_info("[sdrv] init\n");

	result = register_bash_uprobe();
	if (result)
		return result;

	pr_info("[sdrv] uprobe on %s+0x%lx (execute_command)\n",
		BASH_PATH, (unsigned long)EXECUTE_COMMAND_OFFSET);
	return 0;
}

/*===============================================================================================*/
static void sdrv_exit(void)
{
	unregister_bash_uprobe();
	pr_info("[sdrv] exit\n");
}

/*===============================================================================================*/
module_init(sdrv_init);
module_exit(sdrv_exit);
