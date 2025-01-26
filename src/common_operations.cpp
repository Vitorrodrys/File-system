#include "common_operations.hpp"

InodeType get_inode(const std::string& path, DEntry& dentry){
    if(dentry.exists(path)){
        return dentry.get_inode(path);
    }
    std::istringstream stream(path);
    std::string subpath("/");
    std::string token;
    InodeType last_inode = dentry.get_inode(subpath);
    std::getline(stream, token, '/');
    std::getline(stream, token, '/');

    while (true){
        subpath += token;
        if(!dentry.exists(subpath)){
            break;
        }
        last_inode = dentry.get_inode(subpath);
        std::getline(stream, token, '/');
    }

    while ( true ){
        Directory dir(last_inode);
        last_inode = dir.get_inode(token);
        if (last_inode == 0){
            return 0;
        }
        dentry.add_entry(subpath, last_inode);
        std::getline(stream, token, '/');
        if (token.empty()){
            return last_inode;
        }
        subpath += "/" + token;
    }
}