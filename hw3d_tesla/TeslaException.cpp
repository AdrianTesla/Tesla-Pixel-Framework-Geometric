#include "TeslaException.h"
#include <sstream>

TeslaException::TeslaException(int line, const char* file) noexcept
	:
	line(line),
	file(file)
{}

const char* TeslaException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* TeslaException::GetType() const noexcept
{
	return "Tesla Exception";
}

int TeslaException::GetLine() const noexcept
{
	return line;
}

const std::string& TeslaException::GetFile() const noexcept
{
	return file;
}

std::string TeslaException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}