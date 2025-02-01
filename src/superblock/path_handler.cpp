#include <regex>

#include "path_handler.hpp"



PathHandler::PathHandler(DEntry &dentry) : dentry(dentry) {}

PathHandler::PathHandler() : dentry(DEntry()) {}

InodeType PathHandler::get_inode(const std::string &path) {
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
        File file(last_inode);
        if (file.get_type() != FileType::TDIRECTORY) {
            return (stream.eof()) ? last_inode : NOTADIRECTORYERROR;
        }
        Directory dir(file);
        last_inode = dir.get_inode(token);
        if (last_inode == 0) {
            return NOTFOUNDERROR;
        }
        dentry.add_entry(subpath, last_inode);
        std::getline(stream, token, '/');
        subpath += "/" + token;
    }
}

InodeType PathHandler::remove_inode(const std::string &path, BlocksManager &bmanager) {
    
    std::regex last_path_part("[^/]+$");
    std::string parent = std::regex_replace(path, last_path_part, "");
    InodeType parent_inode;

    if (dentry.remove_entry(path)){
        parent_inode = dentry.get_inode(parent);
    }else{
        parent_inode = get_inode(parent);
    }
    Directory dir(parent_inode);
    InodeType removed = dir.remove_entry(path);
    if (removed == NOTFOUNDERROR) {
        return NOTFOUNDERROR;
    }
    dir.flush(bmanager);
    return removed;

}