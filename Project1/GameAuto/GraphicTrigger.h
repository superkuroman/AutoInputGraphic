#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <windows.h>

class InputStep;

struct GraphicTriggerRule
{
	std::string name;
	std::string template_bmp;
	std::string script_file;
	double threshold;
	DWORD cooldown_ms;
	std::chrono::steady_clock::time_point last_run;
};

class GraphicTrigger
{
public:
	explicit GraphicTrigger(const char* config_file);
	void Update(InputStep& player);

private:
	struct Image
	{
		int width;
		int height;
		std::vector<unsigned char> bgra;
	};

	bool LoadConfig();
	bool TryFindWindow(HWND* hwnd) const;
	bool LoadBmp(const std::string& path, Image* image) const;
	bool CaptureWindow(HWND hwnd, Image* image) const;
	bool FindTemplate(const Image& screen, const Image& templ, double threshold) const;
	double CompareAt(const Image& screen, const Image& templ, int left, int top) const;
	bool IsReady(const GraphicTriggerRule& rule) const;
	void MarkRun(GraphicTriggerRule* rule);

	std::string ReadTextFile(const char* filename) const;
	bool ReadStringField(const std::string& text, size_t begin, size_t end, const char* field, std::string* value) const;
	bool ReadDoubleField(const std::string& text, size_t begin, size_t end, const char* field, double* value) const;
	bool ReadDwordField(const std::string& text, size_t begin, size_t end, const char* field, DWORD* value) const;

	std::string config_file_;
	std::wstring window_title_;
	std::vector<GraphicTriggerRule> rules_;
	bool loaded_;
	std::chrono::steady_clock::time_point last_scan_;
};
