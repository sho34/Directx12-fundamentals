#include "WindowExceptions.h"

wExceptions::wExceptions(int line, const char* file)
    :line(line), file(file)
{}

const char* wExceptions::what() const
{    /*
    * - fill up the what buffer with the contents of oss
    * - this makes sure that when the oss dies out at the end of the function
    * - the contents don't disappear with the oss object
    */
    std::ostringstream oss;
    oss << GetType() << std::endl << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* wExceptions::GetType() const
{
    return "window exceptions";
}

int wExceptions::GetLine() const
{
    return line;
}

const std::string& wExceptions::GetFile() const
{
    // TODO: insert return statement here
    return file;
}

std::string wExceptions::GetOriginString() const
{
    std::ostringstream oss;
    oss << "[File]" << file << std::endl << "[Line]" << line;
    return oss.str();
}
