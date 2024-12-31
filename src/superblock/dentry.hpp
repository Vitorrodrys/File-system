#ifndef DENTRY_HPP
#define DENTRY_HPP
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include "inodes/fcb.hpp"

class DEntry {
    public:
        explicit DEntry();
        explicit DEntry(const std::string& binary_filepath);
        ~DEntry();
 
        void save(const std::string& path) const;    

        bool add_entry(const std::string& path, InodeType inode);
        bool remove_entry(const std::string& path);
        InodeType get_inode(const std::string& path) const;
        bool exists(const std::string& path) const;

    private:
        std::unordered_map<std::string, InodeType> inode_map;
        unsigned int quantity_entries;

        std::vector<char> serialize_to_bytes() const;
        void deserialize_from_bytes(const std::string& data);
};

#endif
