#include "InputStep.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iterator>

namespace
{
int ClampInt(int value, int min_value, int max_value)
{
	if (value < min_value)
	{
		return min_value;
	}
	if (value > max_value)
	{
		return max_value;
	}
	return value;
}

bool IsValidScreenInfo(const ScreenInfo& screen)
{
	return screen.width > 0 && screen.height > 0;
}

bool TryReadIntField(const std::string& content, size_t begin, size_t end, const char* field_name, int* value)
{
	const std::string key = std::string("\"") + field_name + "\"";
	const size_t key_pos = content.find(key, begin);
	if (key_pos == std::string::npos || key_pos > end)
	{
		return false;
	}

	const size_t colon = content.find(':', key_pos + key.size());
	if (colon == std::string::npos || colon > end)
	{
		return false;
	}

	size_t value_end = content.find_first_of(",}\n", colon + 1);
	if (value_end == std::string::npos || value_end > end)
	{
		value_end = end;
	}

	*value = atoi(content.substr(colon + 1, value_end - colon - 1).c_str());
	return true;
}

bool TryReadDoubleField(const std::string& content, size_t begin, size_t end, const char* field_name, double* value)
{
	const std::string key = std::string("\"") + field_name + "\"";
	const size_t key_pos = content.find(key, begin);
	if (key_pos == std::string::npos || key_pos > end)
	{
		return false;
	}

	const size_t colon = content.find(':', key_pos + key.size());
	if (colon == std::string::npos || colon > end)
	{
		return false;
	}

	size_t value_end = content.find_first_of(",}\n", colon + 1);
	if (value_end == std::string::npos || value_end > end)
	{
		value_end = end;
	}

	*value = strtod(content.substr(colon + 1, value_end - colon - 1).c_str(), nullptr);
	return true;
}

bool TryReadStringField(const std::string& content, size_t begin, size_t end, const char* field_name, std::string* value)
{
	const std::string key = std::string("\"") + field_name + "\"";
	const size_t key_pos = content.find(key, begin);
	if (key_pos == std::string::npos || key_pos > end)
	{
		return false;
	}

	const size_t colon = content.find(':', key_pos + key.size());
	if (colon == std::string::npos || colon > end)
	{
		return false;
	}

	const size_t quote_begin = content.find('"', colon + 1);
	if (quote_begin == std::string::npos || quote_begin > end)
	{
		return false;
	}

	const size_t quote_end = content.find('"', quote_begin + 1);
	if (quote_end == std::string::npos || quote_end > end)
	{
		return false;
	}

	*value = content.substr(quote_begin + 1, quote_end - quote_begin - 1);
	return true;
}
}

InputStep::InputStep(const char* outFile)
	: recording_(false),
	  records_(),
	  recording_screen_{0, 0, 0, 0},
	  lb_was_down_(false),
	  rb_was_down_(false),
	  mb_was_down_(false),
	  last_event_time_(std::chrono::steady_clock::now()),
	  y_was_down_(false),
	  p_was_down_(false),
	  f12_was_down_(false),
	  output_file_(outFile)
{
}

const char* InputStep::ToString(MouseButton button) const
{
	switch (button)
	{
	case MouseButton::Left:
		return "left";
	case MouseButton::Right:
		return "right";
	case MouseButton::Middle:
		return "middle";
	default:
		return "unknown";
	}
}

bool InputStep::TryParseButton(const std::string& value, MouseButton* button) const
{
	if (value == "left")
	{
		*button = MouseButton::Left;
		return true;
	}
	if (value == "right")
	{
		*button = MouseButton::Right;
		return true;
	}
	if (value == "middle")
	{
		*button = MouseButton::Middle;
		return true;
	}

	return false;
}

