#include "Application.h"
#include "utils/Logger.h"

int main(int argc, char* argv[]) {
    // ¯\_(ツ)_/¯
    (void)argv;
    (void)argc;

    Application app{};

    if (!app.init()) {
        LOG_ERROR("Application initialization failed");
        return 1;
    }

    app.run();

    return 0;
}