#pragma once
#include "TeslaException.h"
#include <string>
#include <memory>
#include "Color.h"

// Stores an image
class Surface
{
public:
	class Exception : public TeslaException
	{
	public:
		Exception(int line, const char* file, std::string note) noexcept;
		virtual const char* what() const noexcept override;
		virtual const char* GetType() const noexcept override;
		const std::string& GetNote() const noexcept;
	private:
		std::string note;
	};
public:
    class GDIPlusManager
    {
    public:
        GDIPlusManager();
        ~GDIPlusManager();
    private:
        static unsigned long long token;
        static int refCount;
    };
public:
    Surface() = delete;
	Surface(unsigned int width, unsigned int height, std::unique_ptr<Color[]> pBuffer) noexcept;
	Surface(unsigned int width, unsigned int height, unsigned int pitch) noexcept;
	Surface(unsigned int width, unsigned int height) noexcept;
	Surface(Surface&& source) noexcept;
	Surface(Surface&) = delete;
	Surface& operator = (Surface&& donor) noexcept;
	Surface& operator = (const Surface&) = delete;
    ~Surface() = default;
    // Clear the entire Surface with the specified color
	void Clear(Color fillvalue) noexcept;
    // Set the pixel at coordinates (x, y)
	void PutPixel(int x, int y, Color c) noexcept;
    // Get the pixel at coordinates (x, y)
	Color GetPixel(unsigned int x, unsigned int y) const noexcept;
	// Sample the texture using normalized uv coordinates
	Color Sample(float u, float v) const noexcept;
    // Get the Surface width (in pixels)
	unsigned int GetWidth() const noexcept;
    // Get the Surface height (in pixels)
	unsigned int GetHeight() const noexcept;
    // Get a pointer to the color buffer
	Color* GetBufferPtr() const noexcept;
    // Get a constant pointer to the color buffer
	const Color* GetBufferPtrConst() const noexcept;
	// Get the Row Pitch in bytes
	unsigned int GetRowPitch() const noexcept;
    // Get the number bytes in the Surface
    unsigned int GetBufferSize() const noexcept;
    // Get the number of Pixels in the Surface
    unsigned int GetPixelCount() const noexcept;
	// Load surface from an image file (bmp, png, jpg, etc.)
	static Surface FromFile(const std::string& filename);
    // Save the Surface to a file (only .bmp)
	void Save(const std::string& filename) const;
    // Copy from another Surface having the same size
	void Copy(const Surface& src) noexcept;
private:
	std::unique_ptr<Color[]> pBuffer;
	unsigned int width;
	unsigned int height;
};