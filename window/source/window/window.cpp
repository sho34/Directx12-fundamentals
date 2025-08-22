#include "window.h"
#include "resource.h"
#include "except_macros.h"
#include "imgui_graphics/imgui_gfx_window.h"



Window::WindowClass Window::WindowClass::wndClass;

const char* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}

Window::WindowClass::WindowClass()
	:hInst(GetModuleHandle(nullptr))
{
	WNDCLASSEX wc = {};
	// fill in the properties that we want for the window. 
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = HandleMsgSetUp;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();

	wc.hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_ICON3));
	wc.hIconSm = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_ICON4));
	wc.hCursor = nullptr;

	wc.hbrBackground = nullptr;
	wc.lpszClassName = (LPCTSTR)GetName();
	wc.lpszMenuName = nullptr;
	wc.style = CS_OWNDC;

	// register window class 
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass((LPCWSTR)GetName(), GetInstance());
}

Window::Window(int width, int height, std::wstring name)
	:width(width), height(height)
{
	// calculate the desired window size based on desired client region size 
	RECT wr = {};
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;


	if (AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE) == 0)
	{
		throw LastException();
	}

	// create the window 
	hWnd = CreateWindowEx(
		0, (LPCWSTR)WindowClass::GetName(),
		(LPCWSTR)name.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		(wr.right - wr.left), (wr.bottom - wr.top),
		nullptr, nullptr,
		WindowClass::GetInstance(),
		this	// This is a pointer to our window instance 
	);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
}

Window::~Window()
{
	// destroy the window 
	DestroyWindow(hWnd);
}

