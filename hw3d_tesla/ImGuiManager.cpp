#include "ImGuiManager.h"
#include "imgui\imgui.h"

ImGuiManager::ImGuiManager()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();
}

ImGuiManager::~ImGuiManager()
{
	ImGui::DestroyContext(); 
}