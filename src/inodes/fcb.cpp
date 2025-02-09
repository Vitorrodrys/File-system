#define FUSE_USE_VERSION 31
#include <cstring>
#include <fstream>
#include <functional>
#include <fuse3/fuse.h>
#include <unordered_map>

#include "../env.hpp"
#include "fcb.hpp"


File::File(
    mode_t mode,
    FileType type,
    InodeType inode,
    uid_t uid,
    gid_t gid
) : file(Env::get_instance().disk_file, std::ios::in | std::ios::out | std::ios::binary),
    fcb(
        [&]() -> FcbInode {
            FcbInode temp;
            strcpy(temp.name, "/");
            temp.id = inode;
            temp.size = 0;
            temp.blocks = 1;
            temp.type = type;
            temp.owner = uid;
            temp.group = gid;
            temp.permissions = mode;
            temp.created_at = temp.modified_at = temp.accessed_at = time(nullptr);
            memset(&temp.headers, 0, sizeof(temp.headers));

            file.seekp(inode * BLOCK_SIZE);
            file.write(reinterpret_cast<char *>(&temp), sizeof(FcbInode));
            return temp;
        }()
    ) {}
File::File(
    const std::string& name,
    mode_t mode,
    BlocksManager &bmanager,
    const struct fuse_context *fc,
    FileType type
)
    : file(Env::get_instance().disk_file, std::ios::in | std::ios::out | std::ios::binary),
      fcb([&]() -> FcbInode {
          InodeType id = bmanager.get_free_block();
          FcbInode temp;
          strcpy(temp.name, name.c_str());
          temp.id = id;
          temp.size = 0;
          temp.blocks = 1;
          temp.type = type;
          temp.owner = fc->uid;
          temp.group = fc->gid;
          temp.permissions = mode;
          temp.created_at = temp.modified_at = temp.accessed_at = time(nullptr);
          memset(&temp.headers, 0, sizeof(temp.headers));


          file.seekp(id * BLOCK_SIZE);
          file.write(reinterpret_cast<char *>(&temp), sizeof(FcbInode));

          return temp;
      }()) {}

File::File(InodeType id)
    : file(Env::get_instance().disk_file, std::ios::in | std::ios::out | std::ios::binary),
        fcb(
            [&]() -> FcbInode {
                FcbInode temp;
                file.seekg(id * BLOCK_SIZE);
                file.read(reinterpret_cast<char *>(&temp), sizeof(FcbInode));
                return temp;
            }()
        ) {}

File::File(const File &other)
    : file(Env::get_instance().disk_file, std::ios::in | std::ios::out | std::ios::binary),
      fcb(other.fcb) {}

File::~File() {
    update_fcb();
    file.flush();
    file.close();
}

void File::update_fcb() {
    file.seekp(fcb.id * BLOCK_SIZE);
    file.write(reinterpret_cast<const char *>(&fcb), sizeof(FcbInode));
}

BlockType File::load_indirect(BlockType block, HeaderIndexs *indirect_inode) {
    file.seekg(block * BLOCK_SIZE);
    file.read(reinterpret_cast<char *>(indirect_inode), sizeof(HeaderIndexs));
    return block;
}

BlockType File::get_block_value(uint32_t index) {
    HeaderIndexs headers = fcb.headers;
    while (index >= MAX_POINTERS - 1) {
        index -= MAX_POINTERS - 1;
        if (headers.single_indirect == 0){
            return 0;
        }
        load_indirect(headers.single_indirect, &headers);
    }
    return headers.data_inodes[index];
}

bool File::load_data_block(BlockType block, DataInode *inoded) {
    if (block == 0) {
        return false;
    }
    file.seekg(block * BLOCK_SIZE);
    file.read(reinterpret_cast<char *>(inoded), sizeof(DataInode));
    return true;
}

