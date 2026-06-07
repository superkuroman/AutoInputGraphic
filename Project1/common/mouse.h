////////////////////
//mouse.h
//模擬滑鼠的常見操作
////////////////////
#pragma once
#include"Me_include.h"

class MOUSE
{
private:
	//座標變數
	POINT m_record_point;
	HCURSOR m_cursor;
	bool m_is_lock;
public:
	MOUSE();
	//移動類函式
	void    Move(int x,int y);
	void    RelativeMove(int cx,int cy);
	void    RelativeMove(int cx,int cy,HWND hwnd);
	void    SavePos();
	void    RestorePos();
	//鎖定啟用類
	void    Lock();
	void    MoveLock(int x, int y);
	void    Unlock();
	//動作類
	void    LBClick();
	void    LBClick(HWND hwnd);
	void    LBDbClick();
	void    LBDown();
	void    LBDown(HWND hwnd);
	void    LBUp();
	void    LBUp(HWND hwnd);
	void    RBClick();
	void    RBDbClick();
	void    RBDown();
	void    RBUp();
	void    MBClick();

	bool IsMouseMoved() const;
};