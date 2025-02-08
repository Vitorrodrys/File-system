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
        std::vector<unsigned char> serialize_entries() const;
        void load_entries();

    public:
        explicit Directory(const std::string& name, mode_t permissions, BlocksManager& bmanager, const struct fuse_context* fc);
        explicit Directory(InodeType id);
        explicit Directory(const File& file);
        ~Directory();
        bool create_entry(const std::string& key, InodeType inode);
        InodeType remove_entry(const std::string& key);
        InodeType get_entry(const std::string& key);
        InodeType get_inode() const;
        void flush(BlocksManager& bmanager);

        // Allow to iterate by directory entries
        using Iterator = std::unordered_map<std::string, InodeType>::iterator;
        using ConstIterator = std::unordered_map<std::string, InodeType>::const_iterator;

        Iterator begin() { return entries.begin(); }
        Iterator end() { return entries.end(); }

        ConstIterator begin() const { return entries.begin(); }
        ConstIterator end() const { return entries.end(); }

        ConstIterator cbegin() const { return entries.cbegin(); }
        ConstIterator cend() const { return entries.cend(); }
};



#endif