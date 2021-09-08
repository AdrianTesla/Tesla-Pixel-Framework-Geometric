#define FULL_WINTARD
#include "Surface.h"
#include "TeslaWin.h"
#include <algorithm>
namespace Gdiplus
{
	using std::min;
	using std::max;
}
#include <gdiplus.h>
#include <sstream>
#include <cassert>

#pragma comment(lib, "gdiplus.lib")

Surface::Surface(unsigned int width, unsigned int height, unsigned int pitch) noexcept
	:
	pBuffer(std::make_unique<Color[]>((size_t)pitch * height)),
	width(width),
	height(height)
{}

Surface::Surface(unsigned int width, unsigned int height) noexcept
	:
	Surface(width, height, width)
{}

Surface::Surface(unsigned int width, unsigned int height, std::unique_ptr<Color[]> pBuffer) noexcept
	:	
	pBuffer(std::move(pBuffer)),
	width(width),
	height(height)
{}

Surface::Surface(Surface&& source) noexcept
	:
	pBuffer(std::move(source.pBuffer)),
	width(source.width),
	height(source.height)
{}

Surface& Surface::operator=(Surface&& donor) noexcept
{
	width = donor.width;
	height = donor.height;
	pBuffer = std::move(donor.pBuffer);
	donor.pBuffer = nullptr;
	return *this;
}

void Surface::Clear(Color fillvalue) noexcept
{
	Color* pStart = pBuffer.get();
	Color* pEnd = &pBuffer[(size_t)width * height];

	for (Color* c = pStart; c < pEnd; c++)
	{
		*c = fillvalue;
	}
}

void Surface::PutPixel(int x, int y, Color c) noexcept
{
	assert(x >= 0 && "Attempting to draw outside the surface");
	assert(x < width && "Attempting to draw outside the surface");
	assert(y >= 0 && "Attempting to draw outside the surface");
	assert(y < height && "Attempting to draw outside the surface");
	pBuffer[x + (size_t)width * y] = c;
}

Color Surface::GetPixel(unsigned int x, unsigned int y) const noexcept
{
	assert(x >= 0u && "Attempting sample outside the surface");
	assert(x < width && "Attempting sample outside the surface");
	assert(y >= 0u && "Attempting sample outside the surface");
	assert(y < height && "Attempting sample outside the surface");
	return pBuffer[x + (size_t)width * y];
}

Color Surface::Sample(float u, float v) const noexcept
{
	const unsigned int x = std::clamp((unsigned int)(u * float(width - 1u)) , 0u, width  - 1u);
	const unsigned int y = std::clamp((unsigned int)(v * float(height - 1u)), 0u, height - 1u);
	return GetPixel(x, y);
}

unsigned int Surface::GetWidth() const noexcept
{
	return width;
}

unsigned int Surface::GetHeight() const noexcept
{
	return height;
}

Color* Surface::GetBufferPtr() const noexcept
{
	return pBuffer.get();
}

const Color* Surface::GetBufferPtrConst() const noexcept
{
	return pBuffer.get();
}

unsigned int Surface::GetRowPitch() const noexcept
{
	return width * sizeof(Color);
}

unsigned int Surface::GetBufferSize() const noexcept
{
	return width * height * sizeof(Color);
}

unsigned int Surface::GetPixelCount() const noexcept
{
	return width * height;
}

Surface Surface::FromFile(const std::string& filename)
{
	// Increase the reference count on GDIPlus cause you need it 
	// (will be decreased when we go out of scope)
	GDIPlusManager gdipm;
	// Convert the filename to wide characters
	std::wstring wfilename = std::wstring(filename.begin(), filename.end());

	// Load the bitmap from the provided filename
	Gdiplus::Bitmap bitmap(wfilename.c_str());
	
	// Check for errors
	const auto gdiStatus = bitmap.GetLastStatus();
	if (gdiStatus == Gdiplus::Status::GdiplusNotInitialized)
	{
		std::stringstream ss;
		ss << "Loading image [" << filename << "]: Gdiplus was not initialized. Make sure to initialize it instanciating a GDIPlusManager object at the beginning of the program.";
		throw Exception(__LINE__, __FILE__, ss.str());
	}
	if (gdiStatus != Gdiplus::Status::Ok)
	{
		std::stringstream ss;
		ss << "Loading image [" << filename  << "]: failed to load.";
		throw Exception(__LINE__, __FILE__, ss.str());
	}

	// We loaded the image into the Bitmap object, now let's get the data!
	// First we need the width and the height
	const unsigned int width  = bitmap.GetWidth();
	const unsigned int height = bitmap.GetHeight();

	// We prepare the buffer of colors with the right size
	auto pBuffer = std::make_unique<Color[]>((size_t)width * height);

	// Now look through every pixel in the loaded image and copy it to our pBuffer
	for (unsigned int y = 0u; y < height; y++)
	{
		for (unsigned int x = 0u; x < width; x++)
		{
			Gdiplus::Color pixel;
			bitmap.GetPixel((INT)x, (INT)y, &pixel);
			pBuffer[x + (size_t)width * y] = pixel.GetValue();
		}
	}

	return Surface(width, height, std::move(pBuffer));
}

