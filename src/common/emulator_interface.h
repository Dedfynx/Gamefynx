#pragma once
#include "types.h"
#include <string>

class IEmulator {
public:
    virtual ~IEmulator() = default;

    virtual bool loadROM(const std::string& path) = 0;
    virtual void reset() = 0;

    virtual void step() = 0;       
    virtual void runFrame() = 0;   

    virtual const uint8_t* getFramebuffer() const = 0;
    virtual int getScreenWidth() const = 0;
    virtual int getScreenHeight() const = 0;

    virtual void setButton(int button, bool pressed) = 0;

    virtual std::string getArchName() const = 0;
    virtual const uint8_t* getMemoryPtr() const = 0;
    virtual size_t getMemorySize() const = 0;
    virtual uint16_t getPC() const = 0;
};