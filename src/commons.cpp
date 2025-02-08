
#include <fcntl.h>
#include <iostream>
#include <regex>
#include <string>
#include <tuple>
#include <unistd.h>

std::tuple<std::string, std::string> separete_parent_and_children(const std::string& path) {

    const std::regex regex(R"(^(.*)/([^/]+)$)");
    std::smatch match;

    std::regex_search(path, match, regex);

    std::string parent = match[1]; // catch the parent directory of the path
    std::string children = match[2]; // catch the children of the path

    return std::make_tuple(parent, children);
}

void create_virtual_disk(const std::string& vdisk_path, size_t vdisk_size){
    int fd = open(vdisk_path.c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        std::cerr << "Error when trying to create virtual disk";
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, vdisk_size) == -1) {
        std::cerr << "Error when trying to set the virtual disk size." << std::endl;
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}