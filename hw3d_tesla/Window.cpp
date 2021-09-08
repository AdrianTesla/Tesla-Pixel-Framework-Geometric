#include "Window.h"
#include <sstream>
#include "resource.h"
#include "imgui/imgui_impl_win32.h"

Window::WindowClass Window::WindowClass::wndClass;  // <-- static

Window::WindowClass::WindowClass() noexcept
	:
	hInst(GetModuleHandle(nullptr))
{
	// Fill the Window Class descriptor
	WNDCLASSEX wc     = { 0 };
	wc.cbSize         = sizeof(wc);
	wc.style          = CS_OWNDC;
	wc.lpfnWndProc    = HandleMsgSetup;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = hInst;
	wc.hIcon          = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 256,256,0)); // the big icon
	wc.hCursor        = nullptr;
	wc.hbrBackground  = nullptr;
	wc.lpszMenuName   = nullptr;
	wc.lpszClassName  = wndClassName;
	wc.hIconSm        = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16,16, 0)); // the small icon

	// Register the window class
	RegisterClassEx(&wc);
}

const char* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, hInst);
}

Window::Window(int width, int height, const char* title, int pos_x, int pos_y)
	:
	width(width),
	height(height),
	cursorEnabled(true),
	title(title)
{
	RECT wr;
	wr.left   = pos_x;
	wr.right  = width + wr.left;
	wr.top    = pos_y;
	wr.bottom = height + wr.top;

	if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
	{
		throw TESLA_WND_LAST_EXCEPT();
	}

	// Create a handle to the window, choose styles and show the window;
	hWnd = CreateWindow(
		WindowClass::GetName(),
		title,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,// | WS_MAXIMIZEBOX, // | WS_SIZEBOX, <-- resizable?
		pos_x,
		pos_y,
		wr.right - wr.left,
		wr.bottom - wr.top,
		nullptr,
		nullptr,
		WindowClass::GetInstance(),
		this  // <--- we pass the pointer to the window to the lParam, catching it in the WC_NCCREATE message
	);

	if (hWnd == nullptr)
	{
		throw TESLA_WND_LAST_EXCEPT();
	}
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	// Register a Mouse RAW input device, so we can get WM_INPUT data
	RAWINPUTDEVICE raw_device;
	raw_device.usUsagePage = 0x01;
	raw_device.usUsage     = 0x02;
	raw_device.dwFlags     = 0;
	raw_device.hwndTarget  = nullptr;
	if (RegisterRawInputDevices(&raw_device, 1u, sizeof(raw_device)) == FALSE)
	{
		throw TESLA_WND_LAST_EXCEPT();
	}
	
	// Give ImGui the handle to the created window
	if (!ImGui_ImplWin32_Init(hWnd))
	{
		throw std::runtime_error("ImGui couldn't take the handle to the hWnd");
	}
}

Window::~Window()
{
	DestroyWindow(hWnd);
	ImGui_ImplWin32_Shutdown();
}

void Window::SetTitle(const std::string& title)
{
	this->title = title;
	if (SetWindowText(hWnd, title.c_str()) == FALSE)
	{
		throw TESLA_WND_LAST_EXCEPT();
	}
}

std::string Window::GetTitle() const noexcept
{
	return title;
}

void Window::EnableCursor() noexcept
{
	cursorEnabled = true;
	ShowCursor();
	FreeCursor();
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void Window::DisableCursor() noexcept
{
	cursorEnabled = false;
	HideCursor();
	ConfineCursor();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

void Window::ToggleCursorState() noexcept
{
	if (cursorEnabled)
	{
		DisableCursor();
		mouse.EnableRawInput();
	}
	else
	{
		EnableCursor();
		mouse.DisableRawInput();
	}
}

bool Window::IsCursorEnabled() const noexcept
{
	return cursorEnabled;
}

std::optional<int> Window::ProcessMessages() noexcept
{
	// Message pump:
	MSG msg;
	// Keep dispatching messages until there are messages in the queue
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// If the message was a quit message...
		if (msg.message == WM_QUIT)
		{
			return (int)msg.wParam;
		}
		// Standard translation and dispatching of the messages
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return {};
}

HWND Window::GetHwnd() const noexcept
{
	return hWnd;
}

void Window::HideCursor() const noexcept
{
	while (::ShowCursor(FALSE) >= 0) {}
}

void Window::ShowCursor() const noexcept
{
	while (::ShowCursor(TRUE) < 0) {}
}

void Window::ConfineCursor() const noexcept
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2u);
	ClipCursor(&rect);
}

