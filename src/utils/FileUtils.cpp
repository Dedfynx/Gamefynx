// file_utils.cpp
#include "utils/FileUtils.h"
#include "utils/Logger.h"
#include <portable-file-dialogs.h>

#include <fstream>
#include <filesystem>



std::string FileUtils::getExtension(const std::string& path) {
    std::filesystem::path p(path);
    std::string ext = p.extension().string();

    // Enlève le point
    if (!ext.empty() && ext[0] == '.') {
        ext = ext.substr(1);
    }

    return ext;
}

std::string FileUtils::openFileDialog(
    const std::string& title,
    const std::vector<std::string>& filters
) {
    // Crée le dialog
    auto dialog = pfd::open_file(
        title,
        ".",  // Dossier initial (. = courant)
        filters,
        pfd::opt::none
    );
    
    // Attend que l'utilisateur choisisse
    auto result = dialog.result();
    
    if (result.empty()) {
        LOG_DEBUG("File dialog cancelled");
        return "";  // Annulé
    }
    
    std::string path = result[0];
    LOG_INFO("File selected: {}", path);
    return path;
}

std::vector<uint8_t> FileUtils::readBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: {}", path);
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        LOG_ERROR("Failed to read file: {}", path);
        return {};
    }

    return buffer;
}

bool fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

size_t getFileSize(const std::string& path) {
    if (!fileExists(path)) {
        return 0;
    }

    try {
        return std::filesystem::file_size(path);
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Failed to get file size: {}", e.what());
        return 0;
    }
}

std::string getFilename(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

