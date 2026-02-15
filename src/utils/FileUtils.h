#pragma once
#include <string>
#include <vector>
#include "common/types.h"

namespace FileUtils {
    // Lecture de fichiers
    std::vector<uint8_t> readBinaryFile(const std::string& path);

    // Vérification
    bool fileExists(const std::string& path);
    size_t getFileSize(const std::string& path);

    // Manipulation de chemins
    std::string getFilename(const std::string& path);
    std::string getExtension(const std::string& path);

    // File dialog
    std::string openFileDialog(
        const std::string& title,
        const std::vector<std::string>& filters = {}
    );
}