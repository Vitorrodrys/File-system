#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#ifndef DENTRY_HPP
#define DENTRY_HPP

typedef unsigned long long int InodeType;

class DEntry {
    public:
        explicit DEntry();
        ~DEntry();

        // TODO: check if we really needs receive a path here
        static DEntry& load(const std::string& path);  
        void save(const std::string& path) const;    

        bool add_entry(const std::string& path, InodeType inode);
        bool remove_entry(const std::string& path);
        InodeType get_inode(const std::string& path) const;
        bool exists(const std::string& path) const;

    private:
        std::unordered_map<std::string, InodeType> inode_map;
        unsigned int quantity_entries;

        std::vector<char> serialize_to_bytes() const;
        static void deserialize_from_bytes(const std::string& data);
};

#endif
