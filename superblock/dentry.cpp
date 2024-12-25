#include <fstream>

#include "superblock/dentry.hpp"



DEntry :: DEntry(){
    this->inode_map = std::unordered_map<std::string, InodeType>();
    this->quantity_entries = 0;
}


// ------------------- Serialize & Deserialize operations -----------------------------------
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



// ------------------ Save & Load operations -----------------------------------------------

void DEntry :: save(const std::string& path) const{
    std::vector<char> serialized_bytes = this->serialize_to_bytes();

    std::ofstream outFile(path, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Could not open file " + path);
    }
    outFile.write(reinterpret_cast<const char*>(serialized_bytes.data()), serialized_bytes.size());

}


