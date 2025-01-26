#include <string>


typedef unsigned long long int InodeType;

const InodeType END_OF_LIST = Env::get_instance().block_quantity+1;
const InodeType OCCUPIED = Env::get_instance().block_quantity+2;

class BlocksManager {

    private:
        InodeType first;
        InodeType last;
        InodeType* free_blocks;

        void init();

    public:
        explicit BlocksManager(const BlocksManager& other);
        explicit BlocksManager();
        ~BlocksManager();

        InodeType get_free_block();
        void release_block(InodeType block);
        void save() const;

};