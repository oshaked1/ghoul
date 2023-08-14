#include "ghoul.h"
#include "hook.h"

#ifndef CONFIG_X86_64
#error Only x86_64 is supported
#endif

#if defined(CONFIG_X86) || defined(CONFIG_X86_64)
#include <asm/insn.h>

void (*private_insn_init)(struct insn *insn, const void *kaddr, int buf_len, int x86_64);
void (*private_insn_get_length)(struct insn *insn);

static __always_inline unsigned long disable_wp(void)
{
    unsigned long cr0, new_cr0;

    preempt_disable();
    barrier();

    cr0 = read_cr0();
    new_cr0 = cr0 & ~X86_CR0_WP;
    if (cr0 != new_cr0)
        // Write cr0 directly instead of using the kernel's write_cr0 function.
        // write_cr0 detects exactly this flag being disabled and throws a massive tantrum.
        asm volatile("mov %0, %%cr0" : "+r" (new_cr0) :: "memory");
    return cr0;
}

static __always_inline void restore_wp(unsigned long cr0)
{
    write_cr0(cr0);

    barrier();
    preempt_enable();
}

static __always_inline void private_kernel_insn_init(struct insn *insn, const void *kaddr, int buf_len)
{
#ifdef CONFIG_X86_64
	private_insn_init(insn, kaddr, buf_len, 1);
#else /* CONFIG_X86_32 */
	private_insn_init(insn, kaddr, buf_len, 0);
#endif
}

notrace static int find_trampoline_size(void *function)
{
    struct insn insn;
    int offset = 0;

    while (offset < HOOK_SIZE) {
        private_kernel_insn_init(&insn, function + offset, MAX_INSN_SIZE);
        private_insn_get_length(&insn);
        offset += insn.length;
    }

    return offset;
}
#endif /* defined(CONFIG_X86) || defined(CONFIG_X86_64) */

static __always_inline void setup_trampoline(void *original_function, void *trampoline, size_t trampoline_size)
{
#ifdef CONFIG_X86_64
    unsigned long orig_cr0 = disable_wp();
#endif

    char jump_back[ABSOLUTE_JUMP_SIZE] = ABSOLUTE_JUMP_TEMPLATE;
#ifdef CONFIG_X86_64
    *(unsigned long *)&jump_back[ABSOLUTE_JUMP_ADDRESS_OFFSET] = (unsigned long)original_function + trampoline_size;
#else
#error !CONFIG_X86_64
#endif

    memcpy(trampoline, original_function, trampoline_size);
    memcpy(trampoline + trampoline_size, jump_back, ABSOLUTE_JUMP_SIZE);

#ifdef CONFIG_X86_64
    restore_wp(orig_cr0);
#endif
}

static __always_inline void install_hook(void *original_funcion, void *hook_function)
{
#ifdef CONFIG_X86_64
    unsigned long orig_cr0 = disable_wp();
#endif

#ifdef CONFIG_X86_64
    char hook_code[HOOK_SIZE] = ABSOLUTE_JUMP_TEMPLATE;
    *(unsigned long *)&hook_code[ABSOLUTE_JUMP_ADDRESS_OFFSET] = (unsigned long)hook_function;
#else
#error !CONFIG_X86_64
#endif

    memcpy(original_funcion, hook_code, HOOK_SIZE);

#ifdef CONFIG_X86_64
    restore_wp(orig_cr0);
#endif
}

static __always_inline void uninstall_hook(void *original_function, void *trampoline)
{
#ifdef CONFIG_X86_64
    unsigned long orig_cr0 = disable_wp();
#endif

    memcpy(original_function, trampoline, HOOK_SIZE);
    
#ifdef CONFIG_X86_64
    restore_wp(orig_cr0);
#endif
}

notrace int register_inline_hook(struct inline_hook *hook)
{
    int trampoline_size;

    hook->address = symbol_addr(hook->name);

    if (hook->address == 0)
        return -ENOENT;

#ifdef CONFIG_X86_64
    trampoline_size = find_trampoline_size((void *)hook->address);
#else
#error !CONFIG_X86_64
#endif
    
    setup_trampoline((void *)hook->address, hook->trampoline, trampoline_size);
    install_hook((void *)hook->address, hook->hook_function);

    return 0;
}

notrace void unregister_inline_hook(struct inline_hook *hook)
{
    uninstall_hook((void *)hook->address, hook->trampoline);
}

notrace int register_inline_hooks(struct inline_hook *hooks, size_t count)
{
    int err;
    size_t i;

#if defined(CONFIG_X86) || defined(CONFIG_X86_64)
    private_insn_init = (void (*)(struct insn *insn, const void *kaddr, int buf_len, int x86_64))kallsyms_lookup_name("insn_init");
    private_insn_get_length = (void (*)(struct insn *insn))kallsyms_lookup_name("insn_get_length");

    if (private_insn_init == NULL || private_insn_get_length == NULL)
        return -ENOENT;
#endif

    for (i = 0; i < count; i++) {
        err = register_inline_hook(&hooks[i]);
        if (err)
            goto error;
    }
    return 0;

error:
    while (i != 0)
        unregister_inline_hook(&hooks[--i]);
    return err;
}

notrace void unregister_inline_hooks(struct inline_hook *hooks, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++)
        unregister_inline_hook(&hooks[i]);
}