#include "Game.h"
#include "imgui/imgui.h"

Game::Game()
	:
	wnd(Graphics::ScreenWidth * Graphics::PixelSize, Graphics::ScreenHeight * Graphics::PixelSize, "Adrian Tesla Pixel Framework", 200, 200),
	gfx(wnd.GetHwnd())
{
}

void Game::Go()
{
	gfx.BeginFrame();
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();
	wnd.SetTitle("Adrian Tesla Pixel Framework - " + gfx.GetWindowInfo());
}

void Game::UpdateModel()
{
}

void Game::ComposeFrame()
{
}