#pragma once
#include "types.h"
#include <string>

class IEmulator {
public:
    virtual ~IEmulator() = default;
    
    // Lifecycle
    virtual bool loadROM(const std::string& path) = 0;
    virtual void reset() = 0;
    
    // Execution
    virtual void step() = 0;       
    virtual void runFrame() = 0;   
    
    // Display
    virtual const uint8_t* getFramebuffer() const = 0;
    virtual int getScreenWidth() const = 0;
    virtual int getScreenHeight() const = 0;
    
    // Input
    virtual void setButton(int button, bool pressed) = 0;
    
    // Info
    virtual std::string getArchName() const = 0;
};