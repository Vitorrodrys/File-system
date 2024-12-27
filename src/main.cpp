#include "superblock/blocks_manager.hpp"


int main(){

    BlocksManager manager;
    InodeType a = manager.get_free_block();
    InodeType b = manager.get_free_block();
    manager.release_block(b);
    manager.release_block(a);
    manager.save("test.bin");
    BlocksManager manager2("test.bin");
    InodeType c = manager2.get_free_block();
    InodeType d = manager2.get_free_block();
}
