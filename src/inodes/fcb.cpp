#include <cstring>
#include <fstream>
#include <functional>
#include <fuse.h>
#include <unordered_map>

#include "../env.hpp"
#include "fcb.hpp"

const Env &fcb_envs = Env::get_instance();

bool File::fill_indirect_header(HeaderIndexs &ind_header, BlockType new_block) {
    for (size_t i = 0; i < MAX_POINTERS - 1; i++) {
        if (ind_header.data_inodes[i] == 0) {
            ind_header.data_inodes[i] = new_block;
            return true;
        }
    }
    return false;
}

File::File(
    BlocksManager &bmanager,
    mode_t mode,
    const struct fuse_context *fc,
    FileType type
)
    : file(fcb_envs.disk_file, std::ios::in | std::ios::out | std::ios::binary),
      fcb([&]() -> FcbInode {
          InodeType id = bmanager.get_free_block();
          FcbInode temp;

          temp.id = id;
          temp.size = BLOCK_SIZE;
          temp.blocks = 1;
          temp.type = type;
          temp.owner = fc->uid;
          temp.group = fc->gid;
          temp.permissions = mode;
          temp.created_at = temp.modified_at = temp.accessed_at = time(nullptr);


          file.seekg(id * BLOCK_SIZE);
          file.write(reinterpret_cast<char *>(&temp), sizeof(FcbInode));

          return temp;
      }()) {}

File::File(InodeType id)
    : file(fcb_envs.disk_file, std::ios::in | std::ios::out | std::ios::binary),
        fcb(
            [&]() -> FcbInode {
                FcbInode temp;
                file.seekg(id * BLOCK_SIZE);
                file.read(reinterpret_cast<char *>(&temp), sizeof(FcbInode));
                return temp;
            }()
        ) {}

File::File(const File &other)
    : file(fcb_envs.disk_file, std::ios::in | std::ios::out | std::ios::binary),
      fcb(other.fcb) {}


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
    uint16_t remaining_size = size;
    uint16_t remaining_block;
    DataInode data;

    while (remaining_size > 0) {
        load_data_block(from_block, &data);
        remaining_block =
            std::min(static_cast<uint16_t>(data.current_size),
                     static_cast<uint16_t>(BLOCK_DSIZE - from_offset));
        memcpy(buf + total_readed, data.data + from_offset, remaining_block);
        ifrom_block++;
        from_block = get_block_value(ifrom_block);
        if (from_block == 0) {
            return total_readed;
        }
        from_offset = 0;
        remaining_size -= remaining_block;
        total_readed += remaining_block;
    }
    return total_readed;
}

BlockType File::add_block(BlocksManager &bmanager) {
    BlockType new_block = bmanager.get_free_block();
    if (fcb.headers.single_indirect == 0) {
        fill_indirect_header(fcb.headers, new_block);
        file.seekg(BLOCK_SIZE * fcb.id);
        file.write(reinterpret_cast<const char *>(&fcb), sizeof(FcbInode));
        return new_block;
    }

    HeaderIndexs headers;
    load_indirect(fcb.headers.single_indirect, &headers);
    BlockType last_indirect = fcb.headers.single_indirect;
    while (headers.single_indirect != 0) {
        last_indirect = headers.single_indirect;
        load_indirect(last_indirect, &headers);
    }

    if (fill_indirect_header(headers, new_block)) {
        file.seekg(last_indirect * BLOCK_SIZE);
        file.write(reinterpret_cast<const char *>(&headers),
                   sizeof(HeaderIndexs));
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

size_t File::write_extra(size_t size, const char *buf,
                         BlocksManager &bmanager) {
    size_t total_writen = 0;
    size_t remaining_size = size;
    off_t offset = 0;
    BlockType new_block = add_block(bmanager);
    DataInode data;
    while (remaining_size > 0) {
        data.current_size = std::min(static_cast<uint16_t>(remaining_size),
                                     static_cast<uint16_t>(BLOCK_DSIZE));
        memcpy(data.data, buf + offset, data.current_size);
        file.seekp(new_block * BLOCK_SIZE);
        file.write(reinterpret_cast<char *>(&data), sizeof(DataInode));
        offset += data.current_size;
        remaining_size -= data.current_size;
        total_writen += data.current_size;
        new_block = add_block(bmanager);
    }
    fcb.blocks += total_writen / BLOCK_DSIZE;
    file.seekp(fcb.id * BLOCK_SIZE);
    file.write(reinterpret_cast<char *>(&fcb), sizeof(FcbInode));
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
                BlockType indirect_index =
                    load_indirect(headers->single_indirect, &indirect);
                clean(0, &indirect);
                bmanager.release_block(indirect_index);
                headers->single_indirect = 0;
            }
        };

    if (last_used < MAX_POINTERS - 1) {
        clean(last_used + 1, &fcb.headers);
        file.seekg(fcb.id * BLOCK_SIZE);
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
    file.seekg(indirect_index * BLOCK_SIZE);
    file.write(reinterpret_cast<const char *>(&headers), sizeof(HeaderIndexs));
}

size_t File::write(off_t offset, size_t size, const char *buf,
                   BlocksManager &bmanager) {
    if (offset >= fcb.size) {
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
            fcb.blocks = total_written / BLOCK_DSIZE;
            fcb.size = total_written;
            update_fcb();
            return total_written;
        }
        remaining_block =
            std::min(static_cast<uint16_t>(remaining_size),
                     static_cast<uint16_t>(BLOCK_DSIZE - from_offset));
        memcpy(data.data + from_offset, buf + total_written, remaining_block);
        data.current_size += remaining_block;
        file.seekp(from_block * BLOCK_SIZE);
        file.write(reinterpret_cast<char *>(&data), sizeof(DataInode));
        ind_from_block++;
        from_offset = 0;
        from_block = get_block_value(ind_from_block);
        remaining_size -= remaining_block;
        total_written += remaining_block;
    }
    if (static_cast<off_t>(total_written / BLOCK_DSIZE) < fcb.blocks) {
        remove_unused_blocks(total_written / BLOCK_DSIZE, bmanager);
        fcb.blocks = total_written / BLOCK_DSIZE;
    }

    fcb.size = total_written;
    update_fcb();

    return total_written;
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