#include <string>

#define BLOCK_SIZE 4096

class Env {

    private:
        Env();

    public:
        const std::string& disk_file = std::getenv("DISK_FILE");
        const unsigned long long int disk_size = std::stoi(std::getenv("DISK_SIZE"));
        const unsigned int block_size = std::stoi(std::getenv("BLOCK_SIZE"));
        const unsigned long long int block_quantity = disk_size / block_size;
        const unsigned int inode_slash = 200;
        // cannot allow copy and assigment
        Env(const Env&) = delete;
        Env& operator=(const Env&) = delete;
    
        // singleton pattern
        static const Env& get_instance();
        ~Env();
        

};