#ifndef COMMON_OPERATIONS_HPP
#define COMMON_OPERATIONS_HPP
#include <iostream>
#include <sstream>
#include <string>

#include "dentry.hpp"
#include "../inodes/directory.hpp"


class PathHandler {

    private:
        DEntry dentry;

    public:
        explicit PathHandler(DEntry& dentry);
        explicit PathHandler();
        InodeType get_inode(const std::string& path);

        /* Delete a path from dentry, and remove the last subpath of the path from
        your parent directory, returns the inode corresponding to the path erased
         args:
            path: string -> a path to file that you want to delete
            bmanager: BlocksManager -> a reference to the blocks manager object
        */
        InodeType remove_path(const std::string& path, BlocksManager& bmanager);
        bool PathHandler:: add_path(const std::string &path,InodeType inode) ;
};
#endif