#ifdef USE_OTHER
#pragma once
#include<fstream>
#include"..\common\mouse.h"
#include"..\common\keyboard.h"

class Other
{
public:
	Other(HWND hwnd);
	~Other();

	void Main();
	
private:
	void MouseLBClick(POINT click_point);
	void UpdateRectInfo();

private:
	HWND m_hwnd;
	POINT m_input_data_screen_size; // 入力データを取る時のスクリーンサイズ
	RECT m_rect;
	double m_rate_x;
	double m_rate_y;
	KeyBoard m_keyboard;
	MOUSE m_mouse;
	
};

#endif