size_t File::read(off_t offset, size_t size, char *buf) {
    if (offset >= fcb.size) {
        return 0;
    }
    size_t total_readed = 0;
    uint32_t ifrom_block = offset / BLOCK_DSIZE;
    BlockType from_block = get_block_value(ifrom_block);
    uint16_t from_offset = offset % BLOCK_DSIZE;
    size_t remaining_size = size;
    size_t remaining_block;
    DataInode data;

    while (remaining_size > 0) {
        load_data_block(from_block, &data);
        remaining_block =
            std::min(static_cast<size_t>(data.current_size),
                     remaining_size);
        memcpy(buf + total_readed, data.data + from_offset, remaining_block);
        ifrom_block++;
        from_block = get_block_value(ifrom_block);
        if (from_block == 0) {
            total_readed += remaining_block;
            return total_readed;
        }
        from_offset = 0;
        remaining_size -= remaining_block;
        total_readed += remaining_block;
    }
    return total_readed;
}

void File::fill_indirect_header(BlocksManager& bmanager, BlockType new_block) {

    auto fill_header = [&](HeaderIndexs& headers, BlockType new_block) -> bool{
        for (size_t i = 0; i < MAX_POINTERS - 1; i++) {
            if (headers.data_inodes[i] == 0) {
                headers.data_inodes[i] = new_block;
                return true;
            }
        }
        return false;
    };
    if (fcb.headers.single_indirect == 0 and fill_header(fcb.headers, new_block)){
        update_fcb();
        return;
    } else if (fcb.headers.single_indirect == 0 ){
        BlockType new_indirect = bmanager.get_free_block();
        fcb.headers.single_indirect = new_indirect;
        HeaderIndexs indirect;
        memset(&indirect, 0, sizeof(HeaderIndexs));
        indirect.data_inodes[0] = new_block;
        file.seekp(new_indirect*BLOCK_SIZE);
        file.write(reinterpret_cast<const char *>(&indirect), sizeof(HeaderIndexs));
        update_fcb();
        return;
    }
    BlockType last_indirect=fcb.headers.single_indirect;
    HeaderIndexs headers;
    load_indirect(last_indirect, &headers);
    while (headers.single_indirect != 0 or not fill_header(headers, new_block) ){
        if (headers.single_indirect == 0){
            BlockType new_bindirect = bmanager.get_free_block();
            headers.single_indirect = new_bindirect;
            file.seekp(last_indirect*BLOCK_SIZE);
            file.write(reinterpret_cast<const char *>(&headers), sizeof(HeaderIndexs));

            HeaderIndexs new_indirect;
            memset(&new_indirect, 0, sizeof(HeaderIndexs));
            new_indirect.data_inodes[0] = new_block;
            file.seekp(headers.single_indirect*BLOCK_SIZE);
            file.write(reinterpret_cast<const char *>(&new_indirect), sizeof(HeaderIndexs));
            return;
        }
        last_indirect = headers.single_indirect;
        load_indirect(last_indirect, &headers);
    }
    file.seekp(last_indirect*BLOCK_SIZE);
    file.write(reinterpret_cast<const char *>(&headers), sizeof(HeaderIndexs));
}


BlockType File::add_block(BlocksManager &bmanager) {
    BlockType new_block = bmanager.get_free_block();
    fill_indirect_header(bmanager, new_block);
    return new_block;
}

size_t File::write_extra(size_t size, const char *buf,
                         BlocksManager &bmanager) {
    size_t total_writen = 0;
    size_t remaining_size = size;
    off_t offset = 0;
    BlockType new_block;
    DataInode data;
    while (remaining_size > 0) {
        new_block = add_block(bmanager);
        data.current_size = std::min(static_cast<uint16_t>(remaining_size),
                                     static_cast<uint16_t>(BLOCK_DSIZE));
        memcpy(data.data, buf + offset, data.current_size);
        file.seekp(new_block * BLOCK_SIZE);
        file.write(reinterpret_cast<char *>(&data), sizeof(DataInode));
        offset += data.current_size;
        remaining_size -= data.current_size;
        total_writen += data.current_size;
    }
    return total_writen;
}

