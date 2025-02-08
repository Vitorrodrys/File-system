#include <cassert>
#include <fstream>
#include <iostream>
#include <string.h>

#include "../env.hpp"
#include "blocks_manager.hpp"

#define MANAGER_INODE_NUMBER 0

BlocksManager ::~BlocksManager() { delete[] this->free_blocks; }

void BlocksManager ::build() {
    const Env &envs = Env::get_instance();
    this->first = 1;
    this->last = envs.block_quantity - 1;
    this->free_blocks = new BlockType[envs.block_quantity];
    for (BlockType i = 1; i < envs.block_quantity - 1; i++) {
        this->free_blocks[i] = i + 1;
    }
    this->free_blocks[envs.block_quantity - 1] = END_OF_LIST;
    // this ensure that the MANAGER block neves will be returned as free
    // for some get_free_block call, as well ensure that the Inode slash
    // never is returned as free block in some get_free_block call
    this->free_blocks[envs.inode_slash-1] = envs.inode_slash + 1;
    this->free_blocks[MANAGER_INODE_NUMBER] = OCCUPIED;
    this->free_blocks[envs.inode_slash] = OCCUPIED;
}

void BlocksManager ::load() {
    const Env &envs = Env::get_instance();
    const std::string &path = envs.disk_file;
    std::ifstream infile(path, std::ios::binary);
    if (!infile) {
        throw std::runtime_error("Could not open file " + path);
    }
    infile.seekg(MANAGER_INODE_NUMBER * BLOCK_SIZE);
    this->free_blocks = new BlockType[envs.block_quantity];
    infile.read(reinterpret_cast<char *>(&this->first), sizeof(BlockType));
    infile.read(
        reinterpret_cast<char *>(this->free_blocks),
        static_cast<std::streamsize>(envs.block_quantity * sizeof(BlockType)));
    infile.read(reinterpret_cast<char *>(&this->last), sizeof(BlockType));
}
BlocksManager ::BlocksManager() {}

BlocksManager& BlocksManager::operator=(const BlocksManager &other) {
    const Env &envs = Env::get_instance();
    if (this == &other) {
        return *this;
    }
    this->first = other.first;
    this->last = other.last;
    this->free_blocks = new BlockType[envs.block_quantity];
    memcpy(this->free_blocks, other.free_blocks,
           envs.block_quantity * sizeof(BlockType));
    return *this;
}

BlockType BlocksManager ::get_free_block() {
    const Env& envs = Env::get_instance();
    if (this->first == END_OF_LIST) {
        return END_OF_LIST;
    }
    BlockType block = this->first;
    this->first = this->free_blocks[block];
    this->free_blocks[block] = OCCUPIED;
    assert(block != MANAGER_INODE_NUMBER && block != envs.inode_slash && block != OCCUPIED && block != END_OF_LIST);
    return block;
}

void BlocksManager ::release_block(BlockType block) {
    this->free_blocks[this->last] = block;
    this->free_blocks[block] = END_OF_LIST;
    this->last = block;
}

void BlocksManager ::save() const {
    const Env &envs = Env::get_instance();
    const std::string &path = envs.disk_file;
    std::ofstream outfile(path, std::ios::binary);
    if (!outfile) {
        throw std::runtime_error("Could not open file " + path);
    }
    outfile.seekp(MANAGER_INODE_NUMBER * BLOCK_SIZE);
    outfile.write(reinterpret_cast<const char *>(&this->first),
                  sizeof(BlockType));
    outfile.write(
        reinterpret_cast<const char *>(this->free_blocks),
        static_cast<std::streamsize>(envs.block_quantity * sizeof(BlockType)));
    outfile.write(reinterpret_cast<const char *>(&this->last),
                  sizeof(BlockType));
}