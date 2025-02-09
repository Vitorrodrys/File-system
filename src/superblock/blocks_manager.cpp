#include <cassert>
#include <fstream>
#include <iostream>
#include <string.h>

#include "../env.hpp"
#include "blocks_manager.hpp"

BlocksManager ::~BlocksManager() { delete[] this->free_blocks; }

void BlocksManager ::build() {
    const Env &envs = Env::get_instance();
    this->qblocks_reserveds = (Env::get_instance().block_quantity*sizeof(BlockType)+3*sizeof(BlockType))/BLOCK_SIZE;
    this->first = qblocks_reserveds;
    this->last = envs.block_quantity - 1;
    this->free_blocks = new BlockType[envs.block_quantity];
    for (BlockType i = 0; i < qblocks_reserveds; i++) {
        this->free_blocks[i] = OCCUPIED;
    }
    for (BlockType i = first; i < envs.block_quantity-1; i++) {
        this->free_blocks[i] = i + 1;
    }
    this->free_blocks[envs.block_quantity - 1] = END_OF_LIST;
    // this ensure that the MANAGER block neves will be returned as free
    // for some get_free_block call, as well ensure that the Inode slash
    // never is returned as free block in some get_free_block call
    this->free_blocks[envs.inode_slash-1] = envs.inode_slash + 1;
    this->free_blocks[envs.inode_slash] = OCCUPIED;
}

void BlocksManager ::load() {
    const Env &envs = Env::get_instance();
    const std::string &path = envs.disk_file;
    std::ifstream infile(path, std::ios::binary);
    if (!infile) {
        throw std::runtime_error("Could not open file " + path);
    }
    infile.seekg(0);
    this->free_blocks = new BlockType[envs.block_quantity];
    infile.read(reinterpret_cast<char *>(&this->qblocks_reserveds), sizeof(BlockType));
    infile.read(reinterpret_cast<char *>(&this->first), sizeof(BlockType));
    infile.read(
        reinterpret_cast<char *>(this->free_blocks),
        static_cast<std::streamsize>(envs.block_quantity * sizeof(BlockType)));
    infile.read(reinterpret_cast<char *>(&this->last), sizeof(BlockType));
}
BlocksManager::BlocksManager() : free_blocks(nullptr), first(0), last(0), qblocks_reserveds(0) {}

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
    assert(block >= qblocks_reserveds && block != envs.inode_slash && block != OCCUPIED && block != END_OF_LIST);
    return block;
}

void BlocksManager ::release_block(BlockType block) {
    if (block >= Env::get_instance().block_quantity) {
        throw std::runtime_error("Invalid block to release");
    }
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
    outfile.seekp(0);
    outfile.write(reinterpret_cast<const char *>(&this->qblocks_reserveds),
                  sizeof(BlockType));
    outfile.write(reinterpret_cast<const char *>(&this->first),
                  sizeof(BlockType));
    outfile.write(
        reinterpret_cast<const char *>(this->free_blocks),
        static_cast<std::streamsize>(envs.block_quantity * sizeof(BlockType)));
    outfile.write(reinterpret_cast<const char *>(&this->last),
                  sizeof(BlockType));
}