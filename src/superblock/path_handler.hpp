#ifndef COMMON_OPERATIONS_HPP
#define COMMON_OPERATIONS_HPP
#include <iostream>
#include <sstream>
#include <string>

#include "dentry.hpp"
#include "../inodes/directory.hpp"


class PathHandler {

    private:
        DEntry& dentry;

    public:
        PathHandler(DEntry& dentry);
        InodeType get_inode(const std::string& path);
        InodeType remove_inode(const std::string& path, BlocksManager& bmanager);
};
#endif