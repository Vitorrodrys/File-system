#include <sys/types.h>
#define MAX_POINTERS 12

#include "../env.hpp"
#include "fcb.hpp"

typedef struct HeaderIndexs{
    BlockType data_inodes[MAX_POINTERS-1];
    BlockType single_indirect;
}HeaderIndexs;

typedef enum FileType {
    TFILE,
    TDIRECTORY,
    THLINK,
    TSLINK
} FileType;

typedef struct FcbInode {
    char name[256];
    InodeType id;
    FileType type;
    unsigned long long int size;
    unsigned long long int blocks;
    uid_t owner;
    gid_t group;
    mode_t permissions;
    time_t created_at;
    time_t modified_at;
    time_t accessed_at;
    HeaderIndexs headers;
} INODEFCB;

typedef struct IndirectInode {
    HeaderIndexs headers;
} INODEINDIRECT;


typedef struct DataInode {
    unsigned int current_size;
    char data[BLOCK_SIZE - sizeof(unsigned int)];
} DataInode;
