#include "GraphicTrigger.h"

#include "../InputStep.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>

namespace
{
constexpr DWORD kScanIntervalMs = 500;
constexpr double kDefaultThreshold = 0.04;
constexpr DWORD kDefaultCooldownMs = 5000;

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

double ClampDouble(double value, double min_value, double max_value)
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

std::wstring ToWide(const std::string& text)
{
	if (text.empty())
	{
		return std::wstring();
	}

	const int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
	if (length <= 0)
	{
		return std::wstring(text.begin(), text.end());
	}

	std::wstring wide(static_cast<size_t>(length - 1), L'\0');
	MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), length);
	return wide;
}
}

GraphicTrigger::GraphicTrigger(const char* config_file)
	: config_file_(config_file),
	  window_title_(),
	  rules_(),
	  loaded_(false),
	  last_scan_(std::chrono::steady_clock::now())
{
}

void GraphicTrigger::Update(InputStep& player)
{
	if (!loaded_)
	{
		loaded_ = true;
		if (!LoadConfig())
		{
			printf("Graphic trigger disabled. Create %s to enable image detection.\n", config_file_.c_str());
			return;
		}
		printf("Graphic trigger loaded: %zu rule(s)\n", rules_.size());
	}

	const auto now = std::chrono::steady_clock::now();
	const DWORD elapsed_ms = static_cast<DWORD>(
		std::chrono::duration_cast<std::chrono::milliseconds>(now - last_scan_).count());
	if (elapsed_ms < kScanIntervalMs)
	{
		return;
	}
	last_scan_ = now;

	HWND hwnd = nullptr;
	if (!TryFindWindow(&hwnd))
	{
		return;
	}

	Image screen = {};
	if (!CaptureWindow(hwnd, &screen))
	{
		return;
	}

	for (size_t index = 0; index < rules_.size(); ++index)
	{
		GraphicTriggerRule& rule = rules_[index];
		if (!IsReady(rule))
		{
			continue;
		}

		Image templ = {};
		if (!LoadBmp(rule.template_bmp, &templ))
		{
			continue;
		}

		if (FindTemplate(screen, templ, rule.threshold))
		{
			printf("Graphic trigger matched: %s -> %s\n", rule.name.c_str(), rule.script_file.c_str());
			MarkRun(&rule);
			player.PlaybackFromFile(rule.script_file.c_str());
			break;
		}
	}
}

bool GraphicTrigger::LoadConfig()
{
	const std::string text = ReadTextFile(config_file_.c_str());
	if (text.empty())
	{
		return false;
	}

	std::string window_title;
	if (!ReadStringField(text, 0, text.size(), "window_title", &window_title))
	{
		printf("%s missing window_title\n", config_file_.c_str());
		return false;
	}
	window_title_ = ToWide(window_title);

	const size_t triggers_key = text.find("\"triggers\"");
	size_t pos = triggers_key == std::string::npos ? 0 : text.find('[', triggers_key);
	if (pos == std::string::npos)
	{
		return false;
	}

	while (true)
	{
		const size_t obj = text.find('{', pos);
		if (obj == std::string::npos)
		{
			break;
		}
		const size_t obj_end = text.find('}', obj);
		if (obj_end == std::string::npos)
		{
			break;
		}

		GraphicTriggerRule rule = {};
		rule.threshold = kDefaultThreshold;
		rule.cooldown_ms = kDefaultCooldownMs;
		rule.last_run = std::chrono::steady_clock::time_point();

		ReadStringField(text, obj, obj_end, "name", &rule.name);
		ReadDoubleField(text, obj, obj_end, "threshold", &rule.threshold);
		ReadDwordField(text, obj, obj_end, "cooldown_ms", &rule.cooldown_ms);

		if (ReadStringField(text, obj, obj_end, "template_bmp", &rule.template_bmp) &&
			ReadStringField(text, obj, obj_end, "script", &rule.script_file))
		{
			if (rule.name.empty())
			{
				rule.name = rule.template_bmp;
			}
			rule.threshold = ClampDouble(rule.threshold, 0.0, 1.0);
			rules_.push_back(rule);
		}

		pos = obj_end + 1;
	}

	return !rules_.empty();
}

bool GraphicTrigger::TryFindWindow(HWND* hwnd) const
{
	*hwnd = FindWindowW(nullptr, window_title_.c_str());
	return *hwnd != nullptr;
}

bool GraphicTrigger::LoadBmp(const std::string& path, Image* image) const
{
	HBITMAP bitmap = static_cast<HBITMAP>(LoadImageA(
		nullptr,
		path.c_str(),
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION));
	if (bitmap == nullptr)
	{
		printf("Failed to load template bmp: %s\n", path.c_str());
		return false;
	}

	BITMAP info = {};
	GetObject(bitmap, sizeof(info), &info);
	image->width = info.bmWidth;
	image->height = info.bmHeight;
	image->bgra.assign(static_cast<size_t>(image->width) * image->height * 4, 0);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = image->width;
	bmi.bmiHeader.biHeight = -image->height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(nullptr);
	const int rows = GetDIBits(hdc, bitmap, 0, image->height, image->bgra.data(), &bmi, DIB_RGB_COLORS);
	ReleaseDC(nullptr, hdc);
	DeleteObject(bitmap);

	return rows == image->height;
}

