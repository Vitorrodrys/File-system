#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <unordered_map>
#include <vector>

#include "fcb.hpp"

#include "../env.hpp"
#include "../superblock/types.hpp"
#include "../superblock/blocks_manager.hpp"

class Directory : protected File {
    private:
        std::unordered_map<std::string, InodeType> entries;

        std::vector<unsigned char> serialize_entries();

    public:
        explicit Directory(BlocksManager& bmanager, struct fuse_file_info* fi, const struct fuse_context* fc);
        explicit Directory(const struct fuse_file_info* fi);
        bool create_entry(const char* name, InodeType inode, BlocksManager& bmanager);
        BlockType remove_entry(const char* name, BlocksManager& bmanager);
        InodeType get_inode(const char* name);
        void flush(BlocksManager& bmanager);
};



#endif