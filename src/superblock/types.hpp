#ifndef TYPES_HPP
#define TYPES_HPP

#include <stdint.h> 
#include <sys/types.h>

typedef uint64_t InodeType; // points to a Inode of the a file or disk
typedef uint64_t BlockType; // points to a logical block of the disk
#endif