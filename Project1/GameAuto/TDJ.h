#ifdef USE_TDJ

#pragma once
#include <cstddef>
#include "..\common\mouse.h"
#include "..\common\keyboard.h"

class TDJ
{
public:
	struct RatioPoint
	{
		double x;
		double y;
	};

	struct ClickStep
	{
		RatioPoint point;
		DWORD wait_after_ms;
	};

	TDJ(HWND hwnd);
	~TDJ();

	void Main();

private:
	void MouseLBClickSequence(const ClickStep* steps, std::size_t step_count);
	void UpdateRectInfo();
	POINT ToWindowPoint(RatioPoint click_point) const;

private:
	HWND m_hwnd;
	RECT m_rect;
	KeyBoard m_keyboard;
	MOUSE m_mouse;
};

#endif
