#define FUSE_USE_VERSION 31
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <regex>

#include "env.hpp"
#include "fuse_interface.hpp"
#include "superblock/blocks_manager.hpp"
#include "superblock/path_handler.hpp"
#include "superblock/types.hpp"
#include "inodes/fcb.hpp"

#define FILE_DEFAULT_PERMISSIONS 0666
#define DIRECTORY_DEFAULT_PERMISSIONS 0777

BlocksManager bmanager = BlocksManager();
PathHandler path_handler = PathHandler();

const Env &envs = Env::get_instance();
int open(const char *path, struct fuse_file_info *fi) {
    InodeType inode=path_handler.get_inode(path);

    if (inode == NOTFOUNDERROR ){
        return -ENOENT;
    }

    File file(inode);
    fi->fh=  file.get_inode();

    return  0;
}

int mknod (const char *path, mode_t mode, dev_t dev) {
    InodeType inode = path_handler.get_inode(path);
    if (inode != NOTFOUNDERROR){
        return -EEXIST;
    }
    const struct fuse_context *context = fuse_get_context();
    mode_t permissions = FILE_DEFAULT_PERMISSIONS & ~context->umask;
    File new_file(bmanager, permissions, context);
    path_handler.add_path(path, new_file.get_inode());
    return 0;
}
int read (const char * path, char *buffer, size_t  size_bytes, off_t offset,struct fuse_file_info *fi) {
    InodeType inode = fi->fh;

    File file(inode) ;
    size_t size =file.read(offset,size_bytes,buffer);

    if (!file.read(offset,size_bytes,buffer)) {
        return -EIO;
    }
    return  (int) size;
}
int write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    InodeType inode=fi->fh;

    File file(inode);
    size_t size_bytes=file.write(offset,size,buf,bmanager);

    if (!size_bytes) {
        return -EIO;
    }
    return (int) size_bytes;
}

int rename (const char *path, const char * newpath, unsigned int flags) {
    InodeType inode=path_handler.get_inode(path);

    if (inode == NOTFOUNDERROR){
        return -EBADF;
    }
    InodeType inode_new_path= path_handler.get_inode(newpath);

    if (flags == RENAME_NOREPLACE) {
        if (inode_new_path != NOTFOUNDERROR) {
            return -EEXIST;
        }
        InodeType inode_remov_path=path_handler.remove_path(path,bmanager);

        path_handler.add_path(newpath, inode_remov_path);
        return 0;
    }


    if (flags == RENAME_EXCHANGE ){
        if (inode_new_path == NOTFOUNDERROR) {
            return -ENOENT;
        }
        InodeType inode_remov_path=path_handler.remove_path(path,bmanager);
        InodeType inode_remov_newpath=path_handler.remove_path(newpath,bmanager);
        path_handler.add_path(newpath, inode_remov_path);
        path_handler.add_path(path, inode_remov_newpath);

        return 0;
    }


}

int mkdir (const char *dpath, mode_t mode){

    InodeType inode = path_handler.get_inode(dpath);
    if (inode != NOTFOUNDERROR){
        return -EEXIST;
    }
    const struct fuse_context *context = fuse_get_context();
    mode_t permissions = DIRECTORY_DEFAULT_PERMISSIONS & ~context->umask;
    Directory new_dir(bmanager, permissions, context);
    path_handler.add_path(dpath, new_dir.get_inode());
    return 0;
}


void *destroy(void *private_data){
    bmanager.save();
    return nullptr;
}



struct fuse_operations *build_fuse_operations() {
    static struct fuse_operations fuse_op = (struct fuse_operations){
        .open = open,
        .read = read,
        .write = write,
        .rename = rename,
        .mknod = mknod,
        .mkdir = mkdir,
        .destroy = destroy,
        

    };
    return &fuse_op;
}
