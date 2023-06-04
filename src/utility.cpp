#include "utility.h"
#include "imgui.h"
#include "imgui_internal.h"


Utility::CustomSetting setting;
void Utility::loadCustomSetting()
{
	setting.filePath = "";
	ImGuiContext& context = *ImGui::GetCurrentContext();
	ImGuiSettingsHandler ini_handler{};
	ini_handler.TypeName = "Custom settings";
	ini_handler.TypeHash = ImHashStr("Custom settings");
	ini_handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*, const char* name) {return (void*)&setting; };
	ini_handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) {
		float r, g, b;
		if (sscanf(line, "ClearColor=%f,%f,%f", &r, &g, &b) == 3) {
			setting.clearColor[0] = r;
			setting.clearColor[1] = g;
			setting.clearColor[2] = b;
			return;
		}else if (sscanf(line, "AxisColor=%f,%f,%f", &r, &g, &b) == 3) {
			setting.axisColor[0] = r;
			setting.axisColor[1] = g;
			setting.axisColor[2] = b;
			return;
		}
		int len = strlen(line);
		if (len > 9) {
			char* f = new char[strlen(line) - 8];
			if (sscanf(line, "FilePath=%[^\t\n]", f)) { setting.filePath = f; }
			delete[]f;
		}
	};
	ini_handler.WriteAllFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
		buf->appendf("[%s][]\n", handler->TypeName);
		buf->appendf("FilePath=%s\n", setting.filePath.c_str());
		buf->appendf("ClearColor=%f,%f,%f\n", setting.clearColor[0], setting.clearColor[1], setting.clearColor[2]);
		buf->appendf("AxisColor=%f,%f,%f\n", setting.axisColor[0], setting.axisColor[1], setting.axisColor[2]);
	};
	context.SettingsHandlers.push_back(ini_handler);
	ImGui::LoadIniSettingsFromDisk(context.IO.IniFilename);
}
