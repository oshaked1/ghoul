/**
 * Code based on helper library for ftrace hooking kernel functions
 * by Harvey Phillips (xcellerator@gmx.com)
*/
#include <linux/ftrace.h>
#include "hook.h"
#include "ghoul.h"

/* We need to prevent recursive loops when hooking, otherwise the kernel will
 * panic and hang. The options are to either detect recursion by looking at
 * the function return address, or by jumping over the ftrace call. We use the 
 * first option, by setting USE_FENTRY_OFFSET = 0, but could use the other by
 * setting it to 1. (Oridinarily ftrace provides it's own protections against
 * recursion, but it relies on saving return registers in $rip. We will likely
 * need the use of the $rip register in our hook, so we have to disable this
 * protection and implement our own).
 * */
#define USE_FENTRY_OFFSET 0
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

/* Ftrace needs to know the address of the original function that we
 * are going to hook. As before, we just use kallsyms_lookup_name() 
 * to find the address in kernel memory.
 * */
notrace static int fh_resolve_hook_address(struct ftrace_hook *hook)
{
    hook->address = symbol_addr(hook->name);

    if (hook->address == 0)
        return -ENOENT;

#if USE_FENTRY_OFFSET
    *((unsigned long*) hook->call_original) = hook->address + MCOUNT_INSN_SIZE;
#else
    *((unsigned long*) hook->call_original) = hook->address;
#endif

    return 0;
}

/* See comment below within fh_install_hook() */
notrace static void fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip, struct ftrace_ops *ops, struct pt_regs *regs)
{
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
    regs->ip = (unsigned long) hook->function;
#else
    if(!within_module(parent_ip, THIS_MODULE))
        regs->ip = (unsigned long) hook->hook_function;
#endif
}

/* Assuming we've already set hook->name, hook->function and hook->original, we 
 * can go ahead and install the hook with ftrace. This is done by setting the 
 * ops field of hook (see the comment below for more details), and then using
 * the built-in ftrace_set_filter_ip() and register_ftrace_function() functions
 * provided by ftrace.h
 * */
notrace int register_ftrace_hook(struct ftrace_hook *hook)
{
    int err;
    err = fh_resolve_hook_address(hook);
    if(err)
        return err;
    /* For many of function hooks (especially non-trivial ones), the $rip
     * register gets modified, so we have to alert ftrace to this fact. This
     * is the reason for the SAVE_REGS and IP_MODIFY flags. However, we also
     * need to OR the RECURSION_SAFE flag (effectively turning if OFF) because
     * the built-in anti-recursion guard provided by ftrace is useless if
     * we're modifying $rip. This is why we have to implement our own checks
     * (see USE_FENTRY_OFFSET). */
    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
            | FTRACE_OPS_FL_RECURSION_SAFE
            | FTRACE_OPS_FL_IPMODIFY;

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if(err)
    {
        debug("ghoul: ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }

    err = register_ftrace_function(&hook->ops);
    if(err)
    {
        debug("ghoul: register_ftrace_function() failed: %d\n", err);
        return err;
    }

    return 0;
}

/* Disabling our function hook is just a simple matter of calling the built-in
 * unregister_ftrace_function() and ftrace_set_filter_ip() functions (note the
 * opposite order to that in register_ftrace_hook()).
 * */
notrace void unregister_ftrace_hook(struct ftrace_hook *hook)
{
    int err;
    err = unregister_ftrace_function(&hook->ops);
    if(err)
    {
        debug("ghoul: unregister_ftrace_function() failed: %d\n", err);
    }

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if(err)
    {
        debug("ghoul: ftrace_set_filter_ip() failed: %d\n", err);
    }
}

/* To make it easier to hook multiple functions in one module, this provides
 * a simple loop over an array of ftrace_hook struct
 * */
notrace int register_ftrace_hooks(struct ftrace_hook *hooks, size_t count)
{
    int err;
    size_t i;

    for (i = 0 ; i < count ; i++)
    {
        err = register_ftrace_hook(&hooks[i]);
        if(err)
            goto error;
    }
    return 0;

error:
    while (i != 0)
    {
        unregister_ftrace_hook(&hooks[--i]);
    }
    return err;
}

notrace void unregister_ftrace_hooks(struct ftrace_hook *hooks, size_t count)
{
    size_t i;

    for (i = 0 ; i < count ; i++)
        unregister_ftrace_hook(&hooks[i]);
}