#include "common_operations.hpp"

InodeType get_inode(const std::string &path, DEntry &dentry) {
    if (dentry.exists(path)) {
        return dentry.get_inode(path);
    }
    std::istringstream stream(path);
    std::string subpath("/");
    std::string token;
    InodeType last_inode = dentry.get_inode(subpath);
    std::getline(stream, token, '/');
    std::getline(stream, token, '/');

    while (true) {
        subpath += token;
        if (!dentry.exists(subpath)) {
            break;
        }
        last_inode = dentry.get_inode(subpath);
        std::getline(stream, token, '/');
    }

    while (true) {
        File dir(last_inode);
        stream = std::getline(stream, token, '/');
        if (dir.get_type() == FileType::TFILE and !std::getline(stream, token, '/')) {
            return last_inode;
        }else if (dir.get_type() == FileType::TFILE and ) {
            return -1;
        }else{
            return -2;
        }
        last_inode = dir.get_inode(token);
        if (last_inode == 0) {
            return 0;
        }
        dentry.add_entry(subpath, last_inode);
        std::getline(stream, token, '/');
        if (token.empty()) {
            return last_inode;
        }
        subpath += "/" + token;
    }
}