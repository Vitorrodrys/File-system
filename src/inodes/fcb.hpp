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
    char data[BLOCK_DSIZE];
} DataInode;

class File {

    protected:
        std::fstream file;
        FcbInode fcb;

    protected:
        static bool fill_indirect_header(HeaderIndexs& ind_header, BlockType new_block);
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
            mode_t mode,
            FileType type,
            InodeType inode,
            uid_t uid,
            gid_t gid
        );
        explicit File(
            const std::string& name,
            mode_t mode,
            BlocksManager& bmanager,
            const struct fuse_context* fc,
            FileType type = FileType::TFILE
        );
        explicit File(InodeType id);
        explicit File(const File &other);
        ~File();

        FileType get_type() const;
        off_t get_size() const;
        mode_t get_permissions() const { return fcb.permissions; }
        gid_t get_group_id() const { return fcb.group; }
        uid_t get_owner_id() const { return fcb.owner; }
        time_t get_created_at() const { return fcb.created_at; }
        time_t get_modified_at() const { return fcb.modified_at; }
        time_t get_accessed_at() const { return fcb.accessed_at; }
        off_t get_quantity_blocks() const { return fcb.blocks; }
        InodeType get_inode() const;
        size_t read(off_t offset, size_t size, char* buf);
        size_t write(off_t offset, size_t size, const char* buf, BlocksManager& bmanager);
        void truncate(off_t new_size, BlocksManager& bmanager);
};

#endif