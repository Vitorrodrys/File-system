#include <fstream>
#include <iostream>
#include <string.h>

#include "blocks_manager.hpp"


const Env& envs = Env::get_instance();

BlocksManager :: BlocksManager(){
    this->first = 0;
    this->last = envs.block_quantity - 1;
    this->free_blocks = new BlockType[envs.block_quantity];
    for (BlockType i = 0; i < envs.block_quantity-1; i++){
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
    this->free_blocks = new BlockType[envs.block_quantity];
    infile.read(reinterpret_cast<char*>(&this->first), sizeof(BlockType));
    infile.read(reinterpret_cast<char*>(this->free_blocks), static_cast<std::streamsize>(envs.block_quantity * sizeof(BlockType)));
    infile.read(reinterpret_cast<char*>(&this->last), sizeof(BlockType));
}

BlocksManager:: BlocksManager(const BlocksManager& other){
    this->first = other.first;
    this->last = other.last;
    this->free_blocks = new BlockType[envs.block_quantity];
    memcpy(this->free_blocks, other.free_blocks, envs.block_quantity * sizeof(BlockType));
}

BlockType BlocksManager :: get_free_block(){
    if (this->first == END_OF_LIST){
        return END_OF_LIST;
    }
    BlockType block = this->first;
    this->first = this->free_blocks[block];
    this->free_blocks[block] = OCCUPIED;
    return block;
}

void BlocksManager :: release_block(BlockType block) {
    this->free_blocks[this->last] = block;
    this->free_blocks[block] = END_OF_LIST;
    this->last = block;
}

void BlocksManager :: save(const std::string& path) const{
    std::ofstream outfile(path, std::ios::binary);
    if (!outfile) {
        throw std::runtime_error("Could not open file " + path);
    }
    outfile.write(reinterpret_cast<const char*>(&this->first), sizeof(BlockType));
    outfile.write(reinterpret_cast<const char*>(this->free_blocks), static_cast<std::streamsize>(envs.block_quantity * sizeof(BlockType)));
    outfile.write(reinterpret_cast<const char*>(&this->last), sizeof(BlockType));
}