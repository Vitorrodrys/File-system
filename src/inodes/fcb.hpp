#ifndef FCB_HPP
#define FCB_HPP

#include <fstream>

#include "../superblock/blocks_manager.hpp"
#include "../superblock/types.hpp"

#define MAX_POINTERS 12
#define BLOCK_DSIZE (BLOCK_SIZE - sizeof(unsigned short))

enum class FileType {
    TFILE,
    TDIRECTORY,
    THLINK,
    TSLINK
};

typedef struct HeaderIndexs{
    BlockType data_inodes[MAX_POINTERS-1];
    BlockType single_indirect;
}HeaderIndexs;

typedef struct FcbInode {
    char name[256];
    InodeType id;
    FileType type;
    off_t size;
    off_t blocks;
    uid_t owner;
    gid_t group;
    mode_t permissions;
    time_t created_at;
    time_t modified_at;
    time_t accessed_at;
    HeaderIndexs headers;
} INODEFCB;



typedef struct DataInode {
    unsigned short current_size;
    char data[BLOCK_SIZE - BLOCK_DSIZE];
} DataInode;

class File {

    protected:
        std::fstream file;
        FcbInode fcb;

    protected:
        bool fill_indirect_header(HeaderIndexs& ind_header, BlockType new_block);
        BlockType add_block(BlocksManager& bmanager);
        void remove_unused_blocks(BlockType last_used, BlocksManager& bmanager);
        
    private:
        void update_fcb();
        BlockType load_indirect(BlockType block, HeaderIndexs* indirect_inode);
        BlockType get_block_value(uint32_t index);
        bool load_data_block(BlockType block, DataInode* inoded);
        size_t write_extra(size_t size, const char* buf, BlocksManager& bmanager);

    public:
        explicit File(
            BlocksManager& bmanager, struct fuse_file_info* fi,
            const struct fuse_context* fc, FileType type = FileType::TFILE
        );
        explicit File(InodeType id);
        explicit File(const File &other);

        FileType get_type() const;
        off_t get_size() const;
        size_t read(off_t offset, size_t size, char* buf);
        size_t write(off_t offset, size_t size, const char* buf, BlocksManager& bmanager);
};

#endif