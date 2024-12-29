#include <string>

#include "../env.hpp"


const Env& envs = Env::get_instance();

typedef unsigned long long int InodeType;

const InodeType END_OF_LIST = envs.block_quantity+1;
const InodeType OCCUPIED = envs.block_quantity+2;

class BlocksManager {

    private:
        InodeType first;
        InodeType last;
        InodeType* free_blocks;

    public:
        explicit BlocksManager();
        explicit BlocksManager(const BlocksManager& other);
        explicit BlocksManager(const std::string& path);
        ~BlocksManager();

        InodeType get_free_block();
        void release_block(InodeType block);
        void save(const std::string& path) const;

};