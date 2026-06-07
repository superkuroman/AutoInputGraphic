#ifdef USE_TDJ

#include "TDJ.h"

#include <cstddef>

namespace
{
constexpr DWORD kFps = 60;
constexpr DWORD kSec = 1000;
constexpr DWORD kWaitMillSec = kSec / kFps;
constexpr DWORD kNextWait = kWaitMillSec;
constexpr DWORD kIdleWait = 16;
constexpr unsigned int kEndKey = VK_2;

constexpr DWORD Seconds(double seconds)
{
	return static_cast<DWORD>(seconds * 1000.0);
}

struct ClickAction
{
	int key;
	const TDJ::ClickStep* steps;
	std::size_t step_count;
};

template <std::size_t Count>
constexpr ClickAction MakeClickAction(int key, const TDJ::ClickStep (&steps)[Count])
{
	return { key, steps, Count };
}

const TDJ::ClickStep kSkill1Steps[] = {
	{ { 0.78750000, 0.88263889 }, Seconds(0.0) },
};

const TDJ::ClickStep kSkill2Steps[] = {
	{ { 0.81757813, 0.69166667 }, Seconds(0.0) },
};

const TDJ::ClickStep kSkill3Steps[] = {
	{ { 0.92382813, 0.62291667 }, Seconds(0.0) },
};

const TDJ::ClickStep kConfirmSteps[] = {
	{ { 0.93750000, 0.86736111 }, Seconds(0.0) },
};

const TDJ::ClickStep kCancelSteps[] = {
	{ { 0.72703125, 0.90416667 }, Seconds(0.0) },
};

const TDJ::ClickStep kZSteps[] = {
	{ { 0.81039198, 0.94327391 }, Seconds(0.5) },
	{ { 0.93299909, 0.91734198 }, Seconds(0.0) },
};

const ClickAction kClickActions[] = {
	MakeClickAction(VK_Q, kSkill1Steps),
	MakeClickAction(VK_W, kSkill2Steps),
	MakeClickAction(VK_E, kSkill3Steps),
	MakeClickAction(VK_SPACE, kConfirmSteps),
	MakeClickAction(VK_R, kCancelSteps),
	MakeClickAction(VK_Z, kZSteps)
};
}

TDJ::TDJ(HWND hwnd)
	: m_hwnd(hwnd)
	, m_rect{}
	, m_mouse()
{
	UpdateRectInfo();
}

TDJ::~TDJ()
{
}

void TDJ::Main()
{
	m_keyboard.Update();
	while (!m_keyboard.IsKeyInput(kEndKey))
	{
		m_keyboard.Update();
		if (m_keyboard.IsKeyInput(VK_1))
		{
			while (true)
			{
				m_keyboard.Update();
				m_mouse.LBClick();
				Sleep(kNextWait * 10);
				if (m_keyboard.IsKeyInput(VK_1))
				{
					break;
				}
				if (m_keyboard.IsKeyInput(kEndKey))
				{
					return;
				}
			}
		}
		if (m_keyboard.IsKeyInput(VK_3))
		{
			while (true)
			{
				m_keyboard.Update();
				m_mouse.LBClick();
				Sleep(kNextWait);
				if (m_keyboard.IsKeyInput(VK_3))
				{
					break;
				}
				if (m_keyboard.IsKeyInput(kEndKey))
				{
					return;
				}
			}
		}
		else
		{
			for (std::size_t index = 0; index < sizeof(kClickActions) / sizeof(kClickActions[0]); ++index)
			{
				if (m_keyboard.IsKeyInput(kClickActions[index].key))
				{
					MouseLBClickSequence(kClickActions[index].steps, kClickActions[index].step_count);
					break;
				}
			}
		}

		Sleep(kIdleWait);
	}
}

void TDJ::MouseLBClickSequence(const ClickStep* steps, std::size_t step_count)
{
	if (steps == nullptr || step_count == 0)
	{
		return;
	}

	ShowCursor(FALSE);
	UpdateRectInfo();
	m_mouse.SavePos();

	for (std::size_t index = 0; index < step_count; ++index)
	{
		const POINT move_after_pos = ToWindowPoint(steps[index].point);
		m_mouse.MoveLock(
			move_after_pos.x,
			move_after_pos.y
		);
		Sleep(kNextWait);
		m_mouse.LBDown();
		Sleep(kNextWait);
		m_mouse.LBUp();
		Sleep(kNextWait);

		if (index + 1 < step_count && steps[index].wait_after_ms > 0)
		{
			Sleep(steps[index].wait_after_ms);
		}
	}

	m_mouse.RestorePos();
	Sleep(kNextWait);
	m_mouse.Unlock();
	ShowCursor(TRUE);
}

void TDJ::UpdateRectInfo()
{
	GetWindowRect(m_hwnd, &m_rect);
}

POINT TDJ::ToWindowPoint(RatioPoint click_point) const
{
	const double size_x = static_cast<double>(m_rect.right - m_rect.left);
	const double size_y = static_cast<double>(m_rect.bottom - m_rect.top);
	return {
		m_rect.left + static_cast<int>(size_x * click_point.x),
		m_rect.top + static_cast<int>(size_y * click_point.y),
	};
}

#endif
