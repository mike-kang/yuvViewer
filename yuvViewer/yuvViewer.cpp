// yuvViewer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdlib>
#include <cstring>
//#include <Windows.h>
using namespace cv;
typedef unsigned char       BYTE;
#define CLAMP(x, min, max) 	if ((x) > max) \
								x = max;  \
							else if ((x) < min)  \
								x = min;  

enum format_yuv_type {
	FM_422_YUYV,
	FM_422_UYVY,
	FM_420_PLANER_UV, // I420
	FM_420_SEMI_PLANER_UV, //NV12
	FM_420_SEMI_PLANER_VU, //NV21
};

//#define FORMAT_PACKED_VU
struct yuyv_t {
	uchar y1;
	uchar u;
	uchar y2;
	uchar v;
};

struct uv_t {
	uchar u;
	uchar v;
};

struct vu_t {
	uchar v;
	uchar u;
};
struct uyvy_t {
	uchar u;
	uchar y1;
	uchar v;
	uchar y2;
};

void convert(unsigned char y, unsigned char u, unsigned char v, unsigned char &r, unsigned char &g, unsigned char &b)
{
	short c, d, e;
	short _r, _g, _b;

	c = y - 16;
	d = u - 128;
	e = v - 128;


	_r = (298 * c + 409 * e + 128) >> 8;
	CLAMP(_r, 0, 255);
	_g = (298 * c - 100 * d - 208 * e + 128) >> 8;
	CLAMP(_g, 0, 255);
	_b = (298 * c + 516 * d + 128) >> 8;
	CLAMP(_b, 0, 255);

	r = _r;
	g = _g;
	b = _b;
}

int main(int argc, char* argv[])
{
	Mat			outputMat;
	format_yuv_type fmt = FM_420_PLANER_UV;
	char deli[] = "_.";
	char *context = NULL;
	char* filename = argv[1];
	char filename_b[255];
	strcpy(filename_b, filename);
	strtok_s(filename_b, deli, &context); //result
	int width = atoi(strtok_s(NULL, deli, &context));
	int height = atoi(strtok_s(NULL, deli, &context));
	int pitch = atoi(strtok_s(NULL, deli, &context));

	int length = pitch * height;

	std::ifstream is(filename, std::ifstream::binary);
	if (!is.is_open())
		return 1;

	int buf_length;
	if (fmt == FM_422_YUYV || fmt == FM_422_UYVY) {
		buf_length = length * 2;
	}
	else
		buf_length = length + length / 2;
	char* buf = new char[buf_length];
	is.read(buf, buf_length);

	char *buf_u, *buf_v;
	
	is.close();

	outputMat.create(height, width, CV_8UC3);
	unsigned char y, u, v;
	unsigned char r, g, b;
	//short c, d, e;
	int i = 0;

	uchar* dest_buf = outputMat.data;
	unsigned char yy[2];
	if (fmt == FM_422_YUYV) {	//packed
		
		yuyv_t* src = (yuyv_t*)buf;
		for (int i = 0; i < length / 2; i++) {
			u = src->u;
			v = src->v;
			yy[0] = src->y1;
			yy[1] = src->y2;
			for (int j = 0; j < 2; j++) {
				y = yy[j];

				convert(y, u, v, r, g, b);

				*(dest_buf++) = b;
				*(dest_buf++) = g;
				*(dest_buf++) = r;
			}
			src++;
		}

	}
	else if (fmt == FM_422_UYVY) {	//packed
		
		uyvy_t* src = (uyvy_t*)buf;
		for (int i = 0; i < length / 2; i++) {
			u = src->u;
			v = src->v;
			yy[0] = src->y1;
			yy[1] = src->y2;
			for (int j = 0; j < 2; j++) {
				y = yy[j];

				convert(y, u, v, r, g, b);

				*(dest_buf++) = b;
				*(dest_buf++) = g;
				*(dest_buf++) = r;
			}
			src++;
		}

	}
	else if (fmt == FM_420_SEMI_PLANER_UV) {
		uv_t* buf_uv = (uv_t*)(buf + length); //for semi_planer

		for (int row = 0; row < outputMat.rows; row++)
		{
			for (int col = 0; col < outputMat.cols; col++)
			{
				y = buf[pitch * row + col];

				uv_t* temp = &buf_uv[(pitch >> 1) * (row >> 1) + (col >> 1)];
				u = temp->u;
				v = temp->v;

				convert(y, u, v, r, g, b);

				*(dest_buf++) = b;
				*(dest_buf++) = g;
				*(dest_buf++) = r;
			}
		}
	}
	else if (fmt == FM_420_SEMI_PLANER_VU) {
		vu_t* buf_vu = (vu_t*)(buf + length); //for semi_planer

		for (int row = 0; row < outputMat.rows; row++)
		{
			for (int col = 0; col < outputMat.cols; col++)
			{
				y = buf[pitch * row + col];

				vu_t* temp = &buf_vu[(pitch >> 1) * (row >> 1) + (col >> 1)];
				u = temp->u;
				v = temp->v;

				convert(y, u, v, r, g, b);

				*(dest_buf++) = b;
				*(dest_buf++) = g;
				*(dest_buf++) = r;
			}
		}
	}
	else if (fmt == FM_420_PLANER_UV) {
		buf_u = buf + length;
		buf_v = buf_u + length / 4;

		for (int row = 0; row < outputMat.rows; row++)
		{
			for (int col = 0; col < outputMat.cols; col++)
			{
				y = buf[pitch * row + col];

				u = buf_u[pitch / 2 * (row >> 1) + (col >> 1)];
				v = buf_v[pitch / 2 * (row >> 1) + (col >> 1)];

				convert(y, u, v, r, g, b);

				*(dest_buf++) = b;
				*(dest_buf++) = g;
				*(dest_buf++) = r;
			}
		}
	}

	imshow("Output image", outputMat);
	cv::imwrite("result.bmp", outputMat);
	cv::waitKey(0);
	delete[] buf;
	return 0;
}

