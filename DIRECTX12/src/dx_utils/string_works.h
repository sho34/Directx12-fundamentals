#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

inline std::string wchar_to_char(const wchar_t* widestr)
{
	// query the buffer size
	int buffer_size{ WideCharToMultiByte(CP_UTF8, 0, widestr, -1, NULL, 0, NULL, NULL) };
	if (buffer_size == 0)
	{
		
		::OutputDebugStringW(L"ERROR IN WSTRING CONVERSION SIZE CHECK\n");
	}
	
	char* multibyte_str = new char[buffer_size];
	// convert from wide strings to ansi strings
	int result{ WideCharToMultiByte(CP_UTF8, 0, widestr, -1, multibyte_str, buffer_size, NULL, NULL) };
	if (result == 0)
	{
		::OutputDebugStringW(L"ERROR IN WSTRING CONVERSION\n");
	}

	return std::string(multibyte_str);
}
