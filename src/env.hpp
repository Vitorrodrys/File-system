#include <string>
#ifndef ENV_HPP
#define ENV_HPP

#define BLOCK_SIZE 4096

class Env {

    private:
        Env() : 
            disk_file(std::getenv("DISK_FILE")),
            disk_size(std::stoull(std::getenv("DISK_SIZE"))),
            block_quantity(disk_size / BLOCK_SIZE),
            inode_slash(block_quantity - 5)   
        {}

    public:
        const std::string disk_file;
        const unsigned long long int disk_size;
        const unsigned long long int block_quantity;
        const uint64_t inode_slash;
        // cannot allow copy and assigment
        Env(const Env&) = delete;
        Env& operator=(const Env&) = delete;
    
        // singleton pattern
        static const Env& get_instance();
        ~Env();
        

};

#endif