#include <windows.h>

#include "common/keyboard.h"
#include "common/mouse.h"
#include "InputStep.h"
#include "GameAuto/GameModeLauncher.h"

namespace
{
constexpr DWORD kFps = 30;
constexpr DWORD kSec = 1000;
constexpr DWORD kWaitMillSec = kSec / kFps;
constexpr DWORD kNextWait = kWaitMillSec;
constexpr double kDefaultLoopRate = 0.3;
constexpr DWORD kDefaultWait = static_cast<DWORD>(kNextWait * kDefaultLoopRate);
constexpr DWORD kIdleWait = 16;
constexpr unsigned int kDefaultEndKey = VK_5;

KeyBoard g_keyboard;
MOUSE g_mouse;

bool ShouldSwitchDefaultLoop()
{
	return
		g_keyboard.IsKeyInput(VK_1) ||
		g_keyboard.IsKeyInput(VK_3) ||
		g_keyboard.IsKeyInput(VK_4);
}

bool ShouldStopDefaultMode()
{
	return g_keyboard.IsKeyInput(kDefaultEndKey);
}

bool RunMouseClickLoop()
{
	while (true)
	{
		g_keyboard.Update();
		Sleep(kDefaultWait);
		g_mouse.LBDown();
		Sleep(kDefaultWait);
		g_mouse.LBUp();
		Sleep(kDefaultWait);

		if (ShouldSwitchDefaultLoop())
		{
			return false;
		}
		if (ShouldStopDefaultMode())
		{
			return true;
		}
	}
}

bool RunKeyInputLoop(int key)
{
	while (true)
	{
		g_keyboard.Update();
		Sleep(kDefaultWait);
		g_keyboard.KeyInput(key);
		Sleep(kDefaultWait);

		if (ShouldSwitchDefaultLoop())
		{
			return false;
		}
		if (ShouldStopDefaultMode())
		{
			return true;
		}
	}
}

void RunDefaultAutoInput()
{
	while (true)
	{
		g_keyboard.Update();
		if (g_keyboard.IsKeyInput(VK_1) || g_keyboard.IsKeyInput(VK_2))
		{
			if (RunMouseClickLoop())
			{
				return;
			}
		}
		else if (g_keyboard.IsKeyInput(VK_3) || g_keyboard.IsKeyInput(VK_4))
		{
			if (RunKeyInputLoop(VK_Z))
			{
				return;
			}
		}
		else if (ShouldStopDefaultMode())
		{
			return;
		}

		Sleep(kIdleWait);
	}
}
}

int main()
{
	InputStep input("TargetClicks.json");
	while (true)
	{
		input.Update();
		Sleep(kIdleWait);
	}
	return 0;
}
