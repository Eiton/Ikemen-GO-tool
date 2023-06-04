#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "IconsFontAwesome6.h"


#include "fa6.cpp"
#include "fileDialog.h"
#include "combobox.h"
#include "utility.h"
#include "char.h"
#include "render.h"
#include "main.h"

#include "windows.h"
#include <stdio.h>
#include <future>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
void addFont(ImGuiIO &io) {
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    builder.BuildRanges(&ranges);

    char winFolder[512]{};
    ImFontConfig config;
    int appendAt = GetWindowsDirectoryA(winFolder, 512);
    strcpy(winFolder + appendAt, "\\Fonts\\MSGOTHIC.TTC");

    io.Fonts->AddFontFromFileTTF(winFolder, 14,&config,ranges.Data);


    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    float fSize = 14.0f;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = fSize;

    io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesome6_compressed_data, FontAwesome6_compressed_size, fSize, &icons_config, icons_ranges);


    io.Fonts->Build();
}

void LoadingIndicatorCircle() {
    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
    ImVec2 size = ImGui::GetMainViewport()->WorkSize;
    ImGui::SetNextWindowSize(size);
    if (ImGui::Begin("Looading", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs))
    {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        float time = ImGui::GetTime();
        const int numCircle = 12;
        for (int i = 0; i < numCircle; i++) {
            float angle = 2 * PI * i / numCircle;
            ImVec2 pos = ImVec2(center.x+55*cosf(angle), center.y+55*sinf(angle));
            draw_list->AddCircleFilled(pos, 6+3*sinf(time*5-i*0.4), ImGui::GetColorU32(ImGuiCol_Text));
        }
        ImGui::End();
    }

}

