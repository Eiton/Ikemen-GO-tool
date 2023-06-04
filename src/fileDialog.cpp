#include "imgui.h"
#include "fileDialog.h"
#include "utility.h"
#include <filesystem>
#include <list>
#include <set>
#include "IconsFontAwesome6.h"
#include "utility.h"

std::string FileDialog::filePath;
enum FileDialog::fileType;

bool FileDialog::showFileDialog(std::string fileType) {
    ImGui::SetNextWindowSize(ImVec2{ 720,360 }, ImGuiCond_Once);
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Open File", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
    defer (ImGui::End());
    static bool init = true;
    static char path[256] = "test";
    static std::filesystem::path selectedPath;
    if (init) {
        init = false;
        if (setting.filePath != "") {
            setting.filePath.copy(path, 256);
        }
        else {
            std::filesystem::current_path().u8string().copy(path, 256);
        }
    }
    //ImGui::Text("Path:");
    //ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
    ImGui::InputText("##", path, IM_ARRAYSIZE(path));
    ImGui::Separator();
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("fileList", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    std::filesystem::path currentPath = std::filesystem::u8path(path);
    if (std::filesystem::exists(currentPath)) {
        if (currentPath.has_parent_path() && ImGui::Selectable(ICON_FA_FOLDER " ..", selectedPath.compare(currentPath.parent_path()) == 0, ImGuiSelectableFlags_AllowDoubleClick)) {
            selectedPath = currentPath.parent_path();
            if (ImGui::IsMouseDoubleClicked(0)) {
                memset(path, 0, 256);
                selectedPath.u8string().copy(path, 256);
            }
        }
        std::list<std::filesystem::path> filePaths;
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::u8path(path))) {
            bool isDirectory = std::filesystem::is_directory(std::filesystem::status(entry.path()));
            std::string fileName = entry.path().filename().u8string();
            if (!isDirectory && (fileName.length() < fileType.length() || fileName.substr(fileName.length()-fileType.length(),fileType.length()) != fileType)) {
                continue;
            }
            filePaths.push_back(entry.path());
        }
        filePaths.sort([](std::filesystem::path p1, std::filesystem::path p2) {
            bool isDirectory1 = std::filesystem::is_directory(std::filesystem::status(p1));
            bool isDirectory2 = std::filesystem::is_directory(std::filesystem::status(p2));
            if (isDirectory1 ^ isDirectory2) {
                return isDirectory1;
            }
            return p1.filename().u8string() > p1.filename().u8string();
            });
        for (auto p : filePaths) {
            bool isDirectory = std::filesystem::is_directory(std::filesystem::status(p));
            std::string fileName = (isDirectory ? ICON_FA_FOLDER " " : ICON_FA_FILE " ") + p.filename().u8string();
            if (ImGui::Selectable(fileName.c_str(), selectedPath.compare(p) == 0, ImGuiSelectableFlags_AllowDoubleClick)) {
                selectedPath = p;
                if (ImGui::IsMouseDoubleClicked(0)) {
                    if (isDirectory) {
                        memset(path, 0, 256);
                        selectedPath.u8string().copy(path, 256);
                    }
                    else {
                        filePath = selectedPath.u8string();
                        ImGui::EndChild();
                        return false;
                    }
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::Separator();
    if (ImGui::Button("Cancel")) {
        return false;
    }
    ImGui::SameLine();
    if (selectedPath.empty()) {
        ImGui::BeginDisabled(true);
    }
    if (ImGui::Button("Open")) {
        bool isDirectory = std::filesystem::is_directory(std::filesystem::status(selectedPath));
        if (isDirectory) {
            memset(path, 0, 256);
            selectedPath.u8string().copy(path, 256);
        }
        else {
            filePath = selectedPath.u8string();
            return false;
        }
    }
    if (selectedPath.empty()) {
        ImGui::EndDisabled();
    }
    return true;
}