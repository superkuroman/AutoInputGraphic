#include "GraphicImageReader.h"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace
{
inline std::wstring ToWide(const std::string& text)
{
	if (text.empty())
	{
		return std::wstring();
	}

	const int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
	if (length <= 0)
	{
		return std::wstring(text.begin(), text.end());
	}

	std::wstring wide(static_cast<size_t>(length - 1), L'\0');
	MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), length);
	return wide;
}

bool EnsureGdiPlusStarted()
{
	static ULONG_PTR token = 0;
	static bool attempted = false;
	static bool started = false;
	if (!attempted)
	{
		Gdiplus::GdiplusStartupInput input = {};
		started = Gdiplus::GdiplusStartup(&token, &input, nullptr) == Gdiplus::Ok;
		attempted = true;
	}

	return started;
}
}

bool GraphicImageReader::LoadImageFile(const std::string& path, GraphicImage* image) const
{
	if (!EnsureGdiPlusStarted())
	{
		printf("Failed to start GDI+ for image loading\n");
		return false;
	}

	Gdiplus::Bitmap bitmap(ToWide(path).c_str());
	if (bitmap.GetLastStatus() != Gdiplus::Ok)
	{
		printf("Failed to load template image: %s\n", path.c_str());
		return false;
	}

	const UINT width = bitmap.GetWidth();
	const UINT height = bitmap.GetHeight();
	if (width == 0 || height == 0)
	{
		return false;
	}

	Gdiplus::Rect rect(0, 0, static_cast<INT>(width), static_cast<INT>(height));
	Gdiplus::BitmapData data = {};
	if (bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok)
	{
		return false;
	}

	image->width = static_cast<int>(width);
	image->height = static_cast<int>(height);
	image->bgra.assign(static_cast<size_t>(image->width) * image->height * 4, 0);

	const unsigned char* src = static_cast<const unsigned char*>(data.Scan0);
	for (int y = 0; y < image->height; ++y)
	{
		const unsigned char* src_row = src + static_cast<ptrdiff_t>(y) * data.Stride;
		unsigned char* dst_row = image->bgra.data() + static_cast<size_t>(y) * image->width * 4;
		memcpy(dst_row, src_row, static_cast<size_t>(image->width) * 4);
	}

	bitmap.UnlockBits(&data);
	return true;
}
