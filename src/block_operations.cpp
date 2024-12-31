#include <fstream>
#include <iostream>

#include "block_operations.hpp"
#include "env.hpp"

BlockOperations :: ~BlockOperations() {}

void BlockOperations :: write_block(BlockType block_number, char* data) const {
    std::ofstream disk(this->binary_disk_path, std::ios::binary);
    if (!disk) {
        throw std::runtime_error("Could not open file " + this->binary_disk_path);
    }
    disk.seekp(block_number * BLOCK_SIZE);
    disk.write(data, BLOCK_SIZE);
}
void BlockOperations :: read_block(BlockType block_number, char* data) const {
    std::ifstream disk(this->binary_disk_path, std::ios::binary);
    if (!disk) {
        throw std::runtime_error("Could not open file " + this->binary_disk_path);
    }
    disk.seekg(block_number * BLOCK_SIZE);
    disk.read(data, BLOCK_SIZE);
}
