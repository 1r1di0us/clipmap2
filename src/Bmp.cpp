#include "Bmp.h"

// default (empty) bitmap constructor
Bmp::Bmp()
{
	width = height = 0; // width and height of the bitmap
	data = NULL; // height data?

}

// bitmap constructor that takes a series of parameters, which
// later are passed to set()
Bmp::Bmp(int x, int y, int b, unsigned char* buffer)
{
	width = height = 0; // set width and height of bitmap to zero
	data = NULL; // 0-dimensional bitmap contains no data
	set(x, y, b, buffer); // now set the bitmap to the appropriate size, using set()
}

// bitmap constructor that takes a filename, which is later passed
// to load()
Bmp::Bmp(const char* filename)
{
	width = height = 0; // set width and height of bitmap to zero
	data = NULL; // 0-dimensional bitmap has no data
	load(filename); // now set the bitmap to the appropraite file, using load()
}

// squiggly constructor?
Bmp::~Bmp()
{
	if (data) free(data); // ??
}

// saves the filename passed to it as the current bitmap?
void Bmp::save(const char* filename)
{
	printf("saving image %s\n", filename);
	unsigned char bmp[58] =
	{ 0x42, 0x4D, 0x36, 0x30, 0, 0, 0, 0, 0, 0, 0x36, 0, 0, 0, 0x28, 0, 0, 0, // ??
		0x40, 0, 0, 0, // X-Size
		0x40, 0, 0, 0, // Y-Size
		1, 0, 0x18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // third entry is bpp

	bmp[18] = width;
	bmp[19] = width >> 8; // bitwise shift right by 8 (same as division by 2^8 = 256
	bmp[22] = height;
	bmp[23] = height >> 8; // bitwise shift right by 8 (same as division by 2^8 = 256)
	bmp[28] = bpp; // bits per pixel

	// open file using filename passed orignally to function
	FILE* fn; // file pointer
	if ((fn = fopen(filename, "wb")) != NULL) // if the file exists, 'wb' for wriitng to non-text files
	{
		// write to the file
		fwrite(bmp, 1, 54, fn); // write the bitmap
		fwrite(data, 1, (long)width * height * (bpp / 8), fn); // write the data
		fclose(fn); // close the file
	}
	else error_stop("Bmp::save");
}

// loads the bitmap specified by the passed file name?
void Bmp::load(const char* filename)
{
	FILE* handle;

	// error handling begins
	if (filename == NULL)
	{
		printf("File not found %s !\n", filename); while (1);;
	}
	if ((char)filename[0] == 0)
	{
		printf("File not found %s !\n", filename); while (1);;
	}
	if ((handle = fopen(filename, "rb")) == NULL) // 'rb' for reading non-text files
	{
		printf("File not found %s !\n", filename); while (1);;
	}
	if (!fread(bmp, 11, 1, handle))
	{
		printf("Error reading file %s!\n", filename);
		while (1);
	}
	if (!fread(&bmp[11], (int)((unsigned char)bmp[10]) - 11, 1, handle))
	{
		printf("Error reading file %s!\n", filename);
		while (1);
	}
	// error handling ends

	// ??
	width = (int)((unsigned char)bmp[18]) + ((int)((unsigned char)(bmp[19])) << 8); // ??
	height = (int)((unsigned char)bmp[22]) + ((int)((unsigned char)(bmp[23])) << 8); // ??
	bpp = bmp[28]; // bits per pixel

	//printf("%s : %dx%dx%d Bit \n",filename,width,height,bpp);

	if (data) free(data); // ?

	int size = width * height * (bpp / 8); // size of the bitmap file? what is bpp?

	data = (unsigned char*)malloc(size + 1);
	fread(data, size, 1, handle); // read from the file

	fclose(handle); // close the file

	printf("read successfully %s ; %dx%dx%d Bit \n", filename, width, height, bpp); // successfull read
}

// save an entry in the clipmap?
void Bmp::save_float(const char* filename)
{
	FILE* fn;
	if ((fn = fopen(filename, "wb")) == NULL)  error_stop("Bmp::save_float");
	fwrite(data, 1, 4 * width * height, fn);
	fclose(fn);
}

// load an entry in the clipmap?
void Bmp::load_float(const char* filename)
{
	FILE* fn;
	if ((fn = fopen(filename, "rb")) == NULL)  error_stop("Bmp::load_float");
	fread(data, 1, 4 * width * height, fn);
	fclose(fn);
}

// set the value of an individual pixel
void Bmp::set_pixel(int x, int y, int r, int g, int b)
{
	// bpp??
	data[(x + y * width) * (bpp / 8) + 2] = r;
	data[(x + y * width) * (bpp / 8) + 1] = g;
	data[(x + y * width) * (bpp / 8) + 0] = b;
}

// get the value of an individual pixel
int Bmp::get_pixel(int x, int y)
{
	if (data == 0) error_stop("get_pixel data=0");
	if (x >= width)return 0;
	if (y >= height)return 0;
	return
		data[(x + y * width) * (bpp / 8) + 0] +
		data[(x + y * width) * (bpp / 8) + 1] * 256 +
		data[(x + y * width) * (bpp / 8) + 2] * 256 * 256;
}

// get the color of an individual pixel?
vec3f Bmp::get_pixel3f(int x, int y)
{
	int color = get_pixel(x, y);
	float r = float(color & 255) / 255.0f;
	float g = float((color >> 8) & 255) / 255.0f;
	float b = float((color >> 16) & 255) / 255.0f;
	return vec3f(r, g, b);
}

// this function will be used to blur the transitions between the different clipmap LODs
void  Bmp::blur(int radius)
{

}

// set a bitmap passed on the passed parameters
void Bmp::set(int x, int y, int b, unsigned char* buffer)
{
	width = x;
	height = y;
	bpp = b;
	if (data) free(data);

	data = (unsigned char*)malloc(width * height * (bpp / 8));
	if (!data) error_stop("Bmp::set : out of memory");

	if (buffer == 0)
		memset(data, 0, width * height * (bpp / 8));
	else
		memmove(data, buffer, width * height * (bpp / 8));

	bmp[18] = width;
	bmp[19] = width >> 8; // bitwise shify right by 8 (same as division by 2^8 = 256)
	bmp[22] = height;
	bmp[23] = height >> 8; // bitwise shify right by 8 (same as division by 2^8 = 256)
	bmp[28] = bpp;
}