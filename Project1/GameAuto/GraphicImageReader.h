#pragma once

#include <string>
#include <vector>

struct GraphicImage
{
	int width;
	int height;
	std::vector<unsigned char> bgra;
};

class GraphicImageReader
{
public:
	bool LoadImageFile(const std::string& path, GraphicImage* image) const;
};
