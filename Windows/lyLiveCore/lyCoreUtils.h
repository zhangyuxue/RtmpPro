#pragma once

#include <Windows.h>

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

//×Ö·û´®·Ö¸îº¯Êý
std::vector<std::string> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//À©Õ¹×Ö·û´®ÒÔ·½±ã²Ù×÷
	int size = str.size();

	for (int i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}


#include <stdio.h>

#define RGB2YUV(r,g,b,y,u,v)\
	y = (77*r + 150*g + 29*b)>>8;\
	u =((128*b - 43*r -85*g)>>8) +128;\
	v =((128*r - 107*g -21*b)>>8) +128;\
	y = y < 0 ? 0 : y;\
	u = u < 0 ? 0 : u;\
	v = v < 0 ? 0 : v;\
	y = y > 255 ? 255 : y;\
	u = u > 255 ? 255 : u;\
	v = v > 255 ? 255 : v

int rgb888_to_yuv422i(unsigned char *prgb888, unsigned char *pyuv422i, int w, int h)
{
	int i, j;
	unsigned char r, g, b;
	int y, u, v;

	if (((w & 0x1) != 0) || ((h & 0x1) != 0))
	{
		printf("width and height must be multiple of 2.\n");
		return -1;
	}

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			r = *prgb888++;
			g = *prgb888++;
			b = *prgb888++;
			RGB2YUV(b, g, r, y, u, v);
			if (j & 0x1)
			{
				*pyuv422i++ = v;
				*pyuv422i++ = y;
			}
			else
			{
				*pyuv422i++ = u;
				*pyuv422i++ = y;
			}
		}
	}

	return 0;
}

int rgb888_to_yuv422p(unsigned char *prgb888, unsigned char *pyuv422p, int w, int h)
{
	int i, j;
	unsigned char r, g, b;
	int y, u, v;
	unsigned char *py, *pu, *pv;

	if (((w & 0x1) != 0) || ((h & 0x1) != 0))
	{
		printf("width and height must be multiple of 2.\n");
		return -1;
	}

	py = pyuv422p;
	pu = py + w * h;
	pv = pu + w * h / 2;

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			r = *prgb888++;
			g = *prgb888++;
			b = *prgb888++;
			RGB2YUV(b, g, r, y, u, v);
			*py++ = y;
			if (j & 0x1)
			{
				*pu++ = u;
				*pv++ = v;
			}
		}
	}

	return 0;
}

int rgb888_to_yuv420i(unsigned char *prgb888, unsigned char *pyuv420i, int w, int h)
{
	int i, j;
	unsigned char r, g, b;
	int y, u, v;
	unsigned char *py, *puv;

	if (((w & 0x1) != 0) || ((h & 0x1) != 0))
	{
		printf("width and height must be multiple of 2.\n");
		return -1;
	}

	py = pyuv420i;
	puv = py + w * h;

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			r = *prgb888++;
			g = *prgb888++;
			b = *prgb888++;
			RGB2YUV(b, g, r, y, u, v);
			*py++ = y;
			if ((i & 0x1) && (j & 0x1))
			{
				*puv++ = u;
				*puv++ = v;
			}
		}
	}

	return 0;
}

int rgb888_to_yuv420p(unsigned char *prgb888, unsigned char *pyuv420p, int w, int h)
{
	int i, j;
	unsigned char r, g, b;
	int y, u, v;
	unsigned char *py, *pu, *pv;

	if (((w & 0x1) != 0) || ((h & 0x1) != 0))
	{
		printf("width and height must be multiple of 2.\n");
		return -1;
	}

	py = pyuv420p;
	pu = py + w * h;
	pv = pu + w * h / 4;

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			r = *prgb888++;
			g = *prgb888++;
			b = *prgb888++;
			RGB2YUV(b, g, r, y, u, v);
			*py++ = y;
			if ((i & 0x1) && (j & 0x1))
			{
				*pu++ = u;
				*pv++ = v;
			}
		}
	}

	return 0;
}

unsigned char clip_value(unsigned char x, unsigned char min_val, unsigned char  max_val) {
	if (x > max_val) {
		return max_val;
	}
	else if (x < min_val) {
		return min_val;
	}
	else {
		return x;
	}
}

//RGB to YUV420
bool RGB24_TO_YUV420(unsigned char *RgbBuf, int w, int h, unsigned char *yuvBuf)
{
	unsigned char*ptrY, *ptrU, *ptrV, *ptrRGB;
	memset(yuvBuf, 0, w*h * 3 / 2);
	ptrY = yuvBuf;
	ptrU = yuvBuf + w * h;
	ptrV = ptrU + (w*h * 1 / 4);
	unsigned char y, u, v, r, g, b;
	for (int j = 0; j < h; j++) {
		ptrRGB = RgbBuf + w * j * 3;
		for (int i = 0; i < w; i++) {

			r = *(ptrRGB++);
			g = *(ptrRGB++);
			b = *(ptrRGB++);
			y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
			u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
			v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
			*(ptrY++) = clip_value(y, 0, 255);
			if (j % 2 == 0 && i % 2 == 0) {
				*(ptrU++) = clip_value(u, 0, 255);
			}
			else {
				if (i % 2 == 0) {
					*(ptrV++) = clip_value(v, 0, 255);
				}
			}
		}
	}
	return true;
}