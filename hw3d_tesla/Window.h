#pragma once
#include "TeslaWin.h"
#include "TeslaException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include <optional>
#include <memory>

class Window
{
public:
	class Exception : public TeslaException
	{
		using TeslaException::TeslaException;
	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		virtual const char* what() const noexcept override;
		virtual const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
 	private:
		HRESULT hr;
	};
private:
	class WindowClass
	{
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator = (const WindowClass&) = delete;
		static constexpr const char* wndClassName = "hw3d_tesla";
		static WindowClass wndClass;
		HINSTANCE hInst;
	};
public:
	Window(int width, int height, const char* title, int pos_x = 300, int pos_y = 300);
	~Window();
	Window(const Window&) = delete;
	Window& operator = (const Window&) = delete;
	void SetTitle(const std::string& title);
	std::string GetTitle() const noexcept;
	void EnableCursor() noexcept;
	void DisableCursor() noexcept;
	void ToggleCursorState() noexcept;
	bool IsCursorEnabled() const noexcept;
	static std::optional<int> ProcessMessages() noexcept;
	HWND GetHwnd() const noexcept;
private:
	void HideCursor() const noexcept;
	void ShowCursor() const noexcept;
	void ConfineCursor() const noexcept;
	void FreeCursor() const noexcept;
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	Keyboard kbd;
	Mouse mouse;
private:
	int width;
	int height;
	bool cursorEnabled;
	std::vector<char> rawBuffer;
	HWND hWnd;
	std::string title;
};

#define TESLA_WND_EXCEPT(hr) Window::HrException(__LINE__, __FILE__, hr)
#define TESLA_WND_LAST_EXCEPT() Window::HrException(__LINE__, __FILE__, GetLastError())
#define TESLA_NO_GFX_EXCEPT() Window::NoGfxException(__LINE__, __FILE__)