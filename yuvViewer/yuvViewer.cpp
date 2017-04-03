// yuvViewer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdlib>
#include <cstring>

using namespace cv;
typedef unsigned char       BYTE;
#define CLAMP(x, min, max) 	if ((x) > max) \
								x = max;  \
							else if ((x) < min)  \
								x = min;  

#define FORMAT_PACKED

int main(int argc, char* argv[])
{
	Mat			outputMat;

	char deli[] = "_.";
	char *context = NULL;
	char* filename = argv[1];
	char filename_b[255];
	strcpy(filename_b, filename);
	strtok_s(filename_b, deli, &context); //result
	int width = atoi(strtok_s(NULL, deli, &context));
	int height = atoi(strtok_s(NULL, deli, &context));
	
	int length = width * height;

	std::ifstream is(filename, std::ifstream::binary);
	if (!is.is_open())
		return 1;
	char* buf = new char[length + length /2];
	is.read(buf, length + length / 2);
#ifdef FORMAT_PACKED
	char* buf_uv = buf + length;
#else
	char* buf_u = buf + length;
	char* buf_v = buf_u + length /4;
#endif
	is.close();

	outputMat.create(height, width, CV_8UC3);
	unsigned char y, u, v;
	short r, g, b;
	short c, d, e;
	int i = 0;
	for (int row = 0; row < outputMat.rows; row++)
	{
		for (int col = 0; col < outputMat.cols; col++)
		{
			y = buf[i++];
#ifdef FORMAT_PACKED
			u = buf_uv[width * (row >> 1) + (col & (~1))];
			v = buf_uv[width * (row >> 1) + (col & (~1)) + 1];
#else
			u = buf_u[width / 2 * (row >> 1) + (col >> 1)];
			v = buf_v[width / 2 * (row >> 1) + (col >> 1)];
#endif
		

			c = y - 16;
			d = u - 128;
			e = v - 128;

			r = (298 * c + 409 * e + 128) >> 8;
			CLAMP(r, 0, 255);
			g = (298 * c - 100*d - 208 * e + 128) >> 8;
			CLAMP(g, 0, 255);
			b = (298 * c + 516 * d + 128) >> 8;
			CLAMP(b, 0, 255);

			outputMat.at<cv::Vec3b>(row, col)[0] = b;
			outputMat.at<cv::Vec3b>(row, col)[1] = g;
			outputMat.at<cv::Vec3b>(row, col)[2] = r;

		

		}
	}
	
	imshow("Output image", outputMat);
	cv::imwrite("result.bmp", outputMat);
	cv::waitKey(0);
	delete[] buf;
	return 0;
}

