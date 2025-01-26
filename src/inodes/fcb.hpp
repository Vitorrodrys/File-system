#ifndef FCB
#define FCB

#include "../superblock/types.hpp"
#include "../superblock/blocks_manager.hpp"
#include "../superblock/types.hpp"



class File {

public:
    explicit File(
        BlocksManager& bmanager,
        struct fuse_file_info* fi,
        const struct fuse_context* fc
    );

    explicit File(const struct fuse_file_info* fi);

    size_t read(off_t offset, size_t size, char* buf);
    size_t write(off_t offset, size_t size, const char* buf, BlocksManager& bmanager);
};
#endif