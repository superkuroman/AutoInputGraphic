#pragma once

#include "GraphicImageJudger.h"
#include "GraphicImageReader.h"
#include "GraphicScreenAccessor.h"

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>
#include <windows.h>

class InputStep;

struct GraphicTriggerRule
{
	std::string name;
	std::string template_image;
	std::string script_file;
	double threshold;
	DWORD cooldown_ms;
	std::chrono::steady_clock::time_point last_run;
};

class GraphicTriggerSelector
{
public:
	explicit GraphicTriggerSelector(const char* config_file);
	void Update(InputStep& player);

private:
	bool LoadConfig();
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
	GraphicImageReader image_reader_;
	GraphicScreenAccessor screen_accessor_;
	GraphicImageJudger image_judger_;
};
