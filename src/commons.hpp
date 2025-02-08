#ifndef COMMONS_H
#define COMMONS_H

#include <tuple>
#include <string>
#include <regex>

std::tuple<std::string, std::string> separete_parent_and_children(const std::string& path);
void create_virtual_disk(const std::string& vdisk_path, size_t vdisk_size);

#endif