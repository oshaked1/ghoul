#include <linux/version.h>

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS
#endif

#ifdef PTREGS_SYSCALL_STUBS
#define SYSCALL_NAME(name) ("__x64_" name)
#else
#define SYSCALL_NAME(name) (name)
#endif

#define HOOK(_name, _hook, _orig)   \
{                   \
    .name = (_name),        \
    .function = (_hook),        \
    .original = (_orig),        \
}

#define SYSCALL_HOOK(_name, _hook, _orig)   \
{                   \
    .name = SYSCALL_NAME(_name),        \
    .function = (_hook),        \
    .original = (_orig),        \
}

struct ftrace_hook {
    const char *name;
    void *function;
    void *original;

    unsigned long address;
    struct ftrace_ops ops;
};

notrace int ftrace_install_hooks(struct ftrace_hook *hooks, size_t count);
notrace void ftrace_remove_hooks(struct ftrace_hook *hooks, size_t count);

notrace void inline_hook_register(void *original_function, void *hook_function);
notrace void inline_hook_pause(void *func);
notrace void inline_hook_resume(void *func);
notrace void inline_hook_unregister(void *func);