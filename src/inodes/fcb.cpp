#include <cstring>
#include <fstream>
#include <functional>
#include <fuse.h>
#include <sys/types.h>

#include "fcb.hpp"

#include "../env.hpp"
#include "../superblock/blocks_manager.hpp"
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

        std::fstream file;
        FcbInode fcb;

    protected:
        bool fill_indirect_header(HeaderIndexs& ind_header, BlockType new_block){
            for (size_t i = 0; i < MAX_POINTERS-1; i++) {
                if (ind_header.data_inodes[i] == 0){
                    ind_header.data_inodes[i] = new_block;
                    return true;
                }
            }
            return false;
        }

        BlockType add_block(
            BlocksManager& bmanager
        ){
            BlockType new_block = bmanager.get_free_block();
            if (fcb.headers.single_indirect == 0){
                fill_indirect_header(fcb.headers, new_block);
                file.seekg(BLOCK_SIZE*fcb.id);
                file.write(reinterpret_cast<const char *>(&fcb), sizeof(FcbInode));
                return new_block;
            }
            HeaderIndexs headers;
            load_indirect(fcb.headers.single_indirect, &headers);
            BlockType last_indirect = fcb.headers.single_indirect;
            while (headers.single_indirect != 0){
                last_indirect = headers.single_indirect;
                load_indirect(last_indirect, &headers);
            }
            if (fill_indirect_header(headers, new_block)){
                file.seekg(last_indirect * BLOCK_SIZE);
                file.write(reinterpret_cast<const char *>(&headers), sizeof(HeaderIndexs));
                return new_block;
            }
            BlockType new_indirect = bmanager.get_free_block();
            headers.single_indirect = new_indirect;
            HeaderIndexs indirect;
            indirect.data_inodes[0] = new_block;
            file.seekg(last_indirect * BLOCK_SIZE);
            file.write(reinterpret_cast<const char *>(&headers), sizeof(HeaderIndexs));
            file.seekg(new_indirect * BLOCK_SIZE);
            file.write(reinterpret_cast<const char *>(&indirect), sizeof(HeaderIndexs));
            return new_block;

        }

        void remove_unused_blocks(
            BlockType last_used,
            BlocksManager& bmanager
        ){
            /*
            Recursive function to clear all blocks from a given block, also release the indirect
            blocks if they are unnecessary
            */
            std::function<void(BlockType, HeaderIndexs*)> clean = [&](BlockType from, HeaderIndexs* headers){
                for (BlockType i = from; i < MAX_POINTERS-1; i++){
                    if (headers->data_inodes[i] != 0){
                        bmanager.release_block(headers->data_inodes[i]);
                        headers->data_inodes[i] = 0;
                    }
                }
                if (headers->single_indirect != 0){
                    HeaderIndexs indirect;
                    BlockType indirect_index = load_indirect(headers->single_indirect, &indirect);
                    clean(0, &indirect);
                    bmanager.release_block(indirect_index);
                    headers->single_indirect = 0;
                }
            };
            if (last_used < MAX_POINTERS-1){
                clean(last_used + 1, &fcb.headers);
                file.seekg(fcb.id * BLOCK_SIZE);
                file.write(reinterpret_cast<const char *>(&fcb), sizeof(FcbInode));
                return;
            }
            BlockType indirect_index;
            HeaderIndexs headers = fcb.headers;
            while (last_used >= MAX_POINTERS-1){
                last_used -= MAX_POINTERS-1;
                indirect_index = load_indirect(headers.single_indirect, &headers);
            }      
            clean(last_used + 1, &headers);
            file.seekg(indirect_index * BLOCK_SIZE);
            file.write(reinterpret_cast<const char *>(&headers), sizeof(HeaderIndexs));
        }
    private:
        BlockType load_indirect(BlockType block, HeaderIndexs* indirect_inode){
            file.seekg(block * BLOCK_SIZE);
            file.read(reinterpret_cast<char*>(indirect_inode), sizeof(HeaderIndexs));
            return block;
        }
        bool find_block(BlockType block, DataInode *inoded){
            auto headers = fcb.headers;
            while (block >= MAX_POINTERS-1){
                block -= MAX_POINTERS-1;
                load_indirect(headers.single_indirect, &headers);
            }
            BlockType inode_block = headers.data_inodes[block];
            if ( inode_block == 0 ){
                return false;
            }
            file.seekg(inode_block * BLOCK_SIZE);
            file.read(reinterpret_cast<char*>(inoded), sizeof(DataInode));
            return true;
        }

        size_t write_extra(
            size_t size,
            const char *buf,
            BlocksManager& bmanager
        ){
            size_t total_writen = 0;
            size_t remaining_size = size;
            off_t offset = 0;
            BlockType new_block = add_block(bmanager);
            DataInode data;
            while (remaining_size > 0){
                data.current_size = std::min(
                    static_cast<uint16_t>(remaining_size),
                    static_cast<uint16_t>(BLOCK_DSIZE)
                );
                memcpy(data.data, buf + offset, data.current_size);
                file.seekp(new_block * BLOCK_SIZE);
                file.write(reinterpret_cast<char*>(&data), sizeof(DataInode));
                offset += data.current_size;
                remaining_size -= data.current_size;
                total_writen += data.current_size;
                new_block = add_block(bmanager);
            }
            fcb.blocks += total_writen / BLOCK_DSIZE;
            file.seekp(fcb.id * BLOCK_SIZE);
            file.write(reinterpret_cast<char*>(&fcb), sizeof(FcbInode));
            return total_writen;
        }
    public:

        explicit File(
            BlocksManager& bmanager,
            struct fuse_file_info *fi,
            const struct fuse_context *fc
        ) : 
            file(envs.disk_file, std::ios::in | std::ios::out | std::ios::binary),
            fcb(
                [&]() -> FcbInode {
                    InodeType id = bmanager.get_free_block();
                    FcbInode temp;

                    temp.id = id;
                    temp.size = BLOCK_SIZE;
                    temp.blocks = 1;
                    temp.type = FileType::TFILE;
                    temp.owner = fc->uid;
                    temp.group = fc->gid;
                    temp.permissions = 0644;
                    temp.created_at = temp.modified_at = temp.accessed_at = time(nullptr);

                    fi->fh = id;
                    file.seekg(id * BLOCK_SIZE);
                    file.write(reinterpret_cast<char*>(&temp), sizeof(FcbInode));

                    return temp;
                }()
            )
        {}
        explicit File(const struct fuse_file_info *fi) : 
            file(envs.disk_file, std::ios::in | std::ios::out | std::ios::binary),
            fcb(
                [&]() -> FcbInode {
                    InodeType id = static_cast<InodeType>(fi->fh);
                    FcbInode temp;
                    file.seekg(id * BLOCK_SIZE);
                    file.read(reinterpret_cast<char*>(&temp), sizeof(FcbInode));
                    return temp;
                }()
            )
            {}

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
                memcpy(buf + total_readed, data.data + from_offset, remaining_block);
                from_block++;
                from_offset = 0;
                remaining_size -= remaining_block;
                total_readed += remaining_block;
            }
            return total_readed;
        }

        size_t write(
            off_t offset,
            size_t size,
            const char *buf,
            BlocksManager& bmanager
        ){
            if (offset >= fcb.size) {
                return 0;
            }
            size_t total_written = 0; 
            BlockType from_block = offset / BLOCK_DSIZE;
            uint16_t from_offset = offset % BLOCK_DSIZE;
            uint16_t remaining_size = size;
            uint16_t remaining_block;
            DataInode data;

            while (remaining_size > 0){
                if (!find_block(from_block, &data)){
                    return write_extra(
                        remaining_size, 
                        buf + total_written,
                        bmanager
                    );
                }
                remaining_block = std::min(
                    static_cast<uint16_t>(remaining_size),
                    static_cast<uint16_t>(BLOCK_DSIZE - from_offset)
                );
                memcpy(data.data + from_offset, buf + total_written, remaining_block);
                data.current_size+=remaining_block;
                file.seekp(from_block * BLOCK_SIZE);
                file.write(reinterpret_cast<char*>(&data), sizeof(DataInode));
                from_block++;
                from_offset = 0;
                remaining_size -= remaining_block;
                total_written += remaining_block;

            }
            if (static_cast<off_t>(total_written / BLOCK_DSIZE) > fcb.blocks){
                remove_unused_blocks(total_written / BLOCK_DSIZE, bmanager);
                fcb.blocks = total_written / BLOCK_DSIZE;
                file.seekp(fcb.id * BLOCK_SIZE);
                file.write(reinterpret_cast<char*>(&fcb), sizeof(FcbInode));
            }


            return total_written;
        }
};
