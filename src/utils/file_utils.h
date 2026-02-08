#pragma once
#include <string>
#include <vector>
#include "common/types.h"

class FileUtils {
public:
    static std::string getExtension(const std::string& path);

    static std::string openFileDialog(
        const std::string& title,
        const std::vector<std::string>& filters = {}
    );
};