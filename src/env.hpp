#include <string>

class Env {

    private:
        Env();

    public:
        const unsigned long long int disk_size = std::stoi(std::getenv("DISK_SIZE"));
        const unsigned int block_size = std::stoi(std::getenv("BLOCK_SIZE"));
        const unsigned long long int block_quantity = disk_size / block_size;
        // cannot allow copy and assigment
        Env(const Env&) = delete;
        Env& operator=(const Env&) = delete;
    
        // singleton pattern
        static const Env& get_instance();
        ~Env();
        

};