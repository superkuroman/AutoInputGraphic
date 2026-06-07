#include"mouse.h"

MOUSE::MOUSE()
	:m_record_point()
	,m_is_lock(false)
{
	m_cursor = GetCursor();
}

//移動滑鼠到絕對位置（X座標，Y座標）
 void MOUSE::Move(int x, int y)
{
	SetCursorPos(x, y);
}
 void MOUSE::MoveLock(int x, int y)
 {
	m_is_lock = true;
	RECT    rt;
	rt.left = rt.right = x;
	rt.top = rt.bottom = y;
	ClipCursor(NULL);
	ClipCursor(&rt);
	SetCursorPos(x, y);
 }

//移動滑鼠到相對位置（X位移，Y位移）
 void MOUSE::RelativeMove(int cx, int cy)
{
	POINT point;
	GetCursorPos(&point);
	point.x += cx;
	point.y += cy;
	SetCursorPos(point.x, point.y);
}
//移動滑鼠到相對位置（X位移，Y位移）
 void MOUSE::RelativeMove(int cx, int cy, HWND hwnd)
{
	POINT point;
	GetCursorPos(&point);
	point.x += cx;
	point.y += cy;
	SetCursorPos(point.x, point.y);
	SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(point.x, point.y));
}

//儲存當前位置（）
 void MOUSE::SavePos()
{
	GetCursorPos(&this->m_record_point);
}

//恢復滑鼠位置（）
 void MOUSE::RestorePos()
{
	if (m_is_lock)
	{
		MoveLock(this->m_record_point.x, this->m_record_point.y);
	}
	else
	{
		SetCursorPos(this->m_record_point.x, this->m_record_point.y);
	}
	
}

bool MOUSE::IsMouseMoved() const
{
	POINT current_point;
	GetCursorPos(&current_point);
	if (current_point.x != m_record_point.x || current_point.y != m_record_point.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//鎖定滑鼠（）
 void MOUSE::Lock()
{
	m_is_lock = true;
	POINT    pt;
	RECT    rt;

	GetCursorPos(&pt);
	rt.left = rt.right = pt.x;
	rt.top = rt.bottom = pt.y;
	rt.right++;
	rt.bottom++;
	ClipCursor(&rt);
}

//解鎖滑鼠（）
 void MOUSE::Unlock()
{
	m_is_lock = false;
	ClipCursor(NULL);
}

//左鍵單擊（）
 void MOUSE::LBClick()
{
	INPUT inputs[2] = {};
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(2, inputs, sizeof(INPUT));
}
//左鍵單擊（）
 void MOUSE::LBClick(HWND hwnd)
{
	SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
	SendMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
}

//左鍵雙擊（）
 void MOUSE::LBDbClick()
{
	LBClick();
	LBClick();
}

//左鍵按下（）
void MOUSE::LBDown()
{
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
}
//左鍵按下（）
void MOUSE::LBDown(HWND hwnd)
{
	SendMessage(hwnd, MOUSEEVENTF_LEFTDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
}

//左鍵擡起（）
void MOUSE::LBUp()
{
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}
//左鍵擡起（）
void MOUSE::LBUp(HWND hwnd)
{
	SendMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
}

//右鍵單擊（）
void MOUSE::RBClick()
{
	INPUT inputs[2] = {};
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	SendInput(2, inputs, sizeof(INPUT));
}

//右鍵雙擊（）
void MOUSE::RBDbClick()
{
	RBClick();
	RBClick();
}

//右鍵按下（）
 void MOUSE::RBDown()
{
	mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
}

//右鍵擡起（）
 void MOUSE::RBUp()
{
	mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}

//中鍵單擊（）
 void MOUSE::MBClick()
{
	INPUT inputs[2] = {};
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
	SendInput(2, inputs, sizeof(INPUT));
}