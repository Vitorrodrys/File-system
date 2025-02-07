#include <fuse.h>
#include "fuse_interface.hpp"

int main(int argc, char *argv[]) {


    init(nullptr);    
    const struct fuse_operations *fuseop = build_fuse_operations();
    return fuse_main(argc, argv, fuseop);
}
