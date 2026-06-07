#pragma once
#include<fstream>
#include"..\common\mouse.h"
#include"..\common\keyboard.h"

class Common
{
public:
	inline void static CreateInstance(HWND hwnd)
	{
		m_instance = Common(hwnd);
	}
	inline Common static GetInstance()
	{
		return m_instance;
	}

	
private:
	Common(HWND hwnd);
	~Common();
	void MouseLBClick(POINT click_point);
	void UpdateRectInfo();

private:
	static Common m_instance;
	HWND m_hwnd;
	POINT m_input_data_screen_size; // 入力データを取る時のスクリーンサイズ
	RECT m_rect;
	double m_rate_x;
	double m_rate_y;
	KeyBoard m_keyboard;
	MOUSE m_mouse;
	
};

