#ifndef DENTRY_HPP
#define DENTRY_HPP
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>


#include "types.hpp"

class DEntry {
    public:
        explicit DEntry();
        DEntry(const DEntry &other);
        ~DEntry();
 

        bool add_entry(const std::string& path, InodeType inode);
        bool remove_entry(const std::string& path);
        InodeType get_inode(const std::string& path) const;
        bool exists(const std::string& path) const;

    private:
        std::unordered_map<std::string, InodeType> inode_map;
        unsigned int quantity_entries;
};

#endif
