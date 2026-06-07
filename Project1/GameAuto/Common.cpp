#include "Common.h"

const DWORD FPS = (DWORD)60.0f;
const DWORD SEC = (DWORD)1000.0f;
const DWORD WAIT_MILL_SEC = SEC / FPS;
const DWORD NEXT_WAIT = WAIT_MILL_SEC * 1;
const unsigned int c_end_key = VK_2;


Common::Common(HWND hwnd)
	: m_hwnd(hwnd)
	, m_mouse()
{
	m_input_data_screen_size.x = 2560;
	m_input_data_screen_size.y = 1440;
	UpdateRectInfo();
}
Common::~Common()
{
}

void Common::MouseLBClick(POINT click_point)
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

void Common::UpdateRectInfo()
{
	GetWindowRect(m_hwnd, &m_rect);
	double size_x = 
		static_cast<double>(m_rect.right) - static_cast<double>(m_rect.left);
	double size_y =
		static_cast<double>(m_rect.bottom) - static_cast<double>(m_rect.top);
	m_rate_x = size_x / m_input_data_screen_size.x;
	m_rate_y = size_y / m_input_data_screen_size.y;
}