std::optional<int> Window::ProcessMessages()
{
	MSG msg;
	// while queue has messages, remove them but do not block on halt
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// check for quit message because peek message does not quit via return 
		if (msg.message == WM_QUIT)
		{
			return msg.wParam;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return {};
}

HWND Window::GetHwnd() const
{
	return hWnd;
}

int Window::GetWidth() const
{
	return width;
}

int Window::GetHeight() const
{
	return height;
}

void Window::CalculateFrameStats()
{
}

void Window::setFullScreen(bool fullscreen)
{
	if (isFullScreen != fullscreen)
	{
		isFullScreen = fullscreen;

		if (isFullScreen)
		{
			::GetWindowRect(this->hWnd, &windowRect);
			UINT windowStyle = WS_OVERLAPPEDWINDOW & ~WINDOW_FRAME;
			::SetWindowLongW(this->hWnd, GWL_STYLE, windowStyle);

			HMONITOR hMonitor = ::MonitorFromWindow(this->hWnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorInfo = {};
			monitorInfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfo(hMonitor, &monitorInfo);



			::SetWindowPos(
				this->hWnd, HWND_TOP,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE
			);

			::ShowWindow(this->hWnd, SW_MAXIMIZE);
		}
		else
		{
			// Restore all the window decorators.

			::SetWindowLong(this->hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			::SetWindowPos(
				this->hWnd, HWND_NOTOPMOST,
				this->windowRect.left,
				this->windowRect.top,
				this->windowRect.right - this->windowRect.left,
				this->windowRect.bottom - this->windowRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE
			);


			::ShowWindow(this->hWnd, SW_NORMAL);

		}

		// need to disable the ALT ENTER FULLSCREEN METHOD.

	}
}

LRESULT Window::HandleMsgSetUp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (msg == WM_NCCREATE)
	{
		// extract pointer to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);

		// set WinAPI-managed user data to store pointer to window class
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// set message proceedure to normal(non-setup) now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));

		// forward the message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}

	// if we get a message before the WM_NCCREATE message, handle with default behavior
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{	/*
	* adapts from the win32 calling convention to the cpp standard calling convension
	*/
	// retrieve pointer to window class 
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	// forward message to window class handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}


LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// store the windows messages in an external queue to use them later.
	{
		std::lock_guard<std::mutex> lock(queue_mutex);
		message_queue.push({ msg, wParam, lParam });
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

	switch (msg)
	{
	case WM_PAINT:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		// clear keystate when window loses focus to prevent input getting 
	case WM_KILLFOCUS:
		kbd.ClearState();
		break;

		/*************** KEYBOARD MESSAGES *************************/
	case WM_KEYDOWN:
	{
		bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
		switch (wParam)
		{
		case VK_RETURN:
		{
			if(alt)
			{
		case VK_F11:
			setFullScreen(!isFullScreen);
			break;
		default:
			break;
			}
		}

		}
	}
	case WM_SYSKEYDOWN:

		if (!(lParam & 0x40000000) || kbd.AutoRepeatIsEnabled())
		{
			kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_CHAR:
		kbd.OnChar(static_cast<unsigned char>(wParam));
		break;
		/************** END OF KEYBOARD MESSAGES *********************/

		/******************** MOUSE MESSAGES ********************/
	case WM_MOUSEMOVE:
	{
		POINTS pt = MAKEPOINTS(lParam);
		// in client region  -> log move, and log enter + capture mouse 
		if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height)
		{
			mouse.OnMouseMove(pt.x, pt.y);
			if (!mouse.isInWindow())
			{
				SetCapture(hWnd);
				mouse.OnMouseEnter();
			}
		}
		// not in client -> log move / maintain capture if buttpn is down 
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON))
			{
				mouse.OnMouseMove(pt.x, pt.y);
			}
			else
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftPressed(pt.x, pt.y);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightpressed(pt.x, pt.y);
	}
	break;
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftReleased(pt.x, pt.y);
	}
	break;
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		mouse.OnWheelDelta(pt.x, pt.y, delta);
	}
	break;
	case WM_ACTIVATE:
	{
		//activate the timer when the application starts.
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			appPaused = true;
		}
		else
		{
			appPaused = false;
		}
	}
	break;

	/***************WHEN THE WINDOW IS RESIZED ***************/
	// i have to create a system that saves the window resizing messages 
	case WM_SIZE:
		// save the new client area dimensions 
		width = LOWORD(lParam);
		height = HIWORD(lParam);

		{
			if (wParam == SIZE_MINIMIZED)
			{
				appPaused = true;
				minimized = true;
				maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				appPaused = false;
				minimized = false;
				maximized = true;
			}
			/// restoring from maximized state
			else if (wParam == SIZE_RESTORED)
			{
				if (minimized)
				{
					appPaused = false;
					minimized = false;
				}
				else if (maximized)
				{
					appPaused = false;
					maximized = false;
				}
			}
		}
		break;
		// when the user grabs the resize bars
	case WM_ENTERSIZEMOVE:
		appPaused = true;
		resizing = true;
		break;
		// when the user releases the resize bars
	case WM_EXITSIZEMOVE:
		appPaused = false;
		resizing = false;
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


/*****************************************************************************************
* -> The window exceptions code 
*****************************************************************************************/

Window::Exception::Exception(int line, const char* file, HRESULT hr) noexcept
	:wExceptions(line, file), hr(hr)
	// calls the constructor for base exception class and initialise the HRESULT hr
{}

const char* Window::Exception::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl << "[Error code] 0x"
		<< std::hex << GetErrorCode() << "(" << std::dec << GetErrorCode() << ")"
		<< std::endl << "[Description]"
		<< GetErrorString() << std::endl << GetOriginString();

	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::Exception::GetType() const noexcept
{
	return "window exceptions";
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{	
	// made static so it can be called from outside this function for general translation
	char* pMsgBuff = nullptr;

	/*
	* - FormatMessage() takes hr and gives the description of that error code
	*	it creates a buffer where it stores the error string.
	*
	* - we give it pMsgBuff, it will make it point to the buffer with the error string
	*/
	DWORD nMsgLen = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&pMsgBuff), 0, nullptr
	);

	if (nMsgLen == 0)
	{
		return "unidentified error code";
	}

	std::string errorString = pMsgBuff;
	LocalFree(pMsgBuff); // free the previoulsy created buffer 

	OutputDebugStringA((LPCSTR)errorString.c_str());
	return errorString;
}

HRESULT Window::Exception::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::Exception::GetErrorString() const noexcept
{
	return TranslateErrorCode(hr);
}
