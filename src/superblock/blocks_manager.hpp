#include <string>

#include "../env.hpp"
#include "../inodes/fcb.hpp"

const BlockType END_OF_LIST = Env::get_instance().block_quantity+1;
const BlockType OCCUPIED = Env::get_instance().block_quantity+2;

class BlocksManager {

    private:
        BlockType first;
        BlockType last;
        BlockType* free_blocks;

    public:
        explicit BlocksManager();
        explicit BlocksManager(const BlocksManager& other);
        explicit BlocksManager(const std::string& path);
        ~BlocksManager();

        BlockType get_free_block();
        void release_block(BlockType block);
        void save(const std::string& path) const;

};