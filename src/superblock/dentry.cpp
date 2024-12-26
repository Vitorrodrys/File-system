#include <fstream>
#include <iostream>

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
void DEntry :: deserialize_from_bytes(DEntry dentry,const std::string& file_name){
    std :: ifstream infile(file_name, std::ios::binary);

    if(!infile){
        throw std :: runtime_error("arquivo da dentry não existe "+file_name);
    }

    unsigned int quantity_entries=0;
    infile.read(reinterpret_cast<char*>(&quantity_entries), sizeof(quantity_entries));



    int size_chave =0;
    DEntry  entry();
    for (int i = 0; i < quantity_entries; ++i) {

        infile.read(reinterpret_cast<char*>(&size_chave),size_chave(quantity_entries));

        std::string chave(size_chave, '\0');
        infile.read(&chave[0], size_chave);


        InodeType valor=0;
        infile.read(reinterpret_cast<char*>(&valor), sizeof(valor));

        this->inode_map[chave]= valor;
    }



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