void Surface::Save(const std::string& filename) const
{
	GDIPlusManager gdipm;

	// Not so easy stuff.
	auto GetEncoderClsid = [&filename](const WCHAR* format, CLSID* pClsid) -> void
	{
		UINT  num = 0;          // Number of image encoders
		UINT  size = 0;         // Size of the image encoder array in bytes

		Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;

		Gdiplus::GetImageEncodersSize(&num, &size);
		if (size == 0)
		{
			std::stringstream ss;
			ss << "Saving surface to [" << filename << "]: failed to get encoder; size == 0.";
			throw Exception(__LINE__, __FILE__, ss.str());
		}

		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == nullptr)
		{
			std::stringstream ss;
			ss << "Saving surface to [" << filename << "]: failed to get encoder; failed to allocate memory.";
			throw Exception(__LINE__, __FILE__, ss.str());
		}

		GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return;
			}
		}

		free(pImageCodecInfo);
		std::stringstream ss;
		ss << "Saving surface to [" << filename <<
			"]: failed to get encoder; failed to find matching encoder.";
		throw Exception(__LINE__, __FILE__, ss.str());
	};

	CLSID bmpID;
	GetEncoderClsid(L"image/bmp", &bmpID);

	// Convert filename to wide string (for Gdiplus)
	std::wstring wfilename(filename.begin(), filename.end());

	Gdiplus::Bitmap bitmap(width, height, width * sizeof(Color), PixelFormat32bppARGB, (BYTE*)pBuffer.get());
	if (bitmap.Save(wfilename.c_str(), &bmpID, nullptr) != Gdiplus::Status::Ok)
	{
		std::stringstream ss;
		ss << "Saving surface to [" << filename << "]: failed to save.";
		throw Exception(__LINE__, __FILE__, ss.str());
	}
}

void Surface::Copy(const Surface& src) noexcept
{
	assert(width == src.width);
	assert(height == src.height);
	memcpy(pBuffer.get(), src.pBuffer.get(), (size_t)width * height * sizeof(Color));
}

/*************************************************************************************/
/************************ GDIPlus Initialization Manager *****************************/
unsigned long long Surface::GDIPlusManager::token = 0;
int Surface::GDIPlusManager::refCount = 0;

Surface::GDIPlusManager::GDIPlusManager()
{
	if (refCount++ == 0)
	{
		Gdiplus::GdiplusStartupInput input;
		input.GdiplusVersion = 1u;
		input.DebugEventCallback = nullptr;
		input.SuppressBackgroundThread = false;
		Gdiplus::GdiplusStartup(reinterpret_cast<ULONG_PTR*>(&token), &input, nullptr);
	}
}

Surface::GDIPlusManager::~GDIPlusManager()
{
	if (--refCount == 0)
	{
		Gdiplus::GdiplusShutdown(token);
	}
}

/***************************************************************************************/
/********************************** EXCEPTION LAND *************************************/
Surface::Exception::Exception(int line, const char* file, std::string note) noexcept
	:
	TeslaException(line, file),
	note(std::move(note))
{
}

const char* Surface::Exception::what() const noexcept
{
	std::ostringstream oss;
	oss << TeslaException::what() << std::endl
		<< "[Note] " << GetNote();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Surface::Exception::GetType() const noexcept
{
	return "Tesla Surface Exception!";
}

const std::string& Surface::Exception::GetNote() const noexcept
{
	return note;
}