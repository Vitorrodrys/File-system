#include <regex>

#include "../env.hpp"
#include "path_handler.hpp"



PathHandler& PathHandler::operator=(const PathHandler &other) {
    if ( this == &other ){
        return *this;
    }
    this->dentry = other.dentry;
    return *this;
}
PathHandler::PathHandler(DEntry &dentry) : dentry(dentry) {}

PathHandler::PathHandler() : dentry(DEntry()) {
    dentry.add_entry("/", Env::get_instance().inode_slash);
}

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
            return (stream.eof()) ? last_inode : NOTFOUNDERROR;
        }
        Directory dir(file);
        last_inode = dir.get_entry(token);
        if (last_inode == 0) {
            return NOTFOUNDERROR;
        }
        dentry.add_entry(subpath, last_inode);
        std::getline(stream, token, '/');
        subpath += "/" + token;
    }
}

InodeType PathHandler::remove_path(const std::string &path, BlocksManager &bmanager) {
    
    std::regex regex(R"(^(.*)/([^/]+)$)");
    std::smatch match;

    std::regex_search(path, match, regex);

    std::string parent = match[1]; // catch the parent directory of the path
    std::string child = match[2]; // catch the last child of path
    InodeType parent_inode;

    if (dentry.remove_entry(path)){
        parent_inode = dentry.get_inode(parent);
    }else{
        parent_inode = get_inode(parent);
    }
    Directory dir(parent_inode);
    InodeType removed = dir.remove_entry(child);
    if (removed == NOTFOUNDERROR) {
        return NOTFOUNDERROR;
    }
    dir.flush(bmanager);
    return removed;

}


bool PathHandler:: add_path(const std::string &path,InodeType inode) {

    if ( path == "/" ){
        dentry.add_entry(path, Env::get_instance().inode_slash);
        return true;
    }
    std::regex regex(R"(^(.*)/([^/]+)$)");
    std::smatch match;

    std::regex_search(path, match, regex);

    const std::string parent = match[1];
    const std::string child = match[2];

    const InodeType inode_parent =get_inode(parent);
    if (inode_parent == NOTFOUNDERROR) {
        return false;
    }

    Directory dir(inode_parent);
    dir.create_entry(child, inode);
    dentry.add_entry(path, inode);

     return true;
}