void File::remove_unused_blocks(BlockType last_used, BlocksManager &bmanager) {
    std::function<void(BlockType, HeaderIndexs *)> clean =
        [&](BlockType from, HeaderIndexs *headers) {
            for (BlockType i = from; i < MAX_POINTERS - 1; i++) {
                if (headers->data_inodes[i] != 0) {
                    bmanager.release_block(headers->data_inodes[i]);
                    headers->data_inodes[i] = 0;
                }
            }
            if (headers->single_indirect != 0) {
                HeaderIndexs indirect;
                BlockType indirect_index = load_indirect(headers->single_indirect, &indirect);
                clean(0, &indirect);
                bmanager.release_block(indirect_index);
                headers->single_indirect = 0;
            }
        };

    if (last_used < MAX_POINTERS - 1) {
        clean(last_used + 1, &fcb.headers);
        file.seekp(fcb.id * BLOCK_SIZE);
        file.write(reinterpret_cast<const char *>(&fcb), sizeof(FcbInode));
        return;
    }

    BlockType indirect_index;
    HeaderIndexs headers = fcb.headers;
    do {
        last_used -= MAX_POINTERS - 1;
        indirect_index = load_indirect(headers.single_indirect, &headers);
    } while (last_used >= MAX_POINTERS - 1);

    clean(last_used + 1, &headers);
    file.seekp(indirect_index * BLOCK_SIZE);
    file.write(reinterpret_cast<const char *>(&headers), sizeof(HeaderIndexs));
}

size_t File::write(off_t offset, size_t size, const char *buf,
                   BlocksManager &bmanager) {
    if ( offset > fcb.size ) {
        return 0;
    }
    size_t total_written = 0;
    uint32_t ind_from_block = offset / BLOCK_DSIZE;
    BlockType from_block = get_block_value(ind_from_block);
    uint16_t from_offset = offset % BLOCK_DSIZE;
    uint16_t remaining_size = size;
    uint16_t remaining_block;
    DataInode data;

    while (remaining_size > 0) {
        if (!load_data_block(from_block, &data)) {
            size_t extra_writted =
                write_extra(remaining_size, buf + total_written, bmanager);
            total_written += extra_writted;
            fcb.size = offset + total_written;
            fcb.blocks = fcb.size / BLOCK_DSIZE+1;
            update_fcb();
            return total_written;
        }
        remaining_block =
            std::min(static_cast<uint16_t>(remaining_size),
                     static_cast<uint16_t>(BLOCK_DSIZE - from_offset));
        memcpy(data.data + from_offset, buf + total_written, remaining_block);
        data.current_size = from_offset + remaining_block;
        file.seekp(from_block * BLOCK_SIZE);
        file.write(reinterpret_cast<char *>(&data), sizeof(DataInode));
        ind_from_block++;
        from_offset = 0;
        from_block = get_block_value(ind_from_block);
        remaining_size -= remaining_block;
        total_written += remaining_block;
    }
    if (fcb.size < static_cast<off_t>(offset + total_written)){
        fcb.size = offset + total_written;
        update_fcb();
    }
    return total_written;
}

void File::delete_file(BlocksManager& bmanager){
    truncate(0, bmanager);
    bmanager.release_block(fcb.id);
}

void File::truncate(off_t new_size, BlocksManager& bmanager){
    if (new_size < fcb.size ){
        BlockType last_used = (new_size / BLOCK_DSIZE) - ((new_size % BLOCK_DSIZE == 0) ? 1 : 0);
        remove_unused_blocks(last_used, bmanager);
    }
    fcb.size = new_size;
    fcb.blocks = fcb.size / BLOCK_DSIZE + 1;
    update_fcb();
}

FileType File::get_type() const {
    return fcb.type;
}
off_t File::get_size() const {
    return fcb.size;
}
InodeType File::get_inode() const{
    return fcb.id;
}