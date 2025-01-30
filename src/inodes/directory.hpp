#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <unordered_map>
#include <vector>

#include "fcb.hpp"

#include "../env.hpp"
#include "../superblock/types.hpp"
#include "../superblock/blocks_manager.hpp"

class Directory {
    private:
        File file;
        std::unordered_map<std::string, InodeType> entries;
        std::vector<unsigned char> serialize_entries();
        void load_entries();

    public:
        explicit Directory(BlocksManager& bmanager, struct fuse_file_info* fi, const struct fuse_context* fc);
        explicit Directory(InodeType id);
        explicit Directory(File& file);
        bool create_entry(const std::string& key, InodeType inode);
        InodeType remove_entry(const std::string& key);
        InodeType get_inode(const std::string& key);
        void flush(BlocksManager& bmanager);
};



#endif