void InputStep::SaveToFile(const char* filename)
{
	char fullpath[MAX_PATH] = {0};
	DWORD res = GetFullPathNameA(filename, MAX_PATH, fullpath, nullptr);
	if (res == 0)
	{
		printf("Failed to resolve full path for '%s'\n", filename);
		// fall back to given filename
		strcpy_s(fullpath, MAX_PATH, filename);
	}
	else
	{
		printf("Saving to full path: %s\n", fullpath);
	}

	std::ofstream ofs(fullpath);
	if (!ofs)
	{
		printf("Failed to open %s for writing\n", fullpath);
		return;
	}
	const ScreenInfo screen = IsValidScreenInfo(recording_screen_) ? recording_screen_ : GetCurrentScreenInfo();
	ofs << std::fixed << std::setprecision(8);
	ofs << "{\n";
	ofs << "  \"coordinate_mode\": \"virtual_screen_ratio\",\n";
	ofs << "  \"screen\": { \"left\": " << screen.left << ", \"top\": " << screen.top
		<< ", \"width\": " << screen.width << ", \"height\": " << screen.height << " },\n";
	ofs << "  \"clicks\": [\n";
	for (size_t i = 0; i < records_.size(); ++i)
	{
		ofs << "    { \"x\": " << records_[i].x << ", \"y\": " << records_[i].y
			<< ", \"x_ratio\": " << records_[i].x_ratio << ", \"y_ratio\": " << records_[i].y_ratio
			<< ", \"button\": \"" << ToString(records_[i].button) << "\", \"delay_ms\": " << records_[i].delay_ms << " }";
		if (i + 1 < records_.size()) ofs << ",";
		ofs << "\n";
	}
	ofs << "  ]\n}\n";
	ofs.close();
	printf("Saved %zu clicks to %s\n", records_.size(), filename);
}

void InputStep::PlaybackFromFile(const char* filename)
{
	std::ifstream ifs(filename);
	if (!ifs)
	{
		printf("Failed to open %s for reading\n", filename);
		return;
	}
	std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();

	std::vector<ClickRecord> play;
	const size_t clicks_key = content.find("\"clicks\"");
	size_t pos = clicks_key == std::string::npos ? 0 : content.find('[', clicks_key);
	if (pos == std::string::npos)
	{
		pos = 0;
	}

	while (true)
	{
		size_t obj = content.find('{', pos);
		if (obj == std::string::npos) break;
		size_t obj_end = content.find('}', obj);
		if (obj_end == std::string::npos) break;

		int x = 0;
		int y = 0;
		int delay = 0;
		double x_ratio = 0.0;
		double y_ratio = 0.0;
		std::string button_value;
		MouseButton button;

		const bool has_position =
			TryReadIntField(content, obj, obj_end, "x", &x) &&
			TryReadIntField(content, obj, obj_end, "y", &y);
		const bool has_button =
			TryReadStringField(content, obj, obj_end, "button", &button_value) &&
			TryParseButton(button_value, &button);
		const bool has_delay = TryReadIntField(content, obj, obj_end, "delay_ms", &delay);
		const bool has_position_ratio =
			TryReadDoubleField(content, obj, obj_end, "x_ratio", &x_ratio) &&
			TryReadDoubleField(content, obj, obj_end, "y_ratio", &y_ratio);

		if (has_position && has_button && has_delay)
		{
			play.push_back({ x, y, button, delay, x_ratio, y_ratio, has_position_ratio });
		}

		pos = obj_end + 1;
	}

	if (play.empty())
	{
		printf("No playable clicks found in %s\n", filename);
		return;
	}

	printf("Playing back %zu clicks from %s\n", play.size(), filename);
	for (size_t i = 0; i < play.size(); ++i)
	{
		Sleep(play[i].delay_ms);
		const POINT point = ResolvePlaybackPoint(play[i]);
		SetCursorPos(point.x, point.y);
		SendClick(play[i].button);
		printf("click %zu x:%d y:%d ms:%d \n", i, point.x, point.y, play[i].delay_ms);
	}
	printf("Playback finished\n");
}

void InputStep::Update()
{
	HandleRecord();
	HandleSaveTrigger();
	HandlePlaybackTrigger();
}
void InputStep::HandleRecord()
{
	// Handle toggle (F12)
	bool f12_down = (GetAsyncKeyState(VK_F12) & 0x8000) != 0;
	if (f12_down && !f12_was_down_)
	{
		if (!recording_)
		{
			recording_ = true;
			records_.clear();
			recording_screen_ = GetCurrentScreenInfo();
			last_event_time_ = std::chrono::steady_clock::now();
			printf(
				"Recording started. screen left=%d top=%d width=%d height=%d. Press F12 again to stop.\n",
				recording_screen_.left,
				recording_screen_.top,
				recording_screen_.width,
				recording_screen_.height);
		}
		else
		{
			recording_ = false;
			SaveToFile(output_file_.c_str());
		}
	}
	f12_was_down_ = f12_down;

	// If recording, process mouse button down edges
	if (!recording_) return;

	bool lb_down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool rb_down = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	bool mb_down = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

	if (lb_down && !lb_was_down_)
	{
		RecordClick(MouseButton::Left);
	}
	if (rb_down && !rb_was_down_)
	{
		RecordClick(MouseButton::Right);
	}
	if (mb_down && !mb_was_down_)
	{
		RecordClick(MouseButton::Middle);
	}

	lb_was_down_ = lb_down;
	rb_was_down_ = rb_down;
	mb_was_down_ = mb_down;
}

