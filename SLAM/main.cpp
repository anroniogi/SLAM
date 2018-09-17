#define _CRT_SECURE_NO_WARNINGS


/*!
\example get_distance.cpp Obtains distance data
\author Satofumi KAMIMURA

$Id$
*/
//Stella B1
#include "stdafx.h"
#include "SerialPort.h"
#include "StellaB1.h"
#include <windows.h>
#include <math.h>

//Laser Scanner
#include "Laser.hpp"

#include <thread>
#include <opencv2/opencv.hpp>


using namespace qrk;
using namespace std;
using namespace cv;
#pragma warning (disable : 4996)

//opencv gui 창선언
void window() {
	Mat image = Mat::zeros(300, 300, CV_8UC3);	// 세로, 가로 각각 400, 400 사이즈의 검정 이미지를 만든다.
	image = Scalar(255, 255, 255);				// 이미지 흰색으로 초기화
	while (1) {
		int cnt = 0;
		for (int i = 0; i < 300; ++i) {
			for (int j = 0; j < 300; ++j) {
				if (MAP::map[i][j] != 0) {
					ellipse(image, Point(i, j), Size(2, 2), 0, 0, 360, Scalar(0, 0, 0), 1, 8);
					++cnt;
				}
			}
		}
		imshow("result 1", image);
		printf("%d개 출력\n", cnt);
		waitKey(500);
	}
}


int main(int argc, char *argv[])
{	//opencv gui thread 선언
	thread t1(window);
	t1.detach();


	// 레이저 스캐너 연결 정보
	Connection_information information(argc, argv);

	// Stella B1 연결정보
	CSerialPort *_rc = new CSerialPort();
	_rc->Open("COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY);
	_rc->SetTimeout(100, 100, 10);

	CStellaB1 *_sg = new CStellaB1(_rc);







	// 레이저 변수
	char name[80] = "";
	char image[80] = "image";
	char* bmp = ".bmp";
	char buffe[10];
	char* buffer = buffe;

	// 스텔라 변수
	char state;
	float left, right;
	double wheel_distance = 75.2;
	double robot_x, robot_y, robot_theta;
	double ds;


	// 스텔라 이니셜라이즈
	_sg->Init();
	_sg->Reset();
	_sg->GetState(&state);
	_sg->GetPosition(&left, &right);


	// Connects to the sensor
	Urg_driver urg;
	if (!urg.open(information.device_or_ip_name(),
		information.baudrate_or_port_number(),
		information.connection_type())) {
		std::cout << "Urg_driver::open(): " << information.device_or_ip_name() << ": " << urg.what() << endl;
		Sleep(10000);
		return 1;
	}

	// Gets measurement data
#if 1
	// Case where the measurement range (start/end steps) is defined
	// Laser 측정 각도 설정 150도, step 143 // position max 4000
	urg.set_scanning_parameter(urg.deg2step(-75), urg.deg2step(+75), 3);
#endif
	enum { Capture_times = 10 };
	urg.start_measurement(Urg_driver::Distance, Urg_driver::Infinity_times, 0);	// 측정 시작

	std::ofstream LaserFile("laser.txt");	// 측정값 파일출력용





											//for (int i = 0; i < Capture_times; ++i) {
											//while(1) {
	for (int i = 0; i < 30; ++i) {
		vector<long> data;
		long time_stamp = 0;

		// 스텔라 속도 설정
		_sg->Velocity(20, 20);
		Sleep(1000);
		_sg->Velocity(0, 0);



		if (!urg.get_distance(data, &time_stamp)) {		//laser 측정
			cout << "Urg_driver::get_distance(): " << urg.what() << endl;
			Sleep(10000);
			return 1;
		}


		//print_data(urg, data, time_stamp);

		_itoa(i, buffe, 10);
		strcat(name, image);
		strcat(name, buffer);
		strcat(name, bmp);

		// Get Mobile Robot Position
		// data 출력 전 측정
		_sg->GetState(&state);
		_sg->GetPosition(&left, &right);


		//// meter단위를 cm단위로 변환
		//left *= 1000;
		//right *= 1000;

		// 휠의 이동거리 left와 right로 x, y, theta 계산
		ds = (right + left) / 2;
		robot_theta = (right - left) / (wheel_distance);
		robot_x = ds*cos(robot_theta / 2);
		robot_y = ds*sin(robot_theta / 2);

		Position.x = ds*cos(robot_theta / 2);
		Position.y = ds*sin(robot_theta / 2);
		Position.theta = (right - left) / (wheel_distance);


		printf("\nrobot position, theta\n");
		printf("robot_x = %f, robot_y = %f, robot_theta = %f\n", robot_x, robot_y, robot_theta);
		printf("\n%1d  %10.1f  %10.1f \n", (int)state, (double)left, (double)right);



		//data 출력(bmp파일로 출력)
		print_data(urg, data, time_stamp, name);	//laser값 받아옴
		std::cout << "lasersensor 측정 : " << i << std::endl;
		std::cout << "Position x : " << Position.x << "\nPosition y : " <<
			Position.y << "\nPosition theta : " << Position.theta << std::endl;
		Sleep(1000);
		name[0] = '\0';
		cout << "\n" << endl;


		//Position 변화 BMP파일 적용Test
		//Position.x += 10;

	}

	for (int y = 0; y < 10000; ++y) {
		LaserFile << laser[y].num << " " << laser[y].x << " " << laser[y].y << std::endl;
	}
	LaserFile.close();


	printf("---------------------------------------------------------------------\n");
	printf("finished, insert any key to end\n");
	printf("---------------------------------------------------------------------\n");
	_sg->Velocity(00, 00);
	urg.close();
	getchar();

#if defined(URG_MSC)
	getchar();
#endif
	return 0;
}
