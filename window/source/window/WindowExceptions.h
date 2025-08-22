#pragma once
#ifndef __WindowExceptions__
#define __WindowExceptions__

#include <exception>
#include <sstream>
#include <string>

class wExceptions : std::exception
{
public:
	wExceptions(int line, const char* file);
	const char* what() const override;
	virtual const char* GetType() const;
	int GetLine() const;
	const std::string& GetFile() const;
	std::string GetOriginString() const;

private:
	int line;
	std::string file;

protected:
	// this was made mutable in order to set it within the what constant function.
	mutable std::string whatBuffer;

};

#endif // !__WindowExceptions__