void InputStep::HandleSaveTrigger()
{
	bool y_down = (GetAsyncKeyState('Y') & 0x8000) != 0;
	if (y_down && !y_was_down_)
	{
		SaveToFile(output_file_.c_str());
	}
	y_was_down_ = y_down;
}

void InputStep::HandlePlaybackTrigger()
{
	bool p_down = (GetAsyncKeyState('P') & 0x8000) != 0;
	if (p_down && !p_was_down_)
	{
		PlaybackFromFile(output_file_.c_str());
	}
	p_was_down_ = p_down;
}

void InputStep::RecordClick(MouseButton button)
{
	POINT point;
	GetCursorPos(&point);
	const ScreenInfo screen = IsValidScreenInfo(recording_screen_) ? recording_screen_ : GetCurrentScreenInfo();
	const bool has_position_ratio = IsValidScreenInfo(screen);
	const double x_ratio = has_position_ratio
		? static_cast<double>(point.x - screen.left) / static_cast<double>(screen.width)
		: 0.0;
	const double y_ratio = has_position_ratio
		? static_cast<double>(point.y - screen.top) / static_cast<double>(screen.height)
		: 0.0;

	const auto now = std::chrono::steady_clock::now();
	const int delay = static_cast<int>(
		std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event_time_).count());
	last_event_time_ = now;

	records_.push_back({ point.x, point.y, button, delay, x_ratio, y_ratio, has_position_ratio });
	printf(
		"Recorded click %zu -> %s x=%d y=%d x_ratio=%.8f y_ratio=%.8f delay=%d\n",
		records_.size(),
		ToString(button),
		point.x,
		point.y,
		x_ratio,
		y_ratio,
		delay);
}

POINT InputStep::ResolvePlaybackPoint(const ClickRecord& record) const
{
	if (!record.has_position_ratio)
	{
		return { record.x, record.y };
	}

	const ScreenInfo screen = GetCurrentScreenInfo();
	if (!IsValidScreenInfo(screen))
	{
		return { record.x, record.y };
	}

	const int max_x = screen.left + screen.width - 1;
	const int max_y = screen.top + screen.height - 1;
	const int x = screen.left + static_cast<int>(std::lround(record.x_ratio * screen.width));
	const int y = screen.top + static_cast<int>(std::lround(record.y_ratio * screen.height));
	return { ClampInt(x, screen.left, max_x), ClampInt(y, screen.top, max_y) };
}

ScreenInfo InputStep::GetCurrentScreenInfo() const
{
	ScreenInfo screen = {
		GetSystemMetrics(SM_XVIRTUALSCREEN),
		GetSystemMetrics(SM_YVIRTUALSCREEN),
		GetSystemMetrics(SM_CXVIRTUALSCREEN),
		GetSystemMetrics(SM_CYVIRTUALSCREEN),
	};

	if (!IsValidScreenInfo(screen))
	{
		screen.left = 0;
		screen.top = 0;
		screen.width = GetSystemMetrics(SM_CXSCREEN);
		screen.height = GetSystemMetrics(SM_CYSCREEN);
	}

	return screen;
}

void InputStep::SendClick(MouseButton button)
{
	DWORD down_flag = 0;
	DWORD up_flag = 0;
	switch (button)
	{
	case MouseButton::Left:
		down_flag = MOUSEEVENTF_LEFTDOWN;
		up_flag = MOUSEEVENTF_LEFTUP;
		break;
	case MouseButton::Right:
		down_flag = MOUSEEVENTF_RIGHTDOWN;
		up_flag = MOUSEEVENTF_RIGHTUP;
		break;
	case MouseButton::Middle:
		down_flag = MOUSEEVENTF_MIDDLEDOWN;
		up_flag = MOUSEEVENTF_MIDDLEUP;
		break;
	default:
		return;
	}

	INPUT inputs[2] = {};
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = down_flag;
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = up_flag;
	SendInput(2, inputs, sizeof(INPUT));
}
