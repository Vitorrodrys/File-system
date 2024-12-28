#include "superblock/dentry.hpp"


int main(){

    DEntry dentry;
    dentry.add_entry("/file1", 1);
    dentry.add_entry("/file2", 2);
    dentry.add_entry("/file3", 3);

    InodeType a = dentry.get_inode("/file3");
    printf("Inode: %llu\n", a);

}