bool GraphicTrigger::CaptureWindow(HWND hwnd, Image* image) const
{
	RECT rect = {};
	GetClientRect(hwnd, &rect);
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;
	if (width <= 0 || height <= 0)
	{
		return false;
	}

	POINT screen_pos = { 0, 0 };
	ClientToScreen(hwnd, &screen_pos);

	HDC screen_dc = GetDC(nullptr);
	HDC memory_dc = CreateCompatibleDC(screen_dc);
	HBITMAP bitmap = CreateCompatibleBitmap(screen_dc, width, height);
	HGDIOBJ old = SelectObject(memory_dc, bitmap);
	BitBlt(memory_dc, 0, 0, width, height, screen_dc, screen_pos.x, screen_pos.y, SRCCOPY);

	image->width = width;
	image->height = height;
	image->bgra.assign(static_cast<size_t>(width) * height * 4, 0);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	const int rows = GetDIBits(memory_dc, bitmap, 0, height, image->bgra.data(), &bmi, DIB_RGB_COLORS);

	SelectObject(memory_dc, old);
	DeleteObject(bitmap);
	DeleteDC(memory_dc);
	ReleaseDC(nullptr, screen_dc);

	return rows == height;
}

bool GraphicTrigger::FindTemplate(const Image& screen, const Image& templ, double threshold) const
{
	if (screen.width < templ.width || screen.height < templ.height || templ.width <= 0 || templ.height <= 0)
	{
		return false;
	}

	const int shorter_edge = templ.width < templ.height ? templ.width : templ.height;
	const int step = ClampInt(shorter_edge / 12, 1, 8);
	for (int y = 0; y <= screen.height - templ.height; y += step)
	{
		for (int x = 0; x <= screen.width - templ.width; x += step)
		{
			if (CompareAt(screen, templ, x, y) <= threshold)
			{
				return true;
			}
		}
	}

	return false;
}

double GraphicTrigger::CompareAt(const Image& screen, const Image& templ, int left, int top) const
{
	unsigned long long total_diff = 0;
	const unsigned long long max_diff =
		static_cast<unsigned long long>(templ.width) * templ.height * 255ull * 3ull;

	for (int y = 0; y < templ.height; ++y)
	{
		const size_t screen_row = static_cast<size_t>(top + y) * screen.width * 4;
		const size_t templ_row = static_cast<size_t>(y) * templ.width * 4;
		for (int x = 0; x < templ.width; ++x)
		{
			const size_t screen_index = screen_row + static_cast<size_t>(left + x) * 4;
			const size_t templ_index = templ_row + static_cast<size_t>(x) * 4;
			total_diff += abs(static_cast<int>(screen.bgra[screen_index + 0]) - static_cast<int>(templ.bgra[templ_index + 0]));
			total_diff += abs(static_cast<int>(screen.bgra[screen_index + 1]) - static_cast<int>(templ.bgra[templ_index + 1]));
			total_diff += abs(static_cast<int>(screen.bgra[screen_index + 2]) - static_cast<int>(templ.bgra[templ_index + 2]));
		}
	}

	return max_diff == 0 ? 1.0 : static_cast<double>(total_diff) / static_cast<double>(max_diff);
}

bool GraphicTrigger::IsReady(const GraphicTriggerRule& rule) const
{
	if (rule.last_run == std::chrono::steady_clock::time_point())
	{
		return true;
	}

	const DWORD elapsed_ms = static_cast<DWORD>(
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - rule.last_run).count());
	return elapsed_ms >= rule.cooldown_ms;
}

void GraphicTrigger::MarkRun(GraphicTriggerRule* rule)
{
	rule->last_run = std::chrono::steady_clock::now();
}

std::string GraphicTrigger::ReadTextFile(const char* filename) const
{
	std::ifstream ifs(filename);
	if (!ifs)
	{
		return std::string();
	}
	return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

bool GraphicTrigger::ReadStringField(const std::string& text, size_t begin, size_t end, const char* field, std::string* value) const
{
	const std::string key = std::string("\"") + field + "\"";
	const size_t key_pos = text.find(key, begin);
	if (key_pos == std::string::npos || key_pos > end)
	{
		return false;
	}

	const size_t colon = text.find(':', key_pos + key.size());
	const size_t quote_begin = colon == std::string::npos ? std::string::npos : text.find('"', colon + 1);
	const size_t quote_end = quote_begin == std::string::npos ? std::string::npos : text.find('"', quote_begin + 1);
	if (colon == std::string::npos || quote_begin == std::string::npos || quote_end == std::string::npos || quote_end > end)
	{
		return false;
	}

	*value = text.substr(quote_begin + 1, quote_end - quote_begin - 1);
	return true;
}

bool GraphicTrigger::ReadDoubleField(const std::string& text, size_t begin, size_t end, const char* field, double* value) const
{
	const std::string key = std::string("\"") + field + "\"";
	const size_t key_pos = text.find(key, begin);
	if (key_pos == std::string::npos || key_pos > end)
	{
		return false;
	}

	const size_t colon = text.find(':', key_pos + key.size());
	if (colon == std::string::npos || colon > end)
	{
		return false;
	}

	size_t value_end = text.find_first_of(",}\n", colon + 1);
	if (value_end == std::string::npos || value_end > end)
	{
		value_end = end;
	}

	*value = atof(text.substr(colon + 1, value_end - colon - 1).c_str());
	return true;
}

bool GraphicTrigger::ReadDwordField(const std::string& text, size_t begin, size_t end, const char* field, DWORD* value) const
{
	double number = 0.0;
	if (!ReadDoubleField(text, begin, end, field, &number))
	{
		return false;
	}

	*value = static_cast<DWORD>(number < 0.0 ? 0.0 : number);
	return true;
}
