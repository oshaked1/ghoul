#include <linux/kernel.h>

/**
 * Some private definitions from the Linux kernel (5.4.0).
 * These are temporary - I will find a way to include these automatically.
 */

// from fs/namei.c
#define EMBEDDED_LEVELS 2
struct nameidata {
	struct path	path;
	struct qstr	last;
	struct path	root;
	struct inode	*inode; /* path.dentry.d_inode */
	unsigned int	flags;
	unsigned	seq, m_seq;
	int		last_type;
	unsigned	depth;
	int		total_link_count;
	struct saved {
		struct path link;
		struct delayed_call done;
		const char *name;
		unsigned seq;
	} *stack, internal[EMBEDDED_LEVELS];
	struct filename	*name;
	struct nameidata *saved;
	struct inode	*link_inode;
	unsigned	root_seq;
	int		dfd;
};

// from kernel/trace/ftrace.c
struct ftrace_iterator {
	loff_t				pos;
	loff_t				func_pos;
	loff_t				mod_pos;
	struct ftrace_page		*pg;
	struct dyn_ftrace		*func;
	struct ftrace_func_probe	*probe;
	struct ftrace_func_entry	*probe_entry;
	struct trace_parser		parser;
	struct ftrace_hash		*hash;
	struct ftrace_ops		*ops;
	struct trace_array		*tr;
	struct list_head		*mod_list;
	int				pidx;
	int				idx;
	unsigned			flags;
};