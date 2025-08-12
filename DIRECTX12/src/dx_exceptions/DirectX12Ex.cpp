#include "Directx12Ex.h"

/// ************************************ EXCEPTIONS ************************************ 

DxException::DxException(
	HRESULT hr, const std::wstring& functionName,
	const std::wstring& filename, int lineNumber,
	std::vector<std::string> Errinfo
)
	:
	ErrorCode(hr),
	FunctionName(functionName),
	FileName(filename),
	LineNumber(lineNumber)
{
	// join all info messages with newlines into single string
	for (const auto& m : Errinfo)
	{
		info += m;
		info.push_back('\n\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

std::wstring DxException::what() const
{
	// get the string description of the error code 
	char buff[512];
	_com_error err(ErrorCode);

	DXGetErrorDescriptionA(ErrorCode, buff, 512);
	std::wstringstream woss;
	woss << "*******************[EXCEPTION THREW BY]*********************\n";
	woss << FunctionName;
	woss << "\n************************** FAILED IN ***************************\n";
	woss << FileName << "\n\n";
	woss << "[ON LINE]: " << std::to_wstring(LineNumber) << "\n";
	woss << "[error code]: 0x" << std::hex << ErrorCode << std::dec << "(" << (unsigned long)ErrorCode << ")" << "\n\n";
	woss << "[error]: " << buff << "\n";

	if (!info.empty())
	{
		woss << "********************[ERROR INFOMATION]*********************" << "\n\n";
		woss << GetErrorInfo().c_str() << "\n";
	}
	return woss.str();
}

std::string DxException::GetErrorInfo() const
{
	return info;
}