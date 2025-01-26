#include <cstring>
#include <fstream>
#include <fuse.h>
#include <sys/types.h>

#include "fcb.hpp"

#include "../env.hpp"
#include "../superblock/types.hpp"

#define MAX_POINTERS 12
#define BLOCK_DSIZE (BLOCK_SIZE - sizeof(unsigned short))
typedef struct HeaderIndexs{
    BlockType data_inodes[MAX_POINTERS-1];
    BlockType single_indirect;
}HeaderIndexs;

enum class FileType {
    TFILE,
    TDIRECTORY,
    THLINK,
    TSLINK
};

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

const Env& envs = Env::get_instance();

class File {

    private:
        void load_indirect(BlockType block, HeaderIndexs* indirect_inode){
            std::ifstream file(envs.disk_file, std::ios::binary);
            file.seekg(block * BLOCK_SIZE);
            file.read(reinterpret_cast<char*>(indirect_inode), sizeof(HeaderIndexs));
        }
        void find_block(BlockType block, DataInode *inoded){
            auto headers = fcb.headers;
            while (block >= MAX_POINTERS-1){
                block -= MAX_POINTERS-1;
                load_indirect(headers.single_indirect, &headers);
            }
            BlockType inode_block = headers.data_inodes[block];
            std::ifstream file(envs.disk_file, std::ios::binary);
            file.seekg(inode_block * BLOCK_SIZE);
            file.read(reinterpret_cast<char*>(inoded), sizeof(DataInode));

        }
        
    public:
        const FcbInode fcb;

        File(struct fuse_file_info *fi) : fcb([&]() -> FcbInode {
            InodeType id = static_cast<InodeType>(fi->fh);
            std::ifstream file(envs.disk_file, std::ios::binary);
            FcbInode temp;
            file.seekg(id * BLOCK_SIZE);
            file.read(reinterpret_cast<char*>(&temp), sizeof(FcbInode));
            return temp;
        }()) {}

        size_t read(off_t offset, size_t size, char *buf){
            if (offset >= fcb.size) {
                return 0;
            }
            size_t total_readed = 0; 
            BlockType from_block = offset / BLOCK_DSIZE;
            uint16_t from_offset = offset % BLOCK_DSIZE;
            uint16_t remaining_size = size;
            uint16_t remaining_block;
            DataInode data;

            while (remaining_size > 0){
                find_block(from_block, &data);
                remaining_block = std::min(
                    static_cast<uint16_t>(data.current_size),
                    static_cast<uint16_t>(BLOCK_DSIZE - from_offset));
                memcpy(buf, data.data + from_offset, remaining_block);
                from_block++;
                from_offset = 0;
                remaining_size -= remaining_block;
                total_readed += remaining_block;
            }
            return total_readed;
        }

        size_t write(off_t offset, size_t size, const char *buf){
            if (offset >= fcb.size) {
                return 0;
            }
            std::ofstream outfile(envs.disk_file, std::ios::binary);
            size_t total_written = 0; 
            BlockType from_block = offset / BLOCK_DSIZE;
            uint16_t from_offset = offset % BLOCK_DSIZE;
            uint16_t remaining_size = size;
            uint16_t remaining_block;
            DataInode data;

            while (remaining_size > 0){
                find_block(from_block, &data);
                remaining_block = std::min(
                    static_cast<uint16_t>(data.current_size),
                    static_cast<uint16_t>(BLOCK_DSIZE - from_offset));
                memcpy(data.data + from_offset, buf + total_written, remaining_block);
                outfile.seekp(from_block * BLOCK_SIZE);
                outfile.write(reinterpret_cast<char*>(&data), sizeof(DataInode));
                from_block++;
                from_offset = 0;
                remaining_size -= remaining_block;
                total_written += remaining_block;

            }
            return total_written;
        }
};
