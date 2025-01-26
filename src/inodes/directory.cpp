#include "fcb.hpp"

class Directory : protected File {


    private:
        std::unordered_map<std::string, InodeType> entries;

    Directory(
        BlocksManager& bmanager,
        struct fuse_file_info *fi,
        const struct fuse_context *fc
    ) :
        File(bmanager, fi, fc)
    {
        fcb.type = FileType::TDIRECTORY;
    }

    Directory(const struct fuse_file_info *fi) : File(fi) {
    char *buf = new char[fcb.size];
    read(0, fcb.size, buf);
    
    int index = 0;

    while (index < fcb.size) {
        std::string key(buf + index);
        index += key.length() + 1;

        std::string inode_str(buf + index);
        index += inode_str.length() + 1;

        // Converte o inode para inteiro e adiciona ao mapa
        entries[key] = static_cast<InodeType>(std::stoi(inode_str));
    }

    delete[] buf;  // Libera o buffer após o uso
}

    bool create_file(const char *name){

    }

}
