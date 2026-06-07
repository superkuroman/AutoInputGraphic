#pragma once

#include "GraphicImageReader.h"

#include <string>
#include <windows.h>

class GraphicScreenAccessor
{
public:
	bool FindWindowByTitle(const std::wstring& window_title, HWND* hwnd) const;
	bool CaptureClient(HWND hwnd, GraphicImage* image) const;
};
