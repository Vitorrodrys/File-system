#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include "fcb.hpp"

class Directory : protected File {
    private:
        std::unordered_map<std::string, InodeType> entries;

    public:
        Directory(BlocksManager& bmanager, struct fuse_file_info* fi, const struct fuse_context* fc);
        Directory(const struct fuse_file_info* fi);
        bool create_file(const char* name);
};



#endif