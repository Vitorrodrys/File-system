#include <fstream>
#include <iostream>

#include "../env.hpp"
#include "blocks_manager.hpp"
#include "dentry.hpp"

const Env &envs = Env::get_instance();

DEntry ::DEntry() {
    this->inode_map = std::unordered_map<std::string, InodeType>();
    this->quantity_entries = 0;
    this->add_entry("/", envs.inode_slash);
}

DEntry::DEntry(const DEntry &other){
    this->inode_map = other.inode_map;
    this->quantity_entries = other.quantity_entries;
}

DEntry ::~DEntry() { this->inode_map.clear(); }

bool DEntry ::exists(const std::string &path) const {

    auto end_signal = this->inode_map.end();
    // if the element already exists on the map, then return false and do
    // nothing
    if (this->inode_map.find(path) != end_signal) {
        return true;
    }
    return false;
}

bool DEntry ::add_entry(const std::string &path, InodeType inode) {

    if (this->exists(path)) {
        return false;
    }

    this->inode_map[path] = inode;
    this->quantity_entries++;
    return true;
}

bool DEntry ::remove_entry(const std::string &path) {

    if (!this->exists(path)) {
        return false;
    }

    this->inode_map.erase(path);
    this->quantity_entries--;
    return true;
}

InodeType DEntry ::get_inode(const std::string &path) const {

    if (!this->exists(path)) {
        return false;
    }

    auto it = this->inode_map.find(path);

    return it->second;
}
