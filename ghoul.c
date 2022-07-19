#include <linux/module.h>

static int __init ghoul_init(void)
{
    pr_info("Ghoul Inserted\n");
    return 0;
}

static void __exit ghoul_exit(void)
{
    return;
}

module_init(ghoul_init)
module_exit(ghoul_exit)
MODULE_LICENSE("GPL");