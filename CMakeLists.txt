cmake_minimum_required(VERSION 4.0)
project(Gamefynx VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_CORE_CHIP8    "Build CHIP-8 emulator core"     ON)
option(BUILD_CORE_GAMEBOY  "Build Game Boy emulator core"   ON)
option(BUILD_WITH_DEBUGGER "Build with debugger UI"         ON)
option(ENABLE_LOGGING      "Enable logging system"          ON)

# Flag
add_compile_options(-Wall -Wextra)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(fmt REQUIRED)

# ImGui library
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_SDL3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL3::SDL3 OpenGL::GL)

# Portable File Dialogs
set(PFD_DIR ${CMAKE_SOURCE_DIR}/third_party/portable-file-dialogs)

#Common
add_library(emu_common STATIC
        src/common/types.h
        src/common/EmulatorInterface.h
        src/common/EmulatorCore.h
        src/utils/Logger.cpp
        src/common/InputManager.cpp
        src/utils/FileUtils.cpp
        src/utils/Audio.cpp
)
target_include_directories(emu_common PUBLIC 
    src 
    ${PFD_DIR} 
)
target_link_libraries(emu_common PUBLIC fmt::fmt)

if(ENABLE_LOGGING)
    target_compile_definitions(emu_common PUBLIC ENABLE_LOGGING)
endif()

#UI
add_library(emu_ui STATIC
        src/ui/MainWindow.cpp
        src/ui/ScreenRenderer.cpp
        src/ui/Workspace.cpp
        src/config/ImguiConfig.cpp
)
target_link_libraries(emu_ui PUBLIC 
    emu_common
    imgui
)

#Emulator Cores
if(BUILD_CORE_CHIP8)
    add_library(core_chip8 STATIC
            src/core/chip8/Chip8.cpp
    )
    target_link_libraries(core_chip8 PUBLIC emu_common)
    target_compile_definitions(core_chip8 PUBLIC CORE_CHIP8_ENABLED)
    target_compile_definitions(emu_ui PUBLIC CORE_CHIP8_ENABLED)
    list(APPEND ENABLED_CORES core_chip8)
    message(STATUS "CHIP-8 core enabled")
endif()

if(BUILD_CORE_GAMEBOY)
    add_library(core_gameboy STATIC
            src/core/gameboy/Gameboy.cpp
            src/core/gameboy/GB_CPU.cpp
            src/core/gameboy/GB_MMU.cpp
            src/core/gameboy/GB_PPU.cpp
            src/core/gameboy/GB_PPU.h
            src/core/gameboy/GB_Joypad.cpp
            src/core/gameboy/GB_Joypad.h
    )
    target_link_libraries(core_gameboy PUBLIC emu_common)
    target_compile_definitions(core_gameboy PUBLIC CORE_GAMEBOY_ENABLED)
    target_compile_definitions(emu_ui PUBLIC CORE_GAMEBOY_ENABLED)
    list(APPEND ENABLED_CORES core_gameboy)
    message(STATUS "Game Boy core enabled")
endif()

#Exe
add_executable(Gamefynx
        src/main.cpp
        src/Application.cpp
)

target_link_libraries(Gamefynx PRIVATE
    emu_common
    emu_ui
    ${ENABLED_CORES}
    imgui
    SDL3::SDL3
    OpenGL::GL
)

# Copy SDL3.dll sur Windows
if(WIN32)
    add_custom_command(TARGET Gamefynx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:Gamefynx>
    )
endif()

# ============================================================================
# RÉSUMÉ DE LA CONFIGURATION
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  Multi-Emulator Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "")
message(STATUS "Enabled cores:")
foreach(core ${ENABLED_CORES})
    message(STATUS "  - ${core}")
endforeach()
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  Debugger: ${BUILD_WITH_DEBUGGER}")
message(STATUS "  Logging:  ${ENABLE_LOGGING}")
message(STATUS "========================================")
message(STATUS "")