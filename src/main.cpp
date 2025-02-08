#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include "fuse_interface.hpp"

int main(int argc, char *argv[]) {

    const struct fuse_operations *fuseop = build_fuse_operations();
    return fuse_main(argc, argv, fuseop, 0);
}