void Window::FreeCursor() const noexcept
{
	ClipCursor(nullptr);
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
	if (msg == WM_NCCREATE)
	{
		// Extract ptr to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		// Set WinAPI-managed user data to store ptr to window instance
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// Set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
		// Forward message to window instance handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	// If we get a message before the WM_NCCREATE message, handle with default handler
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Retrieve ptr to window instance
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	// Forward message to window instance handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

/////////////////////////////////// EXCEPTION DARK LAND ///////////////////////////////////////
Window::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr(hr)
{
}

const char* Window::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept
{
	return "Tesla Window Exception";
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	const DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf),
		0,
		nullptr
	);
	if (nMsgLen == 0)
	{
		return "Unidentified Error Code";
	}
	std::string errorString = pMsgBuf;
	LocalFree(pMsgBuf);
	return errorString;
}

HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Here we handle all the messages!
	// Let ImGui take care of messages if it needs to
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}

	// Otherwise, we handle the messages
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0; 
	/****************************************************/
	/**************KEYBOARD MESSAGES*********************/
	case WM_KILLFOCUS:
		kbd.ClearState();
		FreeCursor();
		ShowCursor();
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!(lParam & 0x40000000) || kbd.AutorepeatIsEnabled())
		{
			kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_CHAR:
		kbd.OnChar(static_cast<unsigned char>(wParam));
		break;
	/**************END KEYBOARD MESSAGES*****************/
	/****************************************************/

	/****************************************************/
	/******************MOUSE MESSAGES********************/
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		// Keep getting mouse move messages if the user clicks and drags
		// the cursor outside of the client region.

		// If the cursor is inside the client region
		if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height)
		{
			mouse.OnMouseMove(pt.x, pt.y);
			// If mouse is not in window
			if (!mouse.IsInWindow())
			{
				// This throws strange exceptions when things go wrong.
				SetCapture(hWnd);
				mouse.OnMouseEnter();
			}
		}
		else
		{
			//// If the cursor is outside the client region and the user is "dragging"
			if (mouse.LeftIsPressed() || mouse.RightIsPressed())
			{
				mouse.OnMouseMove(pt.x, pt.y);
			}
			else
			{
				// Otherwise, release the capture and leave the client region
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		// If the cursor is disabled
		if (!IsCursorEnabled())
		{
			HideCursor();
			ConfineCursor();
		}
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftIsPressed(pt.x, pt.y);
		break;
	}
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftIsReleased(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightIsPressed(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightIsReleased(pt.x, pt.y);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnWheelDelta(pt.x, pt.y, GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	}
	case WM_MBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnMiddlePress(pt.x, pt.y);
		break;
	}
	case WM_MBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnMiddleRelease(pt.x, pt.y);
		break;
	}
	case WM_ACTIVATE:
	{
		if (!IsCursorEnabled())
		{
			if (wParam & WA_CLICKACTIVE)
			{
				ConfineCursor();
				HideCursor();
			}
			else
			{
				FreeConsole();
				ShowCursor();
			}
		}
		break;
	}
	case WM_INPUT:  // RAW input message
	{
		if (!mouse.IsRawInputEnabled())
		{
			break;
		}
		// We need the size of the input data first
		UINT size;
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1)
		{
			throw std::runtime_error("Couldn't get the size of the Mouse Raw input data");
			break;
		}
		// Now we have the size of the data, so we resize the rawBuffer accordingly
		rawBuffer.resize(size);
		// So we call the function again getting the actual Raw data:
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
		{
			throw std::runtime_error("The size of the RAW data doesn't match the calculated size");
			break;
		}
		// Now we have the rawBuffer: let's get the raw_input structure from the rawBuffer
		auto& raw_input = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
		// Process the Raw input only if it's from the mouse and there was an actual change in movement 
		if (raw_input.header.dwType == RIM_TYPEMOUSE && (raw_input.data.mouse.lLastX != 0) || (raw_input.data.mouse.lLastY != 0))
		{
			mouse.OnRawInputDelta(raw_input.data.mouse.lLastX, raw_input.data.mouse.lLastY);
		}
		break;
	}
	/****************END MOUSE MESSAGES******************/
	/****************************************************/

	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

std::string Window::HrException::GetErrorDescription() const noexcept
{
	return Exception::TranslateErrorCode(hr);
}