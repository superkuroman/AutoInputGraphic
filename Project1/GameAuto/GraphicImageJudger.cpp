#include "GraphicImageJudger.h"

#include <cstddef>
#include <cstdlib>

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
}

bool GraphicImageJudger::ContainsTemplate(const GraphicImage& screen, const GraphicImage& templ, double threshold) const
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

double GraphicImageJudger::CompareAt(const GraphicImage& screen, const GraphicImage& templ, int left, int top) const
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
