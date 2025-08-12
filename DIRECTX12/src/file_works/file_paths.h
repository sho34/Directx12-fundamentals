#pragma once


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <sstream>

inline std::wstring get_file_pathw(std::wstring path=L"")
{
	// Only for wide string types, if you want the type for ansi strings call get_file_path().
	// Fetches the dir path of your solution folder.
	// This will save you the trouble of having to write the full path of your files ralative to your :C drive.

	std::wstring shader_location = __FILEW__;
	std::wstring full_path, tmp_path{ L"\\DIRECTX12\\src\\file_works\\file_paths.h" };

	size_t path_stride{ 
		shader_location.length() - tmp_path.length() 
	};

	for (size_t i = 0; i < path_stride; ++i)
	{
		full_path = full_path + shader_location[i];
	}

	full_path += path;

	return full_path;
}

inline std::wstring concatw(std::wstring in)
{
	std::wstring res{ L"output\\Debug\\x64\\" };
	res += in;
	return res;
}

inline std::string concat(std::string in)
{
	std::string res{ "output\\Debug\\x64\\" };
	res += in;
	return res;
}

inline std::string get_file_path(std::string path = "")
{

	std::string shader_location = __FILE__;
	std::string full_path, tmp_path{ "DIRECTX12\\src\\file_works\\file_paths.h" };

	size_t path_stride{ 
		shader_location.length() - tmp_path.length() 
	};

	for (size_t i = 0; i < path_stride; ++i)
	{
		full_path = full_path + shader_location[i];
	}

	full_path += path;

	std::ostringstream oss;
	oss << "***********************SHADER PATH**********************\n";
	oss << full_path << "\n" << std::endl;

	::OutputDebugStringA(oss.str().c_str());

	return full_path;
}


#define _FPW_(__SHADER_PATH__) get_file_pathw(concatw(L#__SHADER_PATH__)).c_str()
#define _FP_(__SHADER_PATH__) get_file_path(concat(__SHADER_PATH__))