#pragma once
#ifndef __exception_macros__
#define __exception_macros__

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	::ZeroMemory(&buffer, 512);
	::MultiByteToWideChar(0, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);

}


#ifndef THROW_GRAPHICS_INFO
/*
* - Will trigger an exception that contains all the raw information about the error
*		from the debug window.
*/
#define THROW_GRAPHICS_INFO(err_code)														\
{																							\
	InfoManager					info_manager;												\
	HRESULT						hr_res = (err_code);										\
\
	std::wstring wfn = AnsiToWString(__FILE__);												\
\
	if(FAILED(hr_res))																		\
	{																						\
		throw DxException(hr_res, L#err_code, wfn, __LINE__, info_manager.GetMessages());	\
	}																						\
}
#endif

#endif
