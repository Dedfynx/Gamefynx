#include "ui/Workspace.h"
#include "imgui_internal.h"
#include "config/EmulatorConfig.h"

Workspace::Workspace() {}

void Workspace::begin() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, flags);
    ImGui::PopStyleVar(3);

    workspaceID = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(workspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    if (first_time) {
        setupDefaultLayout();
        first_time = false;
    }
}

void Workspace::end() {
    ImGui::End();
}

void Workspace::setupDefaultLayout() {
    ImGui::DockBuilderRemoveNode(workspaceID);
    ImGui::DockBuilderAddNode(workspaceID, ImGuiDockNodeFlags_DockSpace);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderSetNodeSize(workspaceID, viewport->Size);

    ImGuiID dock_main = workspaceID;
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(
        dock_main, ImGuiDir_Left, Config::DOCK_SPLIT_LEFT, nullptr, &dock_main
    );
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(
        dock_main, ImGuiDir_Right, Config::DOCK_SPLIT_RIGHT, nullptr, &dock_main
    );

    ImGui::DockBuilderDockWindow("Screen", dock_main);
    ImGui::DockBuilderDockWindow("Controls", dock_left);
    ImGui::DockBuilderDockWindow("Stats", dock_right);
    ImGui::DockBuilderDockWindow("Universal Debugger", dock_right);

    ImGui::DockBuilderFinish(workspaceID);
}