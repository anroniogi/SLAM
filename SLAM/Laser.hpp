#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <stdlib.h>
#include <math.h>

#include "Urg_driver.h"
#include "Connection_information.h"
#include "math_utilities.h"

/**
* BMP파일 생성을 위한 선언들...
*
***************************************************************************************************/
unsigned int width = 200, height = 200;
unsigned int bpp = 24;
unsigned int rowsize = ((bpp*width + 31) / 32) * 4;
unsigned int area = width*height;
unsigned int sizeOfArr = rowsize *height;
unsigned int fileSize = 54 + sizeOfArr;

unsigned int resolution = 2835;//see wiki. 72dpi* 39.3701 in/meter

unsigned char bmpfileheader[14] = { 'B','M',   0,0,0,0 ,   0,0,0,0 ,   54,0,0,0 };
unsigned char bmpinfoheader[40] = { 40,0,0,0,  width,0,0,0,   height,0,0,0 ,   1,0,24,0 };
/***************************************************************************************************/


struct Laser {
	size_t num;
	long x, y;
};

// Mobile Robot Position
struct Position {
	long x = 0, y = 0, theta = 0;
} Position;

int map[300][300] = { 0, };

union RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
RGB white[3] = { 255, 255, 255 };
RGB black[3] = { 0, 0, 0 };


struct Laser laser[143];

using namespace qrk;
using namespace std;

namespace
{
	int cnt = 0;

	void print_data(const Urg_driver& urg, const vector<long>& data, long time_stamp, char* name) {

		// bmp 파일 생성
		bmpinfoheader[25] = (unsigned char)(resolution);      //horizontal resolutions
		bmpinfoheader[29] = (unsigned char)(resolution);      //vertical resolutions
		//FILE *pfile = fopen("image.bmp", "wb");
		
		
		FILE *pfile = fopen(name, "wb");
		fwrite(bmpfileheader, 1, 14, pfile);
		fwrite(bmpinfoheader, 1, 40, pfile);
		

#if 0
		// Shows only the front step
		int front_index = urg.step2index(0);
		cout << data[front_index] << " [mm], ("
			<< time_stamp << " [msec])" << endl;
#else
		// Prints the X-Y coordinates for all the measurement points 좌표로 출력
		long min_distance = urg.min_distance();	//최소거리
		long max_distance = urg.max_distance();	//최대거리

		long tempx, tempy;

		size_t data_n = data.size();
		//상규 수정
		//int j = floor(data_n / 2);
		//for(int i=-j; i<j; ++i){
		for (size_t i = 0; i < data_n; ++i) {
			long l = data[i];
			if ((l <= min_distance) || (l >= max_distance)) {
				continue;
			}

			double radian = urg.index2rad(i);
			long x = static_cast<long>(l * cos(radian));
			long y = static_cast<long>(l * sin(radian));
			cout << i << " : (" << x << ", " << y << ")" << endl;
			
			laser[cnt].num = i;
			laser[cnt].x = x;
			laser[cnt].y = y+4000;
			++cnt;
			
<<<<<<< HEAD
			//mobile robot의 Position x, y만큼 장애물위치 이동
			tempx = floor(x / 50) + Position.x;
			tempy = floor((y+4000) / 50) + Position.y;

			//mobile robot의 theta만큼 장애물위치 회전
			tempx = tempx * (cos(Position.theta)) - tempy * (sin(Position.theta));
			tempy = tempx * (sin(Position.theta)) + tempy * (cos(Position.theta));
			
=======
			////mobile robot의 Position x, y만큼 장애물위치 이동
			//tempx = floor((x + Position.x) / 50);
			//tempy = floor((y+4000 + Position.y) / 50);

			////mobile robot의 theta만큼 장애물위치 회전
			//tempx = tempx * (cos(Position.theta)) - tempy * (sin(Position.theta));
			//tempy = tempx * (sin(Position.theta)) + tempy * (cos(Position.theta));
			//
>>>>>>> 0f7d672450b1236b7d39b467e940b8a7078423e5
			
			// 배열에 대입
//			map[tempx][tempy] += 1;


		}
		cout << endl;
		
	/*	for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++)	{
				if (map[i][j] == 0) {
					fwrite(white, sizeof(unsigned char), 3, pfile);
				}
				else fwrite(black, sizeof(unsigned char), 3, pfile);
			}
		}

		fclose(pfile);
		*/
#endif
	}
}