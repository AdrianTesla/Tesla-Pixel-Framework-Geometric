#pragma once
#include "Window.h"
#include "ImGuiManager.h"

class Game
{
public:
	Game();
	Game(const Game&) = delete;
	Game& operator = (const Game&) = delete;
	void Go();
private:
	void UpdateModel();
	void ComposeFrame();
	/******************************/
	/*******User Functions*********/
	/******************************/
private:
	ImGuiManager imgui;
	Window wnd;
	Graphics gfx;
	/******************************/
	/*******User Variables*********/
	/******************************/
};