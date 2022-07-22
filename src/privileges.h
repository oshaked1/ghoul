#include <linux/kernel.h>

#define THIS_TASK 0
#define PARENT_TASK -1

struct pid_list {
    int pid;
    struct list_head list;
};
extern struct list_head pids_waiting_for_root;

void set_root(void);
void give_root(int pid);