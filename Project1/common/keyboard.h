#pragma once
#include "Me_include.h"


class KeyBoard
{
private:
	static const int MAX = 512;
	bool m_prev_down[MAX];

	inline bool IsValidKey(int key) const
	{
		return key >= 0 && key < MAX;
	}

	inline void SendKeyInput(int key, bool key_up)
	{
		WORD scan_code = static_cast<WORD>(MapVirtualKey(key, MAPVK_VK_TO_VSC));
		INPUT input = {};
		input.type = INPUT_KEYBOARD;
		input.ki.wScan = scan_code;
		input.ki.dwFlags = KEYEVENTF_SCANCODE;
		if (key_up)
		{
			input.ki.dwFlags |= KEYEVENTF_KEYUP;
		}
		SendInput(1, &input, sizeof(INPUT));
	}

public:
	KeyBoard()
	{
		init();
	}

	inline void Update()
	{
	}

	inline void init()
	{
		for (int ii = 0; ii < MAX; ii++)
		{
			m_prev_down[ii] = false;
		}
	}

	inline bool IsKeyInput(int key)
	{
		if (!IsValidKey(key))
		{
			return false;
		}

		const SHORT state = GetAsyncKeyState(key);
		const bool curr = (state & 0x8000) != 0;
		const bool prev = m_prev_down[key];

		if (curr && !prev)
		{
			m_prev_down[key] = true;
			return true;
		}

		if (!curr)
		{
			m_prev_down[key] = false;
		}

		return false;
	}

	inline void KeyInput(int key)
	{
		if (!IsValidKey(key))
		{
			return;
		}

		m_prev_down[key] = true;
		SendKeyInput(key, false);
		SendKeyInput(key, true);
	}

	inline void KeyInput(int key, bool is_up)
	{
		if (!IsValidKey(key))
		{
			return;
		}

		m_prev_down[key] = !is_up;
		SendKeyInput(key, is_up);
	}

	inline void KeyInput(int key, HWND hwnd)
	{
		if (!IsValidKey(key))
		{
			return;
		}

		m_prev_down[key] = true;
		KeyInput(key);
	}
};
