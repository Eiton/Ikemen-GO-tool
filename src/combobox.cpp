#include "imgui.h"
#include <string>
#include <vector>
namespace ImGui {
    bool customCombobox_drawOptions() {
        if (IsWindowFocused() &&
            !IsAnyItemActive() && !IsMouseClicked(0))
        {
            SetKeyboardFocusHere(-1);
        }

        return false;
    }
    //template <class T>
    bool customCombobox(std::vector<char*>& options, int& selectedItemIndex, bool& listVisible) {

        static int selectIndex = -1;
        static bool focusOption = false;
        static bool updateInputText = false;
        static int optionSize = options.size();

        static bool listActive = false;

        static std::vector<char*>* o = &options;
        struct Funcs
        {
            static int MyCallback(ImGuiInputTextCallbackData* data)
            {
                if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
                {
                    if (data->EventKey == ImGuiKey_UpArrow)
                    {
                        selectIndex--;
                        if (selectIndex < 0) {
                            selectIndex = optionSize - 1;
                        }
                    }
                    else if (data->EventKey == ImGuiKey_DownArrow)
                    {
                        selectIndex++;
                        if (selectIndex > optionSize) {
                            selectIndex = optionSize - 1;
                        }
                    }
                    focusOption = true;
                    updateInputText = true;
                }
                else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
                {
                    updateInputText = false;
                    bool found = false;
                    for (int i = 0; i < (*o).size() && !found; i++) {
                        bool notMatch = false;
                        for (int j = 0; j < data->BufTextLen && !notMatch; j++) {
                            if ((*o)[i][j] != data->Buf[j]) {
                                notMatch = true;
                            }
                        }
                        if (!notMatch) {
                            selectIndex = i;
                            focusOption = true;
                            found = true;
                        }
                    }
                }
                else if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
                {
                    if (selectIndex >= 0 && updateInputText) {
                        strcpy(data->Buf, (*o)[selectIndex]);
                        data->BufTextLen = strlen((*o)[selectIndex]);
                        data->CursorPos = data->BufTextLen;
                        data->BufDirty = true;
                    }
                }
                return 0;
            }
        };


        static char inputBuffer[256];
        if (InputText("##Anim Number", inputBuffer, 256, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackAlways, Funcs::MyCallback)) {
            listVisible = false;
            selectedItemIndex = selectIndex;
            return true;
        }

        listActive = false;
        if (IsItemActive() || listVisible) {
            ImGuiIO& io = ImGui::GetIO();
            // draw options
            PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
            ImVec2 pos = GetItemRectMin();
            pos.y += GetItemRectSize().y;
            SetNextWindowPos(pos);
            SetNextWindowSize(ImVec2{ GetItemRectSize().x - 60, GetFrameHeightWithSpacing() * 8 });
            Begin("input_popup", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings);
            PushAllowKeyboardFocus(false);
            for (int i = 0; i < options.size(); i++)
            {
                PushID(i);
                if (ImGui::Selectable(options[i], selectIndex == i)) {
                    selectIndex = i;
                    strcpy(inputBuffer, (*o)[selectIndex]);
                    selectedItemIndex = selectIndex;
                    listVisible = false;
                    PopID();
                    PopAllowKeyboardFocus();
                    End();
                    PopStyleVar(1);
                    return true;
                }
                if (IsItemHovered() && ((io.MouseDelta[0] != 0 && io.MouseDelta[1] != 0) || io.MouseWheel != 0)) {
                    selectIndex = i;
                    updateInputText = true;
                }
                if (selectIndex == i && focusOption) {
                    SetScrollHereY();
                }
                if (IsItemActive()) {
                    listActive = true;
                }
                PopID();
            }
            PopAllowKeyboardFocus();
            focusOption = false;
            if (IsWindowFocused()) {
                listActive = true;
            }



            End();
            PopStyleVar(1);



            if (!listVisible) {
                listVisible = true;
                inputBuffer[0] = '\0';
                SetKeyboardFocusHere(-1);
                return false;
            }
        }
        if ((!IsItemActive() || !IsWindowFocused()) && !listActive) {
            listVisible = false;
            if (selectedItemIndex != -1) {
                strcpy(inputBuffer, options[selectedItemIndex]);
            }
            selectIndex = selectedItemIndex;
        }
        return false;
    }
	
}