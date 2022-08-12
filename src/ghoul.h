#ifdef __DEBUG__
#define debug(fmt, ...) \
    pr_info(fmt, ##__VA_ARGS__)
#define error(fmt, ...) \
    pr_err(fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#define error(fmt, ...)
#endif