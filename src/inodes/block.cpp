#include <fstream>
#include <iostream>

#include "../env.hpp"
#include "fcb.hpp"

void load_block(char *buffer, BlockType block_number){

    std::ifstream infile("disk.bin", std::ios::binary);
    if (!infile) {
        throw std::runtime_error("Could not open file disk.bin");
    }
    infile.seekg(block_number * BLOCK_SIZE);
    infile.read(buffer, BLOCK_SIZE);

}
