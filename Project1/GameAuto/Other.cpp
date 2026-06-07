#ifdef USE_OTHER
#include "Other.h"

const DWORD FPS = (DWORD)60.0f;
const DWORD SEC = (DWORD)1000.0f;
const DWORD WAIT_MILL_SEC = SEC / FPS;
const DWORD NEXT_WAIT = WAIT_MILL_SEC * 1;
const DWORD IDLE_WAIT = 16;
const unsigned int c_end_key = VK_2;

Other::Other(HWND hwnd)
	:m_hwnd(hwnd)
	, m_mouse()
{
	m_input_data_screen_size.x = 2560;
	m_input_data_screen_size.y = 1440;
	UpdateRectInfo();
}
Other::~Other()
{
}

void Other::Main()
{
	
	CopyCursor(GetCursor());
	while (true)
	{
		const unsigned int c_end_key = VK_2;
		m_keyboard.Update();
		if (m_keyboard.IsKeyInput(VK_1))
		{
			while (true)
			{
				m_keyboard.Update();
				Sleep(NEXT_WAIT * 10);
				m_keyboard.KeyInput(VK_Z);
				//m_mouse.LBClick();
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

		if (m_keyboard.IsKeyInput(VK_Q))
		{
			POINT click_point;
			click_point.x = 2495;//2495, 1585
			click_point.y = 1585;
			ShowCursor(FALSE);
			m_mouse.SavePos();

			float rate = 1.0f / 1.5f;
			POINT move_after_pos;
			move_after_pos.x = static_cast<int>(click_point.x * rate);
			move_after_pos.y = static_cast<int>(click_point.y * rate);
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

		if (m_keyboard.IsKeyInput(c_end_key))
		{
			return;
		}

		Sleep(IDLE_WAIT);
	}
}

void Other::MouseLBClick(POINT click_point)
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

void Other::UpdateRectInfo()
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
