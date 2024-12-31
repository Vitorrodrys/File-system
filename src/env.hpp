#include <string>

#define BLOCK_SIZE 4096

class Env {

    private:
        Env();

    public:
        const std::string binary_disk_filename = std::getenv("BINARY_DISK_FILENAME");
        const unsigned long long int disk_size = std::stoi(std::getenv("DISK_SIZE"));
        const unsigned long long int block_quantity = disk_size / BLOCK_SIZE;
        // cannot allow copy and assigment
        Env(const Env&) = delete;
        Env& operator=(const Env&) = delete;
    
        // singleton pattern
        static const Env& get_instance();
        ~Env();
        

};