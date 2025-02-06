#define FUSE_USE_VERSION 31
#include <errno.h>
#include <filesystem>
#include <fcntl.h>
#include <fuse.h>
#include <fuse/fuse_common.h>
#include <fuse/fuse_lowlevel.h>
#include <stdio.h>
#include <regex>
#include <cstring>

#include "commons.hpp"
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
    }else{
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

int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    InodeType inode = fi->fh;
    File file(inode);
    off_t position = 2;
    struct stat current_file_stat = {}; // Declare uma vez e inicialize com valores padrão

    if (file.get_type() != FileType::TDIRECTORY ) {
        return -ENOTDIR;
    }
    Directory dir(file);
    if (offset != 0) {

        current_file_stat.st_mode = file.get_permissions();
        current_file_stat.st_gid = file.get_group_id();
        current_file_stat.st_uid = file.get_owner_id();
        current_file_stat.st_size = file.get_size();
        current_file_stat.st_atime = file.get_accessed_at();
        current_file_stat.st_mtime = file.get_modified_at();
        current_file_stat.st_ctime = file.get_created_at();
        current_file_stat.st_nlink = 0;
        current_file_stat.st_ino = file.get_inode();
        current_file_stat.st_dev = 0;
        current_file_stat.st_rdev = 0;
        current_file_stat.st_blksize = 0;
        current_file_stat.st_blocks = file.get_quantity_blocks();

        filler(buf, ".", &current_file_stat, 1);

        std::tuple<std::string, std::string> parent_and_children = separete_parent_and_children(path);
        std::string parent = std::get<0>(parent_and_children);
        InodeType parent_inode = path_handler.get_inode(parent);
        File parent_file(parent_inode);


        current_file_stat.st_mode = parent_file.get_permissions();
        current_file_stat.st_gid = parent_file.get_group_id();
        current_file_stat.st_uid = parent_file.get_owner_id();
        current_file_stat.st_size = parent_file.get_size();
        current_file_stat.st_atime = parent_file.get_accessed_at();
        current_file_stat.st_mtime = parent_file.get_modified_at();
        current_file_stat.st_ctime = parent_file.get_created_at();

        filler(buf, "..", &current_file_stat, 2);
    }

    for (auto it = dir.begin(); it != dir.end(); ++it) {
        position++;
        // Ensure that we are respecting the offset given
        if (position < offset) {
            continue;
        }

        File current(it->second);

        current_file_stat.st_mode = current.get_permissions();
        current_file_stat.st_gid = current.get_group_id();
        current_file_stat.st_uid = current.get_owner_id();
        current_file_stat.st_size = current.get_size();
        current_file_stat.st_atime = current.get_accessed_at();
        current_file_stat.st_mtime = current.get_modified_at();
        current_file_stat.st_ctime = current.get_created_at();

        filler(buf, it->first.c_str(), &current_file_stat, position);
    }

    return 0;
}

void *init(fuse_conn_info *conn){

    std::cerr << "[DEBUG] init function called" << std::endl;
    const Env &envs = Env::get_instance();
    if (std::filesystem::exists(envs.disk_file)) {
        bmanager.load();
        path_handler.add_path("/", envs.inode_slash);
    } else {
        create_virtual_disk(envs.disk_file, envs.disk_size);
        bmanager.build();
        bmanager.save();
        fuse_context *context = fuse_get_context();
        File source(DIRECTORY_DEFAULT_PERMISSIONS, FileType::TDIRECTORY, envs.inode_slash, context->uid, context->gid);
        
    }

}

void destroy(void *private_data){
    bmanager.save();
}


int opendir (const char * path, struct fuse_file_info *fi){
    InodeType inode=path_handler.get_inode(path);
    if (inode == NOTFOUNDERROR) {
        return -ENOENT;
    }
    File file(inode);
    if (file.get_type() != FileType::TDIRECTORY ) {
        return -ENOTDIR;
    }
     fi->fh= inode;
    return  0;
}

int rmdir (const char *path){
    InodeType inode=path_handler.get_inode(path);
    if (inode == NOTFOUNDERROR) {
        return -ENOENT;
    }
    Directory directory(inode);
    directory.remove_entry(path);
    return 0;
}

#include <iostream>

int getattr(const char *path, struct stat *fstat) {
    std::cerr << "[DEBUG] getattr called for path: " << path << std::endl;

    InodeType inode = path_handler.get_inode(path);
    
    if (inode == NOTFOUNDERROR) {
        std::cerr << "[ERROR] Path not found: " << path << std::endl;
        return -ENOENT;
    }

    std::cerr << "[DEBUG] Found inode: " << inode << std::endl;

    File file(inode);

    fstat->st_mode = file.get_permissions();
    std::cerr << "[DEBUG] Permissions: " << fstat->st_mode << std::endl;

    fstat->st_gid = file.get_group_id();
    std::cerr << "[DEBUG] Group ID: " << fstat->st_gid << std::endl;

    fstat->st_uid = file.get_owner_id();
    std::cerr << "[DEBUG] Owner ID: " << fstat->st_uid << std::endl;

    fstat->st_size = file.get_size();
    std::cerr << "[DEBUG] File size: " << fstat->st_size << " bytes" << std::endl;

    fstat->st_atime = file.get_accessed_at();
    std::cerr << "[DEBUG] Last access time: " << fstat->st_atime << std::endl;

    fstat->st_mtime = file.get_modified_at();
    std::cerr << "[DEBUG] Last modified time: " << fstat->st_mtime << std::endl;

    fstat->st_ctime = file.get_created_at();
    std::cerr << "[DEBUG] Creation time: " << fstat->st_ctime << std::endl;

    fstat->st_nlink = 0;
    std::cerr << "[DEBUG] Number of links: " << fstat->st_nlink << std::endl;

    fstat->st_ino = file.get_inode();
    std::cerr << "[DEBUG] Inode number: " << fstat->st_ino << std::endl;

    fstat->st_dev = 0;
    fstat->st_rdev = 0;
    fstat->st_blksize = 0;

    fstat->st_blocks = file.get_quantity_blocks();
    std::cerr << "[DEBUG] Number of blocks: " << fstat->st_blocks << std::endl;

    std::cerr << "[DEBUG] getattr completed successfully for path: " << path << std::endl;

    return 0;
}

struct fuse_operations *build_fuse_operations() {
    static struct fuse_operations fuse_op;
    memset(&fuse_op, 0, sizeof(fuse_op));

    fuse_op.open = open;
    fuse_op.read = read;
    fuse_op.write = write;
    fuse_op.rename = rename;
    fuse_op.mknod = mknod;
    fuse_op.mkdir = mkdir;
    fuse_op.destroy = destroy;
    fuse_op.readdir = readdir;
    fuse_op.init = init;
    fuse_op.getattr = getattr;

    return &fuse_op;
}
