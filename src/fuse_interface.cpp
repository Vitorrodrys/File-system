#define FUSE_USE_VERSION 31
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>

#include "env.hpp"
#include "fuse_interface.hpp"
#include "superblock/blocks_manager.hpp"
#include "superblock/path_handler.hpp"


BlocksManager bmanager();
PathHandler path_handler(DEntry());

const Env &envs = Env::get_instance();
int open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/teste.txt") != 0) {
        return -ENOENT; // Arquivo não encontrado
    }
    if ((fi->flags & O_ACCMODE) != O_RDONLY) {
        return -EACCES; // Somente leitura
    }
    return 0; // Sucesso
}

struct fuse_operations *build_fuse_operations() {
    static struct fuse_operations fuse_op = (struct fuse_operations){
        .open = open,
        .lookup = NULL,
    };
    return &fuse_op;
}
