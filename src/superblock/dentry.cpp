#include <fstream>

#include "dentry.hpp"



DEntry :: DEntry(){
    this->inode_map = std::unordered_map<std::string, InodeType>();
    this->quantity_entries = 0;
}


// ------------------- Serialize & Deserialize operations -----------------------------------
std::vector<char> DEntry::serialize_to_bytes() const {
    std::vector<char> bytes;

    //store at the first 4 bytes the quantity of entries exists on hashmap
    bytes.insert(
        bytes.end(),
        reinterpret_cast<const char*>(&this->quantity_entries),
        reinterpret_cast<const char*>(&this->quantity_entries) + sizeof(unsigned int)
    );
    for (const auto& pair : this->inode_map) {
        //store the key_size followed by the key string in bytes vector
        size_t key_size = pair.first.size();
        bytes.insert(
            bytes.end(),
            reinterpret_cast<const char*>(&key_size),
            reinterpret_cast<const char*>(&key_size) + sizeof(size_t)
        );
        bytes.insert(
            bytes.end(),
            pair.first.begin(),
            pair.first.end()
        );

        //store the value into the bytes vector
        size_t value_size = sizeof(InodeType);
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&pair.second), reinterpret_cast<const char*>(&pair.second) + sizeof(InodeType));
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


bool DEntry ::exists(const std::string& path) const {

    auto end_signal = this->inode_map.end();
    // if the element already exists on the map, then return false and do nothing
    if (this->inode_map.find(path) != end_signal ){
        return true;
    }
    return false;
}

bool DEntry :: add_entry(const std::string& path, InodeType inode){

    if(this->exists(path)){
       return false;
    }

    this->inode_map[path] = inode;
    this->quantity_entries++;
    return true;
}

bool DEntry ::remove_entry(const std::string& path) {

    if(!this->exists(path)){
        return false;
    }

    this->inode_map.erase(path);

}

InodeType DEntry ::get_inode(const std::string& path) const {

    if(!this->exists(path)){
        return false;
    }


    auto it=this->inode_map.find(path);

    return  it->second;

}

