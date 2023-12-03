
#pragma once

#include "core.h"

// bitmap class. A bitmap is quite literally, a map of bits.
// each bitmap has a length and a width, as well as a bits per pixel
// 
// 
// https://en.wikipedia.org/wiki/Bitmap
// https://en.wikipedia.org/wiki/BMP_file_format

class Bmp
{
public:

	// public fields
	unsigned char* data;
	int width;
	int height;
	int bpp;

	// constructors
	Bmp();
	Bmp(const char* filename);
	Bmp(int x, int y, int bpp, unsigned char* data = 0);
	~Bmp();

	// public methods
	void load(const char* filename);
	void save(const char* filename);
	void save_float(const char* filename);
	void load_float(const char* filename);
	void set(int x, int y, int bpp, unsigned char* data);
	void blur(int radius);
	void set_pixel(int x, int y, int r, int g, int b);
	int  get_pixel(int x, int y);
	vec3f get_pixel3f(int x, int y);

private:

	// private fields
	unsigned char bmp[54];
};