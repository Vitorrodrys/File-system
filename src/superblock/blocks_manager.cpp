#include <fstream>
#include <iostream>

#include "blocks_manager.hpp"

BlocksManager :: BlocksManager(){
    this->first = 0;
    this->last = envs.block_quantity - 1;
    this->free_blocks = new InodeType[envs.block_quantity];
    for (InodeType i = 0; i < envs.block_quantity-1; i++){
        this->free_blocks[i] = i+1;
    }
    this->free_blocks[envs.block_quantity-1] = END_OF_LIST;
}
BlocksManager :: ~BlocksManager(){
    delete[] this->free_blocks;
}

BlocksManager :: BlocksManager(const std::string& path){
    std::ifstream infile(path, std::ios::binary);
    if (!infile) {
        throw std::runtime_error("Could not open file " + path);
    }
    infile.read(reinterpret_cast<char*>(&this->first), sizeof(InodeType));
    infile.read(reinterpret_cast<char*>(this->free_blocks), envs.block_quantity * sizeof(InodeType));
    infile.read(reinterpret_cast<char*>(&this->last), sizeof(InodeType));
}

InodeType BlocksManager :: get_free_block(){
    if (this->first == -1){
        return END_OF_LIST;
    }
    InodeType block = this->first;
    this->first = this->free_blocks[block];
    this->free_blocks[block] = OCCUPIED;
    return block;
}

void BlocksManager :: release_block(InodeType block) {
    this->free_blocks[this->last] = block;
    this->free_blocks[block] = END_OF_LIST;
    this->last = block;
}

void BlocksManager :: save(const std::string& path) const{
    std::ofstream outfile(path, std::ios::binary);
    if (!outfile) {
        throw std::runtime_error("Could not open file " + path);
    }
    outfile.write(reinterpret_cast<const char*>(&this->first), sizeof(InodeType));
    outfile.write(reinterpret_cast<const char*>(this->free_blocks), envs.block_quantity * sizeof(InodeType));
    outfile.write(reinterpret_cast<const char*>(&this->last), sizeof(InodeType));
}