#pragma once
#ifndef __except_macros__
#define __except_macros__

// window exception helper macros 
#define WinException(hr) Window::Exception(__LINE__, __FILE__, hr);
#define LastException() Window::Exception(__LINE__, __FILE__, GetLastError())
#endif