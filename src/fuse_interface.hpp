#ifndef FUSE
#define FUSE


struct fuse_operations * build_fuse_operations();
void *init(fuse_conn_info *conn);
#endif