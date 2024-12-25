#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstring> 

#ifndef DENTRY_HPP
#define DENTRY_HPP

typedef unsigned long long int InodeType;

class DEntry {
    public:
        DEntry();
        ~DEntry();
        static DEntry& load(const char& path); // TODO: check if we really will be receive a path file here
        void save(const char& path); // TODO: check if we really will be receive a path file here
        bool add_entry(const char& path, InodeType inode );
        bool remove_entry(const char& path);
        InodeType get_inode(const char& path);
        bool is_directory(const char& path);

    private:
        //we will needs have more things here, such as the listing of free inodes, or other 
        //data structs to help us to manage us superblock
        std::unordered_map<std::string, unsigned long long int> inode_map;
        unsigned int quantity_entries;

        std::vector<char> serialize_to_bytes() const;
        static void deserialize_from_bytes();


}

#endif
