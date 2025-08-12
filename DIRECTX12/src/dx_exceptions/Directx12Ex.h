#pragma once 
#ifndef __DirectX12Ex__
#define __DirectX12Ex__

#include "Infoman.h"
#include "dxerr.h"
#include <comdef.h>

// to handle the graphics exceptions 
class DxException
{
public:
	DxException() = default;
	DxException(
		HRESULT hr, const std::wstring& functionName,
		const std::wstring& filename, int lineNumber,
		std::vector<std::string> Errinfo = {}
	);
	std::wstring what() const;

private:
	std::string GetErrorInfo() const;

private:
	std::string info;
	// for the DxExceptions
	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring FileName;
	int LineNumber = -1;
};


#endif // !__DirectX12Ex__
