#ifndef COMMON_OPERATIONS_HPP
#define COMMON_OPERATIONS_HPP
#include <iostream>
#include <sstream>
#include <string>

#include "superblock/dentry.hpp"
#include "inodes/directory.hpp"


InodeType get_inode(const std::string& path, DEntry& dentry);
#endif