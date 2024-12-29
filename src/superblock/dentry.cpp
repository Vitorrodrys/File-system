#include <fstream>
#include <iostream>

#include "dentry.hpp"



DEntry :: DEntry(){
    this->inode_map = std::unordered_map<std::string, InodeType>();
    this->quantity_entries = 0;
}

DEntry :: DEntry(const std::string& binary_filepath){
    this->deserialize_from_bytes(binary_filepath);
}

DEntry :: ~DEntry(){
    this->inode_map.clear();
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
        bytes.insert(bytes.end(), reinterpret_cast<const char*>(&pair.second), reinterpret_cast<const char*>(&pair.second) + sizeof(InodeType));
    }

    return bytes;
}
void DEntry :: deserialize_from_bytes(const std::string& file_name){
    std :: ifstream infile(file_name, std::ios::binary);

    if(!infile){
        throw std :: runtime_error("arquivo da dentry não existe "+file_name);
    }

    unsigned int quantity_archives;
    infile.read(reinterpret_cast<char*>(&quantity_archives), sizeof(unsigned int));


    size_t key_size = 0;
    InodeType value=0;
  
    for (unsigned int i = 0; i < quantity_archives; ++i) {

        infile.read(reinterpret_cast<char*>(&key_size),sizeof(size_t));

        std::string key(key_size, '\0');
        infile.read(&key[0], sizeof(char)*key_size);

        infile.read(reinterpret_cast<char*>(&value), sizeof(InodeType));

        this->inode_map[key] = value;
    }
    this->quantity_entries=quantity_archives;
}





// ------------------ Save & Load operations -----------------------------------------------

void DEntry :: save(const std::string& path) const{
    std::vector<char> serialized_bytes = this->serialize_to_bytes();

    std::ofstream outFile(path, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Could not open file " + path);
    }
    outFile.write(reinterpret_cast<const char*>(serialized_bytes.data()), static_cast<std::streamsize>(serialized_bytes.size()));

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
    this->quantity_entries--;
    return true;

}

InodeType DEntry ::get_inode(const std::string& path) const {

    if(!this->exists(path)){
        return false;
    }


    auto it=this->inode_map.find(path);

    return  it->second;

}

