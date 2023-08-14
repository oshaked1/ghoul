#include <linux/version.h>
#include <linux/ftrace.h>

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS
#endif

#ifdef PTREGS_SYSCALL_STUBS
#define SYSCALL_NAME(name) ("__x64_" name)
#else
#define SYSCALL_NAME(name) (name)
#endif

struct ftrace_hook {
    const char *name;
    void *hook_function;
    void *call_original;

    unsigned long address;
    struct ftrace_ops ops;
};

#define FTRACE_HOOK(_name, _hook, _orig)   \
{                   \
    .name = (_name),        \
    .hook_function = (_hook),        \
    .call_original = (_orig),        \
}

#define FTRACE_SYSCALL_HOOK(_name, _hook, _orig)   \
{                   \
    .name = SYSCALL_NAME(_name),        \
    .hook_function = (_hook),        \
    .call_original = (_orig),        \
}

notrace int register_ftrace_hooks(struct ftrace_hook *hooks, size_t count);
notrace void unregister_ftrace_hooks(struct ftrace_hook *hooks, size_t count);

#ifdef HIDE_FTRACE
struct inline_hook {
    const char *name;
    void *hook_function;
    void *trampoline;

    unsigned long address;
};

#define INLINE_HOOK(_name, _hook, _tramp)   \
{                   \
    .name = (_name),        \
    .hook_function = (_hook),        \
    .trampoline = (_tramp),        \
}

#ifdef CONFIG_X86_64
// mov rax, $addr; jmp rax;
#define ABSOLUTE_JUMP_TEMPLATE "\x48\xb8\x00\x00\x00\x00\x00\x00\x00\x00\xff\xe0"
#define ABSOLUTE_JUMP_ADDRESS_OFFSET 2
#define ABSOLUTE_JUMP_SIZE 12
#define HOOK_SIZE ABSOLUTE_JUMP_SIZE

// Create an empty function with enough space to hold a hook trampoline.
// Maximum required size for a trampoline is HOOK_SIZE - 1 + MAX_INSN_SIZE = 12 - 1 + 15 = 26
#define RESERVE_TRAMPOLINE(func) \
    notrace __attribute__((naked)) void func(void) { \
        /* make sure nothing happens if trampoline is called before being set up */ \
        asm volatile ("ret"); /* 1 byte instruction */ \
        /* fill up the rest of the function with meaningless instructions */ \
        asm volatile ("add $0, %rax"); /* 4 byte instruction */ \
        asm volatile ("add $0, %rax"); \
        asm volatile ("add $0, %rax"); \
        asm volatile ("add $0, %rax"); \
        asm volatile ("add $0, %rax"); \
        asm volatile ("add $0, %rax"); \
        asm volatile ("nop"); /* 1 byte instruction */ \
    }
#else /* !CONFIG_X86_64 */
#error Only x86_64 is supported for inline hooking
#endif /* CONFIG_X86_64 */

notrace int register_inline_hooks(struct inline_hook *hooks, size_t count);
notrace void unregister_inline_hooks(struct inline_hook *hooks, size_t count);
#endif /* HIDE_FTRACE */