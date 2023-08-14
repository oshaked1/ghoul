#ifdef __DEBUG__
#define debug(fmt, ...) \
    pr_info(fmt, ##__VA_ARGS__)
#define error(fmt, ...) \
    pr_err(fmt, ##__VA_ARGS__)
#else /* !__DEBUG__ */
#define debug(fmt, ...)
#define error(fmt, ...)
#endif /* __DEBUG__ */

notrace unsigned long symbol_addr(const char *name);