// Main code
int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Ikemen GO Tool", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    Utility::loadCustomSetting();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    addFont(io);

    char loadFile[5] = "";

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    static Render render = Render(display_w, display_h);

    static bool leftClick = false;
    static double oldX;
    static double oldY;
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window,button,action,mods);
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            leftClick = true;
            glfwGetCursorPos(window, &oldX, &oldY);
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            leftClick = false;
        }
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
        ImGui_ImplGlfw_CursorPosCallback(window,x,y);
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse && leftClick) {
            render.moveCamera((float)(x - oldX), (float)(y - oldY));
        }
        if (leftClick || io.MouseDown[0]) {
            int w, h;
            bool wrapped = false;
            glfwGetFramebufferSize(window, &w, &h);
            if (x <= 0 && x - oldX < 0) {
                io.MousePos[0] = w;
                oldX = w;
                io.MousePosPrev[0] = w;
                io.WantSetMousePos = true;
            }
            else if (x >= w && x - oldX > 0) {
                io.MousePos[0] = 0;
                oldX = 0;
                io.MousePosPrev[0] = 0;
                io.WantSetMousePos = true;
            }
            if (y <= 0 && y - oldY < 0) {
                io.MousePos[1] = h;
                oldY = h;
                io.MousePosPrev[1] = h;
                io.WantSetMousePos = true;
            }
            else if (y >= h && y - oldY > 0) {
                io.MousePos[1] = 0;
                oldY = 0;
                io.MousePosPrev[1] = 0;
                io.WantSetMousePos = true;
            }
            if (!io.WantSetMousePos) {
                oldX = x;
                oldY = y;
            }
        }
    });
    glfwSetScrollCallback(window, [](GLFWwindow* window,double x, double y) {
        ImGui_ImplGlfw_ScrollCallback(window, x, y);
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return;
        }
        render.scaleCamera(y);
    });
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return;
        }
        if (action == GLFW_RELEASE) {
            return;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL == GLFW_PRESS))
        {
            switch (key)
            {
            case GLFW_KEY_EQUAL:
            {
                render.scaleCamera(1);
            }
            break;
            case GLFW_KEY_MINUS:
            {
                render.scaleCamera(-1);
            }
            break;
            case GLFW_KEY_0:
            {
                render.resetCameraZoom();
            }
            break;
            }
        }

    });

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiID dock_main_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),ImGuiDockNodeFlags_PassthruCentralNode);
        ImGuiContext* g = ImGui::GetCurrentContext();
        
        ImGuiDockNode* node = ImGui::DockContextFindNodeByID(g, dock_main_id);
        if (!node->IsSplitNode()) {
            ImGuiID dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
            ImGuiID dock_id_draw = ImGui::DockBuilderSplitNode(dock_id_prop, ImGuiDir_Down, 0.50f, NULL, &dock_id_prop);

            ImGui::DockBuilderDockWindow("DrawList", dock_id_draw);
            ImGui::DockBuilderDockWindow("Properties", dock_id_prop);
            
            ImGui::DockBuilderFinish(dock_main_id);
        }
        
        
        
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File", !isLoading))
            {
                if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open def")) {
                    strcpy(loadFile, ".def");
                    FileDialog::filePath = "\0";
                }
                if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open sff")) {
                    strcpy(loadFile, ".sff");
                    FileDialog::filePath = "\0";
                }
                if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open air")) {
                    strcpy(loadFile, ".air");
                    FileDialog::filePath = "\0";
                }
                if (ImGui::MenuItem("   Reload", "Ctrl+R")) {
                    isLoading = true;
                    loadingResult = std::async(&Char::reload, &character);
                }
                if (ImGui::MenuItem(ICON_FA_DOOR_OPEN " Quit", "Alt+F4")) break;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View", !isLoading))
            {
                if (ImGui::MenuItem(ICON_FA_MAGNIFYING_GLASS_PLUS " Zoom In")) {
                    render.scaleCamera(1);
                }
                if (ImGui::MenuItem(ICON_FA_MAGNIFYING_GLASS_MINUS " Zoom Out")) {
                    render.scaleCamera(-1);
                }
                if (ImGui::MenuItem("   Reset Position")) {
                    render.resetCameraPosition();
                }
                if (ImGui::MenuItem("   Reset Zoom")) {
                    render.resetCameraZoom();
                }
                if (ImGui::MenuItem(render.drawAxis ? ICON_FA_CHECK" Draw XY axis" : "   Toggle XY axis")) {
                    render.toggleXYAxis();
                }
                if (ImGui::BeginMenu("   Color"))
                {
                    ImGui::ColorEdit3("Background Color##clearColor", (float*)&setting.clearColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit3("Axis Color##axisColor", (float*)&setting.axisColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (ImGui::Begin("DrawList")) {
            bool disableButton = isLoading;
            if (disableButton) {
                ImGui::BeginDisabled(true);
            }
            if (ImGui::Button("\xef\x81\xa7")) {
                if (character.air.animList.size() > 0) {
                    DrawListItem item(character.air.animList[0], 0);
                    item.animIndex = 0;
                    auto s = character.sff.spriteIndexMap.find(std::pair<int, int>{item.anim->elements[0]->spriteGroup, item.anim->elements[0]->spriteIndex});
                    if (s != character.sff.spriteIndexMap.cend()) {
                        item.spr = s->second;
                    }
                    else {
                        item.spr = nullptr;
                    }
                    render.drawList.items.push_back(item);
                }
                else {
                    DrawListItem item(nullptr, 0);
                    item.animIndex = -1;
                    item.spr = nullptr;
                    render.drawList.items.push_back(item);
                }

            }
            if (disableButton) {
                ImGui::EndDisabled();
            }
            disableButton = isLoading || render.drawList.selectedIndex == -1;
            if (disableButton) {
                ImGui::BeginDisabled(true);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_MINUS)) {
                render.drawList.items.erase(render.drawList.items.begin() + render.drawList.selectedIndex);
                render.drawList.selectedIndex = -1;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_COPY)) {
                DrawListItem item(nullptr, 0);
                memcpy(&item, &render.drawList.items[render.drawList.selectedIndex], sizeof(item));
                render.drawList.items.push_back(item);
            }

            if (disableButton) {
                ImGui::EndDisabled();
            }
            disableButton = isLoading || render.drawList.selectedIndex <= 0;
            if (disableButton) {
                ImGui::BeginDisabled(true);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROW_UP)) {
                std::iter_swap(render.drawList.items.begin() + render.drawList.selectedIndex, render.drawList.items.begin() + render.drawList.selectedIndex - 1);
                render.drawList.selectedIndex--;
            }
            if (disableButton) {
                ImGui::EndDisabled();
            }
            disableButton = isLoading || render.drawList.selectedIndex >= render.drawList.items.size() - 1;
            if (disableButton) {
                ImGui::BeginDisabled(true);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROW_DOWN)) {
                std::iter_swap(render.drawList.items.begin() + render.drawList.selectedIndex, render.drawList.items.begin() + render.drawList.selectedIndex + 1);
                render.drawList.selectedIndex++;
            }
            if (disableButton) {
                ImGui::EndDisabled();
            }

            ImGui::BeginChild("list");
            if (ImGui::BeginTable("drawList", 2)) {
                if (render.drawList.getSize() > 0 && !isLoading) {
                    ImGui::TableSetupColumn("item", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("action", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize(ICON_FA_EYE_SLASH)[0] * 1.5);
                    int count = 0;
                    for (auto& item : render.drawList.items) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(count);
                        ImGui::AlignTextToFramePadding();
                        if (ImGui::Selectable(item.anim == nullptr ? "-1" : std::to_string(item.anim->animNumber).c_str(), render.drawList.selectedIndex == count)) {
                            render.drawList.selectedIndex = count;
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::Button(item.visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH, ImVec2{ ImGui::CalcTextSize(ICON_FA_EYE_SLASH)[0] * 1.5f,0 })) {
                            item.visible ^= 1;
                        }
                        ImGui::PopID();
                        count++;
                    }
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
        }
        ImGui::End();
        auto updateAnimSpr = [](DrawListItem* item) {
            item->anim = character.air.animList[item->animIndex];
            item->animNumber = item->anim->animNumber;
            if (item->anim->elements.size() <= item->element) {
                item->element = item->anim->elements.size() - 1;
            }
            AnimationElement* elem = item->anim->elements[item->element];
            auto result = character.sff.spriteIndexMap.find(std::pair<int, int>{elem->spriteGroup, elem->spriteIndex});
            if (result == character.sff.spriteIndexMap.end()) {
                item->spr = nullptr;
            }
            else {
                item->spr = result->second;
            }

        };
        {


            static bool listVisible = false;
            if (ImGui::Begin("Properties", NULL, listVisible ? ImGuiWindowFlags_NoBringToFrontOnFocus : NULL)) {
                if (!isLoading && render.drawList.selectedIndex != -1 && render.drawList.getSize() > render.drawList.selectedIndex) {
                    DrawListItem* item = &render.drawList.items[render.drawList.selectedIndex];
                    if (ImGui::customCombobox(character.air.animNameList, item->animIndex, listVisible)) {
                        updateAnimSpr(item);
                        ImGui::SetWindowFocus();
                    }
                    ImGui::PushButtonRepeat(true);
                    bool disableButton = item->animIndex <= 0;
                    if (disableButton) {
                        ImGui::BeginDisabled(true);
                    }
                    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                    ImGui::SameLine();
                    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
                        item->animIndex--;
                        updateAnimSpr(item);
                    }
                    if (disableButton) {
                        ImGui::EndDisabled();
                    }
                    disableButton = item->animIndex >= character.air.animList.size() - 1;
                    if (disableButton) {
                        ImGui::BeginDisabled(true);
                    }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##right", ImGuiDir_Right) && render.drawList.items[render.drawList.selectedIndex].animIndex < character.air.animList.size()) {
                        item->animIndex++;
                        updateAnimSpr(item);
                    }
                    if (disableButton) {
                        ImGui::EndDisabled();
                    }
                    ImGui::PopButtonRepeat();

                    if (ImGui::SliderInt("Element", &item->element, item->anim == nullptr ? -1 : 0, item->anim == nullptr ? -1 : item->anim->elements.size() - 1)) {
                        if (item->anim != nullptr) {

                            Animation* anim = item->anim;
                            AnimationElement* elem = anim->elements[item->element];
                            auto result = character.sff.spriteIndexMap.find(std::pair<int, int>{elem->spriteGroup, elem->spriteIndex});
                            if (result == character.sff.spriteIndexMap.end()) {
                                item->spr = nullptr;
                            }
                            else {
                                item->spr = result->second;
                            }
                        }
                    }

                    ImGui::DragFloat2("Pos", &item->pos[0], 1, -2000, 2000);
                    ImGui::DragFloat2("Scale", &item->scale[0], 0.05, 0, 999);
                    ImGui::DragFloat3("Angle", &item->angle[0], 0.2, -720, 720);
                    ImGui::DragFloat("Focal Length", &item->focalLength, 1, 1, 65535);


                    if (ImGui::BeginCombo("Facing", (item->flip == Flip::H || item->flip == Flip::VH ? "-1" : "1"), 0))
                    {
                        const bool is_selected = (item->flip == Flip::H || item->flip == Flip::VH);
                        if (ImGui::Selectable("1", is_selected)) {
                            item->flip = (item->flip == Flip::None || item->flip == Flip::H ? Flip::None : Flip::V);
                        }
                        if (ImGui::Selectable("-1", is_selected)) {
                            item->flip = (item->flip == Flip::None || item->flip == Flip::H ? Flip::H : Flip::VH);
                        }

                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                    if (ImGui::BeginCombo("vFacing", (item->flip == Flip::V || item->flip == Flip::VH ? "-1" : "1"), 0))
                    {
                        const bool is_selected = (item->flip == Flip::V || item->flip == Flip::VH);
                        if (ImGui::Selectable("1", is_selected)) {
                            item->flip = (item->flip == Flip::None || item->flip == Flip::V ? Flip::None : Flip::V);
                        }
                        if (ImGui::Selectable("-1", is_selected)) {
                            item->flip = (item->flip == Flip::None || item->flip == Flip::V ? Flip::V : Flip::VH);
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                    if (ImGui::BeginCombo("Projection", (item->projection == Projection::Orthographic ? "Orthographic" : "Perspective2"), 0))
                    {
                        bool is_selected = item->projection == Projection::Orthographic;
                        if (ImGui::Selectable("Orthographic", is_selected)) {
                            item->projection = Projection::Orthographic;
                        }
                        is_selected = item->projection == Projection::Perspective2;
                        if (ImGui::Selectable("Perspective2", is_selected)) {
                            item->projection = Projection::Perspective2;
                        }

                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                }
            }
            ImGui::End();
        }
        if (loadFile[0] != '\0') {
            if (!FileDialog::showFileDialog(loadFile)) {
                if (FileDialog::filePath != "\0") {
                    isLoading = true;
                    setting.filePath = FileDialog::filePath.substr(0, FileDialog::filePath.find_last_of("\\"));
                    if (strcmp(loadFile, ".def") == 0) {
                        loadingResult = std::async(&Char::loadDef, &character, FileDialog::filePath);
                    }
                    else if (strcmp(loadFile, ".air") == 0) {
                        loadingResult = std::async(&Air::loadAir, &character.air, FileDialog::filePath);
                    }
                    else if (strcmp(loadFile, ".sff") == 0) {
                        loadingResult = std::async(&Sff::loadSff, &character.sff, FileDialog::filePath);
                    }

                }
                loadFile[0] = '\0';
            }
        }
        if (isLoading) {
            if (loadingResult.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
                LoadingIndicatorCircle();
            }
            else {
                Utility::ExecutionResult<int> result = loadingResult.get();
                if (!result) {
                    popupMessage = result.getMessage();
                }
                else {
                    for (auto& item : render.drawList.items) {
                        auto a = std::find_if(character.air.animList.begin(), character.air.animList.end(), [&item](auto anim) {
                            return anim->animNumber == item.animNumber;
                            });
                        if (a != character.air.animList.end()) {
                            item.anim = *a;
                            if (item.element >= item.anim->elements.size()) {
                                item.element = item.anim->elements.size() - 1;
                            }
                            updateAnimSpr(&item);
                        }
                        else {
                            item.animIndex = -1;
                            item.anim = nullptr;
                            item.spr = nullptr;
                        }
                    }
                }
                isLoading = false;
            }

        }
        if (popupMessage != "") {
            ImGui::OpenPopup("Message");
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(500, 0));
            if (ImGui::BeginPopupModal("Message", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextWrapped(popupMessage.c_str());

                ImGuiStyle& style = ImGui::GetStyle();
                float width = ImGui::CalcTextSize("ok").x + style.FramePadding.x * 2.0f;
                float avail = ImGui::GetContentRegionAvail().x;
                float off = (avail - width) * 0.5f;
                if (off > 0.0f) {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
                }
                if (ImGui::Button("OK")) {
                    popupMessage = "";
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }


        // Rendering
        ImGui::Render();
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        if (!isLoading) {
            render.drawScene(display_w, display_h);
        }
        

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glClearColor(setting.clearColor[0], setting.clearColor[1], setting.clearColor[2], 1.0f);
        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}