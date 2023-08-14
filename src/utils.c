#include <linux/kallsyms.h>
#include "ghoul.h"

notrace unsigned long symbol_addr(const char *name)
{
    unsigned long addr = kallsyms_lookup_name(name);

    if (addr == 0)
        debug("ghoul: unresolved symbol %s\n", name);

    return addr;
}