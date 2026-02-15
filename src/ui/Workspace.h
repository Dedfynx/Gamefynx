#pragma once
#include <imgui.h>

class Workspace {
public:
    Workspace();

    void begin();
    void end();

    ImGuiID getWorkspaceID() const { return workspaceID; }

private:
    ImGuiID workspaceID = 0;
    bool first_time = true;

    void setupDefaultLayout();
};