#include "GraphicScreenAccessor.h"

#include <cstddef>

bool GraphicScreenAccessor::FindWindowByTitle(const std::wstring& window_title, HWND* hwnd) const
{
	*hwnd = FindWindowW(nullptr, window_title.c_str());
	return *hwnd != nullptr;
}

bool GraphicScreenAccessor::CaptureClient(HWND hwnd, GraphicImage* image) const
{
	RECT rect = {};
	GetClientRect(hwnd, &rect);
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;
	if (width <= 0 || height <= 0)
	{
		return false;
	}

	POINT screen_pos = { 0, 0 };
	ClientToScreen(hwnd, &screen_pos);

	HDC screen_dc = GetDC(nullptr);
	HDC memory_dc = CreateCompatibleDC(screen_dc);
	HBITMAP bitmap = CreateCompatibleBitmap(screen_dc, width, height);
	HGDIOBJ old = SelectObject(memory_dc, bitmap);
	BitBlt(memory_dc, 0, 0, width, height, screen_dc, screen_pos.x, screen_pos.y, SRCCOPY);

	image->width = width;
	image->height = height;
	image->bgra.assign(static_cast<size_t>(width) * height * 4, 0);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	const int rows = GetDIBits(memory_dc, bitmap, 0, height, image->bgra.data(), &bmi, DIB_RGB_COLORS);

	SelectObject(memory_dc, old);
	DeleteObject(bitmap);
	DeleteDC(memory_dc);
	ReleaseDC(nullptr, screen_dc);

	return rows == height;
}
