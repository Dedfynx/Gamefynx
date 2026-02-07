// file_utils.cpp
#include "utils/file_utils.h"

std::string FileUtils::getExtension(const std::string& path) {
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return "";
    return path.substr(dot + 1);
}