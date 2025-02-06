#include <fstream>
#include <iostream>
#include <string.h>

#include "../env.hpp"
#include "blocks_manager.hpp"

#define MANAGER_INODE_NUMBER 1
const Env &bm_envs = Env::get_instance();

BlocksManager ::~BlocksManager() { delete[] this->free_blocks; }

void BlocksManager ::build() {
    this->first = 0;
    this->last = bm_envs.block_quantity - 1;
    this->free_blocks = new BlockType[bm_envs.block_quantity];
    for (BlockType i = 0; i < bm_envs.block_quantity - 1; i++) {
        this->free_blocks[i] = i + 1;
    }
    this->free_blocks[bm_envs.block_quantity - 1] = END_OF_LIST;
}

void BlocksManager ::load() {
    const std::string &path = bm_envs.disk_file;
    std::ifstream infile(path, std::ios::binary);
    if (!infile) {
        throw std::runtime_error("Could not open file " + path);
    }
    infile.seekg(MANAGER_INODE_NUMBER * BLOCK_SIZE);
    this->free_blocks = new BlockType[bm_envs.block_quantity];
    infile.read(reinterpret_cast<char *>(&this->first), sizeof(BlockType));
    infile.read(
        reinterpret_cast<char *>(this->free_blocks),
        static_cast<std::streamsize>(bm_envs.block_quantity * sizeof(BlockType)));
    infile.read(reinterpret_cast<char *>(&this->last), sizeof(BlockType));
}
BlocksManager ::BlocksManager() {}

BlocksManager& BlocksManager::operator=(const BlocksManager &other) {
    if (this == &other) {
        return *this;
    }
    this->first = other.first;
    this->last = other.last;
    this->free_blocks = new BlockType[bm_envs.block_quantity];
    memcpy(this->free_blocks, other.free_blocks,
           bm_envs.block_quantity * sizeof(BlockType));
    return *this;
}

BlockType BlocksManager ::get_free_block() {
    if (this->first == END_OF_LIST) {
        return END_OF_LIST;
    }
    BlockType block = this->first;
    this->first = this->free_blocks[block];
    this->free_blocks[block] = OCCUPIED;
    return block;
}

void BlocksManager ::release_block(BlockType block) {
    this->free_blocks[this->last] = block;
    this->free_blocks[block] = END_OF_LIST;
    this->last = block;
}

void BlocksManager ::save() const {
    const std::string &path = bm_envs.disk_file;
    std::ofstream outfile(path, std::ios::binary);
    if (!outfile) {
        throw std::runtime_error("Could not open file " + path);
    }
    outfile.seekp(MANAGER_INODE_NUMBER * BLOCK_SIZE);
    outfile.write(reinterpret_cast<const char *>(&this->first),
                  sizeof(BlockType));
    outfile.write(
        reinterpret_cast<const char *>(this->free_blocks),
        static_cast<std::streamsize>(bm_envs.block_quantity * sizeof(BlockType)));
    outfile.write(reinterpret_cast<const char *>(&this->last),
                  sizeof(BlockType));
}