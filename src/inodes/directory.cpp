#include "directory.hpp"

#include "../env.hpp"

const Env& env = Env::get_instance();


Directory::Directory(
    BlocksManager& bmanager,
    struct fuse_file_info *fi,
    const struct fuse_context *fc
) :
    File(bmanager, fi, fc)
{
    fcb.type = FileType::TDIRECTORY;
}

Directory::Directory(InodeType id) : File(id) {
    char *buf = new char[fcb.size];
    read(0, fcb.size, buf);
    
    int index = 0;

    while (index < fcb.size) {
        std::string key(buf + index);
        index += key.length() + 1;

        std::string inode_str(buf + index);
        index += inode_str.length() + 1;

        entries[key] = static_cast<InodeType>(std::stoi(inode_str));
    }

    delete[] buf;
}

std::vector<unsigned char> Directory::serialize_entries() {
    std::vector<unsigned char> buffer;
    for (auto& entry : entries) {
        buffer.insert(buffer.end(), entry.first.begin(), entry.first.end());
        buffer.push_back('\0');

        std::string entry_str = std::to_string(entry.second);
        buffer.insert(buffer.end(), entry_str.begin(), entry_str.end());
        buffer.push_back('\0');
    }
    return buffer;
}

bool Directory::create_entry(const std::string& key, InodeType inode) {
    if (entries.find(key) != entries.end()) {
        return false;
    }
    entries[key] = inode;
    return true;
}

InodeType Directory::remove_entry(const std::string& key) {
    if (entries.find(key) == entries.end()) {
        return 0;
    }
    InodeType inode = entries[key];
    entries.erase(key);
    return inode;
}
void Directory::flush(BlocksManager& bmanager) {
    std::vector<unsigned char> buffer = serialize_entries();
    write(0, buffer.size(), reinterpret_cast<char*>(buffer.data()), bmanager);
}

InodeType Directory::get_inode(const std::string& key) {
    if (entries.find(key) == entries.end()) {
        return 0;
    }
    return entries[key];
}
