#include "GraphicImageJudger.h"

#include <cstddef>
#include <cstdlib>

namespace
{
inline int ClampInt(int value, int min_value, int max_value)
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

inline size_t RowPixelOffset(int x)
{
	return static_cast<size_t>(x) * 4;
}

inline unsigned int AbsDiff(unsigned char lhs, unsigned char rhs)
{
	return lhs > rhs ? lhs - rhs : rhs - lhs;
}

inline unsigned int ColorDiff(const unsigned char* lhs, const unsigned char* rhs)
{
	return
		AbsDiff(lhs[0], rhs[0]) +
		AbsDiff(lhs[1], rhs[1]) +
		AbsDiff(lhs[2], rhs[2]);
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
			const size_t screen_index = screen_row + RowPixelOffset(left + x);
			const size_t templ_index = templ_row + RowPixelOffset(x);
			total_diff += ColorDiff(&screen.bgra[screen_index], &templ.bgra[templ_index]);
		}
	}

	return max_diff == 0 ? 1.0 : static_cast<double>(total_diff) / static_cast<double>(max_diff);
}
