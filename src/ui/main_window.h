#pragma once
#include "common/emulator_interface.h"
#include <memory>

class MainWindow {
public:
    void render(IEmulator* emulator);
    
private:
    bool show_demo = false;
};