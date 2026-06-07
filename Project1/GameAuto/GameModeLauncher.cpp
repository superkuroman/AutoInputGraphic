#include "GameModeLauncher.h"

#include <cstddef>
#include <cstdio>
#include <windows.h>

namespace
{
struct GameMode
{
	const wchar_t* name;
	const wchar_t* const* window_titles;
	std::size_t window_title_count;
	void (*run)(HWND hwnd);
};

HWND FindModeWindow(const GameMode& mode)
{
	for (std::size_t index = 0; index < mode.window_title_count; ++index)
	{
		HWND hwnd = FindWindowW(nullptr, mode.window_titles[index]);
		if (hwnd != nullptr)
		{
			return hwnd;
		}
	}

	return nullptr;
}

bool TryRunMode(const GameMode& mode)
{
	HWND hwnd = FindModeWindow(mode);
	if (hwnd == nullptr)
	{
		return false;
	}

	wprintf(L"Auto mode: %s\n", mode.name);
	mode.run(hwnd);
	return true;
}
}

bool RunFirstAvailableGameMode()
{
	return false;
}
