#define FUSE_USE_VERSION 31
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "fuse.hpp"

int open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/teste.txt") != 0) {
        return -ENOENT; // Arquivo não encontrado
    }
    if ((fi->flags & O_ACCMODE) != O_RDONLY) {
        return -EACCES; // Somente leitura
    }
    return 0; // Sucesso
}


struct fuse_operations * build_fuse_operations(){
    struct fuse_operations *fuse_op = new struct fuse_operations;
    memset(fuse_op, 0, sizeof(struct fuse_operations));
    *fuse_op = (struct fuse_operations){
        .open = open,
    };
    return fuse_op;
}
