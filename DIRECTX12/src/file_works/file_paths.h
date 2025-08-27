#pragma once


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <sstream>


inline std::wstring get_project_dir()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(nullptr, buffer, MAX_PATH);

	std::wstring fullPath(buffer);
	size_t lastSlash = fullPath.find_last_of(L"\\/");

	return fullPath.substr(0, lastSlash); // Removes the executable name
}

inline std::wstring get_shader_loc(std::wstring shader_name)
{
	// Example usage: load_shader(L"myshader.hlsl");
	std::wstring path{ get_project_dir() };
	path += L"\\";
	path += shader_name;

	return  path;
}

#define LOAD_SHADER(__SHADER_FILE__) get_shader_loc(__SHADER_FILE__)