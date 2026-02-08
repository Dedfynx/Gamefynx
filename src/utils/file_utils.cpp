// file_utils.cpp
#include "utils/file_utils.h"
#include "utils/logger.h"
#include <portable-file-dialogs.h>


std::string FileUtils::getExtension(const std::string& path) {
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return "";
    return path.substr(dot + 1);
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