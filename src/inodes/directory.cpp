#include "directory.hpp"
#include <iostream>


Directory::Directory(
    const std::string& name,
    mode_t permissions,
    BlocksManager &bmanager,
    const struct fuse_context *fc
) : file(name, permissions, bmanager, fc, FileType::TDIRECTORY) {}

Directory::Directory(InodeType id) : file(id) {
    load_entries();
}
Directory::Directory(const File &file) : file(file) {
    load_entries();
}
Directory::~Directory() {}

void Directory::load_entries() {
    off_t file_size = file.get_size();
    std::vector<char> buf(file_size);
    file.read(0, file_size, buf.data());

    int index = 0;
    while (index < file_size) {
        std::string key(buf.data() + index);
        index += key.length() + 1;

        if (index > file_size){
            throw std::runtime_error("Corrupted directory file: invalid entry format");
        }
        std::string inode_str(buf.data() + index);
        index += inode_str.length() + 1;
        if (index > file_size){
            throw std::runtime_error("Corrupted directory file: invalid entry format");
        }

        entries[key] = static_cast<InodeType>(std::stoi(inode_str));
        std::cerr << "[DEBUG] loading directory entry: " << key << std::endl;
    }
}
std::vector<unsigned char> Directory::serialize_entries() const {
    std::vector<unsigned char> buffer;
    for (auto &entry : entries) {
        buffer.insert(buffer.end(), entry.first.begin(), entry.first.end());
        buffer.push_back('\0');

        std::string entry_str = std::to_string(entry.second);
        buffer.insert(buffer.end(), entry_str.begin(), entry_str.end());
        buffer.push_back('\0');
    }
    return buffer;
}

bool Directory::create_entry(const std::string &key, InodeType inode) {
    if (entries.contains(key)) {
        return false;
    }
    entries[key] = inode;
    return true;
}

InodeType Directory::remove_entry(const std::string &key) {
    if (not entries.contains(key)) {
        return NOTFOUNDERROR;
    }
    const InodeType inode = entries[key];
    entries.erase(key);
    return inode;
}
void Directory::delete_dir(BlocksManager& bmanager){
    file.delete_file(bmanager);
}
bool Directory::empty() const{
    return entries.empty();
}
void Directory::flush(BlocksManager &bmanager) {
    std::vector<unsigned char> buffer = serialize_entries();
    file.write(0, buffer.size(), reinterpret_cast<const char *>(buffer.data()), bmanager);
    if (static_cast<std::vector<unsigned char>::size_type>(file.get_size()) > buffer.size()){
        file.truncate(buffer.size(), bmanager);
    }
}

InodeType Directory::get_entry(const std::string &key) {
    if (not entries.contains(key)) {
        return NOTFOUNDERROR;
    }
    return entries[key];
}

InodeType Directory::get_inode() const{
    return file.get_inode();
}
