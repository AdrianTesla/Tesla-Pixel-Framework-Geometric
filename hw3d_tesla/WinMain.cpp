#include "Window.h"
#include "Game.h"

// This is the entry point of our application
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try
	{
		// Create a game object
		Game game;
		// Start the game loop
		while (true)
		{
			// Process windows messages
			if (const auto error_code = Window::ProcessMessages())
			{
				return *error_code;
			}
			// Do a frame composition
			game.Go();
		}
	}
	catch (const TeslaException& e)
	{
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}