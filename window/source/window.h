#pragma once
#ifndef __window__
#define __window__

#include <windows.h>
#include <string>
#include <optional>
#include <memory>

#include "mouse/mouse.h"
#include "keyboard/kbd.h"
#include "WindowExceptions.h"

#include "timer/timer.h"
#include "wm/windows_messages.h"

#define WINDOW_FRAME (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

class Window
{
public:
	class WindowClass
	{
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass();
		~WindowClass();

		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;

		// the name of the window class 
		static constexpr const char* wndClassName = "window";
		static WindowClass wndClass;
		HINSTANCE hInst = {};
	};

public:
	Window(int width, int height, std::wstring name = L"");
	~Window();

	// delete the copy and assignment constructors
	Window(const Window&) = delete; // copy
	Window& operator=(const Window&) = delete; // assignment

public:
	// process the windows messages 
	static std::optional<int> ProcessMessages();

private:
	// function methods for the window object
	static LRESULT CALLBACK HandleMsgSetUp(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
	) noexcept;

	static LRESULT CALLBACK HandleMsgThunk(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
	) noexcept;

	LRESULT CALLBACK HandleMsg(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
	) noexcept;

public:
	// the window exceptions go here 
	class Exception : wExceptions
	{
	public:
		Exception(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept override;
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;

	private:
		HRESULT hr;
	};

private:
	int			width		= {};
	int			height		= {};
	HWND		hWnd		= {};
	RECT		windowRect;

public:
	Keyboard	kbd;
	Mouse		mouse;
	Timer		timer;
	UINT		message;

public:
	bool		resizing	= false;
	bool		minimized	= false;
	bool		maximized	= false;
	bool		appPaused	= false;

public:
	// get the handle to the window 
	HWND		GetHwnd()	const;
	int			GetWidth()	const;
	int			GetHeight() const;

	void CalculateFrameStats();

public:
	LPARAM lParam = {};
	WPARAM wParam = {};

public:
	std::queue<windows_messages>	message_queue;
	std::mutex						queue_mutex;

private:
	// handle window full screen toggling.
	bool isFullScreen = false;
	void setFullScreen(bool fullscreen);
};
#endif // !__window__
