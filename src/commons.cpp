#include <tuple>
#include <string>
#include <regex>


std::tuple<std::string, std::string> separete_parent_and_children(std::string path) {

    std::regex regex(R"(^(.*)/([^/]+)$)");
    std::smatch match;

    std::regex_search(path, match, regex);

    std::string parent = match[1]; // catch the parent directory of the path
    std::string children = match[2]; // catch the children of the path

    return std::make_tuple(parent, children);
}