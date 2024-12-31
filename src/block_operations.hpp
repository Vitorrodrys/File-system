#include <string>


typedef unsigned long long int BlockType;

class BlockOperations{

    public:
        explicit BlockOperations(std::string binary_disk_path)
        : binary_disk_path(binary_disk_path) {}
        ~BlockOperations();

        /**
         * @brief write a set of bytes (max 4096 ) into disk at a specific block
         * specified by block_number
         * 
         * This method can be use to store a block in disk at a specific block
         * position
         * block_number: the block number which you wish to write the data
         * data: A array of bytes with the maximum size being 4096, containing the
         * datas that you wish to store, if given a array bigger than 4096 bytes, then
         * just 4096 bytes will be stored.
         */
        void write_block(BlockType block_number, char* data) const;

        /**
         * @brief read a block from disk at a specific block number, store
         * the data read into the data array, this array must have at least 4096 bytes
         * and you need to allocate it before calling this method
         */
        void read_block(BlockType block_number, char* data) const;

    private:
        std::string binary_disk_path;
        
};
