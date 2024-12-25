#include "superblock/dentry.hpp"




std::vector<char> DEntry::serialize_to_bytes() const {
    std::vector<char> bytes;

    for (const auto& pair : this->inode_map) {
        // Serializando a chave
        size_t key_size = sizeof(std::string);
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&key_size), reinterpret_cast<const char*>(&key_size + sizeof(std::string)));
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&pair.first), reinterpret_cast<const char*>(&pair.first + sizeof(std::string)));

        // Serializando o valor
        size_t value_size = sizeof(InodeType);
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&value_size), reinterpret_cast<const char*>(&value_size + sizeof(InodeType)));
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&pair.second), reinterpret_cast<const char*>(&pair.second + sizeof(InodeType)));
    }

    return bytes;
}



