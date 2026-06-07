#pragma once

#include "GraphicImageReader.h"

class GraphicImageJudger
{
public:
	bool ContainsTemplate(const GraphicImage& screen, const GraphicImage& templ, double threshold) const;

private:
	double CompareAt(const GraphicImage& screen, const GraphicImage& templ, int left, int top) const;
};
