#ifdef USE_ARCHE_LAND

#include "InArcheLand.h"

const DWORD FPS = (DWORD)60.0f;
const DWORD SEC = (DWORD)1000.0f;
const DWORD WAIT_MILL_SEC = SEC / FPS;
const DWORD NEXT_WAIT = WAIT_MILL_SEC * 1;
const DWORD IDLE_WAIT = 16;
const unsigned int c_end_key = VK_2;

InArcheLand::InArcheLand(HWND hwnd)
	:m_hwnd(hwnd)
	, m_mouse()
{
	m_input_data_screen_size.x = 2560;
	m_input_data_screen_size.y = 1440;
	UpdateRectInfo();
}
InArcheLand::~InArcheLand()
{
}

void InArcheLand::Main()
{
	CopyCursor(GetCursor());
	while (true)
	{
		m_keyboard.Update();
		if (m_keyboard.IsKeyInput(VK_1))
		{
			while (true)
			{
				m_keyboard.Update();
				m_mouse.LBClick();
				Sleep(NEXT_WAIT * 10);
				if (m_keyboard.IsKeyInput(VK_1))
				{
					break;
				}
				if (m_keyboard.IsKeyInput(c_end_key))
				{
					return;
				}
			}
		}

		// 1スキル
		if (m_keyboard.IsKeyInput(VK_Q))
		{
			POINT clikc_point;
			clikc_point.x = 2105;
			clikc_point.y = 1301;
			MouseLBClick(clikc_point);
		}

		// 2スキル
		if (m_keyboard.IsKeyInput(VK_W))
		{
			POINT clikc_point;
			clikc_point.x = 2165;
			clikc_point.y = 1043;
			MouseLBClick(clikc_point);
		}

		// 3スキル
		if (m_keyboard.IsKeyInput(VK_E))
		{
			POINT clikc_point;
			clikc_point.x = 2431;
			clikc_point.y = 943;
			MouseLBClick(clikc_point);
		}

		// 決定
		if (m_keyboard.IsKeyInput(VK_SPACE))
		{
			POINT clikc_point;
			clikc_point.x = 2397;
			clikc_point.y = 1236;
			MouseLBClick(clikc_point);
		}
		// 取り消し
		if (m_keyboard.IsKeyInput(VK_R))
		{
			POINT clikc_point;
			clikc_point.x = 2103;
			clikc_point.y = 1231;
			MouseLBClick(clikc_point);
		}

		if (m_keyboard.IsKeyInput(c_end_key))
		{
			return;
		}

		Sleep(IDLE_WAIT);
	}
}

void InArcheLand::MouseLBClick(POINT click_point)
{
	ShowCursor(FALSE);
	UpdateRectInfo();
	m_mouse.SavePos();

	POINT move_after_pos;
	move_after_pos.x = m_rect.left + static_cast<int>(m_rate_x * click_point.x);
	move_after_pos.y = m_rect.top + static_cast<int>(m_rate_y * click_point.y);
	m_mouse.MoveLock(
		move_after_pos.x,
		move_after_pos.y
	);
	Sleep(NEXT_WAIT);
	m_mouse.LBDown();
	Sleep(NEXT_WAIT);
	m_mouse.LBUp();
	Sleep(NEXT_WAIT);
	m_mouse.RestorePos();
	Sleep(NEXT_WAIT);
	m_mouse.Unlock();
	ShowCursor(TRUE);
}

void InArcheLand::UpdateRectInfo()
{
	GetWindowRect(m_hwnd, &m_rect);
	double size_x = 
		static_cast<double>(m_rect.right) - static_cast<double>(m_rect.left);
	double size_y =
		static_cast<double>(m_rect.bottom) - static_cast<double>(m_rect.top);
	m_rate_x = size_x / m_input_data_screen_size.x;
	m_rate_y = size_y / m_input_data_screen_size.y;
}

#endif