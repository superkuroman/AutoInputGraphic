#include "GraphicTriggerSelector.h"

#include "../InputStep.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>

namespace
{
constexpr DWORD kScanIntervalMs = 500;
constexpr double kDefaultThreshold = 0.04;
constexpr DWORD kDefaultCooldownMs = 5000;

inline double ClampDouble(double value, double min_value, double max_value)
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

inline std::wstring ToWide(const std::string& text)
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

GraphicTriggerSelector::GraphicTriggerSelector(const char* config_file)
	: config_file_(config_file),
	  window_title_(),
	  rules_(),
	  loaded_(false),
	  last_scan_(std::chrono::steady_clock::now()),
	  image_reader_(),
	  screen_accessor_(),
	  image_judger_()
{
}

void GraphicTriggerSelector::Update(InputStep& player)
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
	if (!screen_accessor_.FindWindowByTitle(window_title_, &hwnd))
	{
		return;
	}

	GraphicImage screen = {};
	if (!screen_accessor_.CaptureClient(hwnd, &screen))
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

		GraphicImage templ = {};
		if (!image_reader_.LoadImageFile(rule.template_image, &templ))
		{
			continue;
		}

		if (image_judger_.ContainsTemplate(screen, templ, rule.threshold))
		{
			printf("Graphic trigger matched: %s -> %s\n", rule.name.c_str(), rule.script_file.c_str());
			MarkRun(&rule);
			player.PlaybackFromFile(rule.script_file.c_str());
			break;
		}
	}
}

bool GraphicTriggerSelector::LoadConfig()
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

		if (!ReadStringField(text, obj, obj_end, "template_image", &rule.template_image))
		{
			ReadStringField(text, obj, obj_end, "template_bmp", &rule.template_image);
		}

		if (!rule.template_image.empty() &&
			ReadStringField(text, obj, obj_end, "script", &rule.script_file))
		{
			if (rule.name.empty())
			{
				rule.name = rule.template_image;
			}
			rule.threshold = ClampDouble(rule.threshold, 0.0, 1.0);
			rules_.push_back(rule);
		}

		pos = obj_end + 1;
	}

	return !rules_.empty();
}

bool GraphicTriggerSelector::IsReady(const GraphicTriggerRule& rule) const
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

void GraphicTriggerSelector::MarkRun(GraphicTriggerRule* rule)
{
	rule->last_run = std::chrono::steady_clock::now();
}

std::string GraphicTriggerSelector::ReadTextFile(const char* filename) const
{
	std::ifstream ifs(filename);
	if (!ifs)
	{
		return std::string();
	}
	return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

bool GraphicTriggerSelector::ReadStringField(const std::string& text, size_t begin, size_t end, const char* field, std::string* value) const
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

bool GraphicTriggerSelector::ReadDoubleField(const std::string& text, size_t begin, size_t end, const char* field, double* value) const
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

bool GraphicTriggerSelector::ReadDwordField(const std::string& text, size_t begin, size_t end, const char* field, DWORD* value) const
{
	double number = 0.0;
	if (!ReadDoubleField(text, begin, end, field, &number))
	{
		return false;
	}

	*value = static_cast<DWORD>(number < 0.0 ? 0.0 : number);
	return true;
}
