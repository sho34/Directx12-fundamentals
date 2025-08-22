#include "App.h"

int WINAPI WinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ PSTR pCmdLine, 
	_In_ int nCmdShow
)
{
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	try
	{
		return App{}.Run();
	}

	catch (const Window::Exception& e)
	{
		MessageBoxA(
			nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION
		);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(
			nullptr, e.what(), "STANDARD EXCEPTION", MB_OK | MB_ICONEXCLAMATION
		);
	}
	catch (...)
	{
		MessageBoxA(nullptr, "no details available", "Unkown exception", MB_OK | MB_ICONEXCLAMATION);
	}

	return 0;
}